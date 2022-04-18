
project(kernel LANGUAGES C)  

# 使用自定义的链接器
# 加入相应的库
set(LIBS_FLAGS "-L ${CMAKE_BINARY_DIR}/source/applib/ -lapp")
set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386 -T ${PROJECT_SOURCE_DIR}/kernel.lds ${LIBS_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.elf  -b binary init.bin")

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头
file(GLOB_RECURSE C_LIST "*.c" "*.h" "*.S")
add_executable(${PROJECT_NAME} ${C_LIST})

# 不带调试信息的elf生成，何种更小，写入到image目录下
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJCOPY_TOOL} -S ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/../../image/${PROJECT_NAME}.elf)