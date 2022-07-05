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

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct kcpumem {
  struct spinlock lock;
  struct run *freelist;
  int pgsize;
};

struct kcpumem kmems[NCPU];

int getcputid()
{
  push_off();
  int cpu = cpuid();
  pop_off();
  return cpu;
}

void
kinit()
{
  for(int i = 0; i < NCPU; ++i){
    initlock(&kmems[i].lock, "kmem");
    kmems[i].pgsize = 0;
  }
  // initlock(&kmem.lock, "kmem");
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

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // acquire(&kmem.lock);
  // r->next = kmem.freelist;
  // kmem.freelist = r;
  // release(&kmem.lock);

  int mcpu = getcputid();
  acquire(&kmems[mcpu].lock);
  r->next = kmems[mcpu].freelist;
  kmems[mcpu].freelist = r;
  kmems[mcpu].pgsize += 1;
  release(&kmems[mcpu].lock);
}

void stealmem(int mcpu)
{
  for(int i = 0; i < NCPU; ++i){
    if(i == mcpu){
      continue;
    }
    int success = 0;
    acquire(&kmems[i].lock);
    if(kmems[i].pgsize >= 2){
      int stealpgsize = kmems[i].pgsize / 2;
      struct run *m_freelist = kmems[i].freelist;
      struct run *i_freelist = kmems[i].freelist;
      for(int i = 1; i < stealpgsize; ++i){
        i_freelist = i_freelist->next;
      }
      kmems[i].freelist = i_freelist->next;
      i_freelist->next = 0;
      kmems[i].pgsize -= stealpgsize;
      kmems[mcpu].pgsize = stealpgsize;
      kmems[mcpu].freelist = m_freelist;
      success = 1;
    }
    release(&kmems[i].lock);
    if(success == 1){
      return;
    }
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  // acquire(&kmem.lock);
  // r = kmem.freelist;
  // if(r)
  //   kmem.freelist = r->next;
  // release(&kmem.lock);

  int mcpu = getcputid();
  acquire(&kmems[mcpu].lock);
  if(kmems[mcpu].pgsize == 0){
    stealmem(mcpu);
  }
  r = kmems[mcpu].freelist;
  if(r){
    kmems[mcpu].freelist = r->next;
    kmems[mcpu].pgsize -= 1;
  }
  release(&kmems[mcpu].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
