#include <stdio.h>

int main (void) {
    int count = 3;

    printf("Hello\n");

    int pid = fork();
    if (pid < 0) {
        printf("create child proc failed.\n");
    } else if (pid == 0) {
        printf("child: %d\n", count);
        char * argv[] = {"arg0", "arg1", "arg2", "arg3"};
        execve("echo", argv, (char **)0);
    } else {
        printf("child task id=%d\n", pid);
        printf("parent: %d\n", count);
    }

    return 0;
} 

