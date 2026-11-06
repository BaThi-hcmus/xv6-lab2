#ifndef _PTREE_H_
#define _PTREE_H_

#include "types.h" // định nghĩa kiểu uint64

struct ptreeinfo {
    int pid;
    int ppid;
    int state;
    uint64 memsize;
    char name[16];
};

#endif