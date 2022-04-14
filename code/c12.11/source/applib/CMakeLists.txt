
project(app LANGUAGES C)  

include_directories(
    ${PROJECT_SOURCE_DIR}/../../../newlib/i686-elf/include
)

file(GLOB C_LIST "*.c" "*.h" "*.asm")
add_library(${PROJECT_NAME} ${C_LIST})
