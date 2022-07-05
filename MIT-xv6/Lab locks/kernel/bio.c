// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKET_SIZE 7

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache[BUCKET_SIZE];

void
binit(void)
{
  struct buf *b;

  for(int i = 0; i < BUCKET_SIZE; ++i){
    initlock(&bcache[i].lock, "bcache");
    for(b = bcache[i].buf; b < bcache[i].buf + NBUF; b++){
      b->timestamp = ticks;
      initsleeplock(&b->lock, "buffer");
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  
  int id = blockno % BUCKET_SIZE;
  uint least_timestamp = -1;
  struct buf *evict_b = 0;
  acquire(&bcache[id].lock);
  // Is the block already cached?
  for(b = bcache[id].buf; b < bcache[id].buf + NBUF; b++){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[id].lock);
      acquiresleep(&b->lock);
      return b;
    }
    if(b->refcnt == 0 && b->timestamp < least_timestamp){
      least_timestamp = b->timestamp;
      evict_b = b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  if(evict_b != 0){
    evict_b->dev = dev;
    evict_b->blockno = blockno;
    evict_b->valid = 0;
    evict_b->refcnt = 1;
    release(&bcache[id].lock);
    acquiresleep(&evict_b->lock);
    return evict_b;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int id = b->blockno % BUCKET_SIZE;
  acquire(&bcache[id].lock);
  b->refcnt--;
  if(b->refcnt == 0)
    b->timestamp = ticks;
  release(&bcache[id].lock);
}

void
bpin(struct buf *b) {
  int id = b->blockno % BUCKET_SIZE;
  acquire(&bcache[id].lock);
  b->refcnt++;
  release(&bcache[id].lock);
}

void
bunpin(struct buf *b) {
  int id = b->blockno % BUCKET_SIZE;
  acquire(&bcache[id].lock);
  b->refcnt--;
  release(&bcache[id].lock);
}


