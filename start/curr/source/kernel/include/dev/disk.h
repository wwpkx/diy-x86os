#ifndef DISK_H
#define DISK_H

#include "comm/types.h"

#define DISK_NAME_SIZE  32
#define PART_NAME_SIZE  32
#define DISK_PRIMARY_PART_CNT   (4+1)
#define DISK_CNT        2

struct _disk_t;
typedef struct _partinfo_t {
    char name[PART_NAME_SIZE];
    struct _disk_t * disk;

    enum {
        FS_INVALID = 0x00,
        FS_FAT16_0 = 0x6,
        FS_FAT16_1 = 0xE,
    }type;

    int start_sector;
    int total_sector;
}partinfo_t;

typedef struct _disk_t {
    char name[DISK_NAME_SIZE];
    int sector_size;
    int sector_count;
    partinfo_t partinfo[DISK_PRIMARY_PART_CNT];
}disk_t;

void disk_init (void);

#endif
