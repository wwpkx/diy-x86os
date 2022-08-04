#ifndef DEVFS_H
#define DEV_FS_H

#include "fs/fs.h"

typedef struct _devfs_type_t {
    const char * name;
    int dev_type;
    int file_type;
}devfs_type_t;

#endif
