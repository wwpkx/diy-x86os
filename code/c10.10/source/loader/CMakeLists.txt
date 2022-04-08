
project(loader LANGUAGES C)  

# 使用自定义的链接器
set(CMAKE_C_LINK_EXECUTABLE "i686-elf-ld <OBJECTS> -m elf_i386 -Ttext=0x8000 -o ${PROJECT_NAME}.elf")

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头
file(GLOB C_LIST "*.c" "*.h")
add_executable(${PROJECT_NAME} start.asm ${C_LIST})

# lbin文件生成，写入到image目录下
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJCOPY_TOOL} -O binary ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/../../image/${PROJECT_NAME}.bin)