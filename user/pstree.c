#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/ptree.h"

#define MAX_PROC 64

void print_tree(struct ptreeinfo *buf, int count, int parent_pid, int depth) {
    for (int i = 0; i < count; i++) {
        if (buf[i].ppid == parent_pid) {
            for (int j = 0; j < depth; j++) {
                printf("  ");
            }

            printf("%d %s state=%d mem=%ld\n",
                   buf[i].pid, buf[i].name, buf[i].state, buf[i].memsize);
            print_tree(buf, count, buf[i].pid, depth + 1);
        }
    }
}

int main(int argc, char *argv[]) {
    struct ptreeinfo buf[MAX_PROC];

    int count = ptree(buf, MAX_PROC);
    if (count < 0) {
        printf("pstree: failed to get process tree\n");
        exit(1);
    }

    print_tree(buf, count, 0, 0);

    exit(0);
}
