#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);            // Lấy đối số thứ 0 (a0) từ trapframe
  myproc()->tracemask = mask;  // Lưu vào tiến trình đang gọi
  return 0;
}

uint64 sys_ptree(void) {
    uint64 buf_addr;
    int max;

    argaddr(0, &buf_addr);
    argint(1, &max);

    // Gọi hàm get_ptree
    return get_ptree(buf_addr, max);
}

uint64
sys_sysinfo(void)
{
  struct proc *p = myproc(); // Lấy con trỏ cấu trúc của tiến trình đang gọi syscall
  struct sysinfo info;       // Khai báo biến struct sysinfo 
  uint64 addr;               // Biến lưu địa chỉ ảo do người dùng truyền vào

  // Đọc đối số thứ nhất lưu vào biến addr
  argaddr(0, &addr);

  // Thu thập thông tin hệ thống và gán vào biến tạm của Kernel
  info.freemem = kcollect_freemem();
  info.nproc = count_active_procs();

  // Sao chép dữ liệu từ biến 'info' (Kernel) sang địa chỉ ảo 'addr' (User)
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1; // Trả về -1 báo hiệu thực thi thất bại

  return 0; // Trả về 0 báo hiệu thực thi thành công
}