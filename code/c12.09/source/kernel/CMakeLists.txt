
project(kernel LANGUAGES C)  

# 使用自定义的链接器
set(CMAKE_C_LINK_EXECUTABLE
    "i686-elf-ld <OBJECTS> -m elf_i386 -T ${PROJECT_SOURCE_DIR}/kernel.lds -o ${PROJECT_NAME}.elf")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头
file(GLOB_RECURSE C_LIST "*.c" "*.h" "*.asm")
add_executable(${PROJECT_NAME} ${C_LIST})

# lbin文件生成，写入到image目录下
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJCOPY_TOOL} -S ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/../../image/${PROJECT_NAME}.elf)