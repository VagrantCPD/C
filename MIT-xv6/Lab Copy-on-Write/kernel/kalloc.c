// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

// recode how many user page tables are mapped to one physical page
int ref_cnt_tbl[128 * 256 + 1];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

int
get_ref_cnt_tbl_idx(uint64 pa)
{
  uint64 begin = PGROUNDUP((uint64)end);
  return (pa - begin) / PGSIZE;
}

void 
ref_cnt_decr(uint64 pa)
{
  acquire(&kmem.lock);
  ref_cnt_tbl[get_ref_cnt_tbl_idx(pa)] -= 1;
  release(&kmem.lock);
}

void 
ref_cnt_incr(uint64 pa)
{
  acquire(&kmem.lock);
  ref_cnt_tbl[get_ref_cnt_tbl_idx(pa)] += 1;
  release(&kmem.lock);
}

int 
get_ref_cnt(uint64 pa)
{
  return ref_cnt_tbl[get_ref_cnt_tbl_idx(pa)];
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  ref_cnt_decr((uint64) pa);
  if(get_ref_cnt((uint64) pa) > 0)
    return;

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    ref_cnt_tbl[get_ref_cnt_tbl_idx((uint64) r)] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
