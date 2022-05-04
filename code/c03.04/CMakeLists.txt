cmake_minimum_required(VERSION 3.0.0)

# 保存一些通用的配置
set(CMAKE_VERBOSE_MAKEFILE on)

# gcc编译器与参数配置
set(CMAKE_C_COMPILER "i686-elf-gcc") 
set(CMAKE_C_FLAGS "-c -O0 -m32 -fno-pie -fno-asynchronous-unwind-tables")

# 汇编器与参数配置
set(CMAKE_ASM_COMPILER "i686-elf-as")
set(CMAKE_ASM_FLAGS "--32 -g")
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "asm")

# 其它工具
set(OBJCOPY_TOOL "i686-elf-objcopy")

# 工程，启用C语言和汇编语言
project(os LANGUAGES C)  
enable_language(ASM)

include_directories(
    ${PROJECT_SOURCE_DIR}/source
)

# 底层的若干子项目
add_subdirectory(./source/boot)
add_subdirectory(./source/loader)
# add_subdirectory(./source/kernel)

