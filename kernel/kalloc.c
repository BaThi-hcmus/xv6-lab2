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

// Free the page of physical memory pointed at by pa,
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

uint64
kcollect_freemem(void)
{
  struct run *r;          // Con trỏ dùng để duyệt danh sách liên kết
  uint64 free_bytes = 0;  // Biến lưu tổng số byte trống, khởi tạo bằng 0

  acquire(&kmem.lock);    // Khóa spinlock bảo vệ cấu trúc kmem
  
  r = kmem.freelist;      // Bắt đầu từ trang trống đầu tiên trong danh sách
  while(r){               // Lặp chừng nào con trỏ r còn trỏ tới một trang hợp lệ (khác 0)
    free_bytes += PGSIZE; // Mỗi trang trống tương ứng 4096 bytes (PGSIZE)
    r = r->next;          // Di chuyển sang trang trống tiếp theo trong danh sách
  }
  
  release(&kmem.lock);    // Mở khóa spinlock sau khi đã duyệt xong

  return free_bytes;      // Trả về tổng số byte bộ nhớ trống
}