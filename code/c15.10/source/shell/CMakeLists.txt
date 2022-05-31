
project(shell LANGUAGES C)  

# 使用自定义的链接器
# 加入相应的库
set(LIBS_FLAGS "-L ${CMAKE_BINARY_DIR}/source/applib/ -lapp -L ${CMAKE_BINARY_DIR}/../../newlib/i686-elf/lib -lm -lc")
set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386 -T ${PROJECT_SOURCE_DIR}/link.lds ${LIBS_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${CMAKE_SOURCE_DIR}/../../image/${PROJECT_NAME}.elf")

include_directories(
    ${PROJECT_SOURCE_DIR}/../applib/
)

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头
file(GLOB C_LIST "*.c" "*.h")
add_executable(${PROJECT_NAME} ${C_LIST})
