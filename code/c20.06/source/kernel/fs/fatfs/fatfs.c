/**
 * 简单的FAT文件系统结构
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "fs/fs.h"
#include "fs/fatfs/fatfs.h"

/**
 * @brief 挂载fat文件系统
 */
int fatfs_mount (struct _fs_t * fs, int dev_major, int dev_minor) {
    return -1;
}

/**
 * @brief 卸载fatfs文件系统
 */
void fatfs_unmount (struct _fs_t * fs) {
}

/**
 * @brief 打开指定的文件
 */
int fatfs_open (struct _fs_t * fs, const char * path, file_t * file) {
	return -1;
}

/**
 * @brief 读了文件
 */
int fatfs_read (char * buf, int size, file_t * file) {
    return 0;
}

/**
 * @brief 写文件数据
 */
int fatfs_write (char * buf, int size, file_t * file) {
    return 0;
}

void fatfs_close (file_t * file) {

}

/**
 * @brief 文件读写位置的调整
 */
int fatfs_seek (file_t * file, uint32_t offset, int dir) {
    return -1;          // 不支持，只允许应用程序连续读取
}

int fatfs_stat (file_t * file, struct stat *st) {
    return -1;
}

fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .seek = fatfs_seek,
    .stat = fatfs_stat,
    .close = fatfs_close,
};