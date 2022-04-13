
project(app LANGUAGES C)  

add_library(${PROJECT_NAME} lib_syscall.c lib_syscall.h)

