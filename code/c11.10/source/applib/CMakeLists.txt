
project(applib LANGUAGES C)  

# 将所有的汇编、C文件加入工程
file(GLOB C_LIST "*.c" "*.h")
add_library(${PROJECT_NAME} ${C_LIST})
