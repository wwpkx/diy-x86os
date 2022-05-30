
project(app LANGUAGES C)  

file(GLOB C_LIST "*.c" "*.h" "*.asm")
add_library(${PROJECT_NAME} ${C_LIST})
