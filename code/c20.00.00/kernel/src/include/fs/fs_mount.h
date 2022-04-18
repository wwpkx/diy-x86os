/**
 * 文件系统挂载
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_FS_FS_MOUNT_H_
#define SRC_INCLUDE_FS_FS_MOUNT_H_

#include <core/os_cfg.h>
#include <fs/fat.h>

/**
 * 挂载配置
 */
typedef struct _mount_point_t {
	char name[MOUT_NAME_MAX_SIZE];	// 挂载的名称
	int dev;								// 对应的设备号
	xfat_t fat;								// 文件系统配置数据

	enum {
		MOUNT_DISK,
		MOUNT_TTY,
	}type;

}mount_point_t;

void fs_mount_init (void);
int fs_mount (const char * name, int dev);
void fs_unmount(int dev);
mount_point_t * fs_mount_find (int dev);
mount_point_t * fs_mount_find_name(const char * path);
xfat_t * fs_mount_get_fat (int dev);

#endif /* SRC_INCLUDE_FS_FS_MOUNT_H_ */
