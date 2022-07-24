#ifndef DEVFS_H
#define DEVFS_H

#include "fs/fs.h"

/**
 * @brief 设备类型描述结构
 */
typedef struct _devfs_type_t {
    const char * name;
    int dev_type;
    int file_type;
}devfs_type_t;

#endif
