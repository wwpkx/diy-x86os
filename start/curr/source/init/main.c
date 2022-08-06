#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_syscall.h"
#include "main.h"
#include <getopt.h>
#include <sys/file.h>
#include "fs/file.h"
#include "dev/tty.h"

int main (int argc, char ** argv) {
    //int a = 3 / 0;
    *(char *)0 = 0x1234;
    return 0;
}
