#include "init.h"
#include "comm/boot_info.h"

int global_var = 0x1234;
int globa_var_zero;
static int static_global_var = 0x2345;
static int static_global_var_zero;

const int const_int = 0x33;
const char * str = "abcdefg";

void kernel_init (boot_info_t * boot_info) {
    int locar_var;
    static int static_local_var = 0x33;
    static int static_local_var_zero;
    
    for (;;) {}
}