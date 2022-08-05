#ifndef FATFS_H
#define FATFS_H

#pragma pack(1)

typedef struct _dbr_t {
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;

    uint8_t BS_drvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
}dbr_t;

#pragma pack()

typedef struct _fat_t {
    uint32_t tbl_start;
    uint32_t tbl_cnt;
    uint32_t tbl_sectors;

    uint32_t bytes_per_sec;
    uint32_t sec_per_cluster;
    uint32_t root_start;
    uint32_t root_ent_cnt;
    uint32_t data_start;
    uint32_t cluster_byte_size;

    uint8_t * fat_buffer;

    struct _fs_t * fs;
}fat_t;

#endif
