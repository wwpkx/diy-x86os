
project(app LANGUAGES C)  

file(GLOB C_LIST "*.c" "*.h" "*.S")
add_library(${PROJECT_NAME} ${C_LIST})
