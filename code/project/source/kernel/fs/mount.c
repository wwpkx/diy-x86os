/**
 * 文件系统挂载
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/klib.h>
#include <dev/dev.h>
#include <fs/dir.h>
#include <fs/mount.h>

static mount_point_t mount_list[MOUNT_LIST_MAX_SIZE];	// 挂载列表

/**
 * 分配挂载点结构
 */
static mount_point_t * alloc_mount_point (const char * name, int dev) {
	for (int i = 0; i < MOUNT_LIST_MAX_SIZE; i++) {
		mount_point_t * p = mount_list + i;
		if (p->name[0] == '\0') {
			k_memset(p, 0, sizeof(mount_point_t));
			p->dev = dev;
			k_strncpy(p->name, name, MOUT_NAME_MAX_SIZE);
			return p;
		}
	}

	return (mount_point_t *)0;
}

/**
 * 释放挂载结构
 */
static void free_mount_point (mount_point_t * p) {
	p->name[0] = '\0';
}

/**
 * 挂载处理初始化
 */
void fs_mount_init (void) {
	k_memset(mount_list, 0, sizeof(mount_list));
}

/**
 * 添加xfat到链表中
 */
int fs_mount (const char * name, int dev) {
	mount_point_t * p = alloc_mount_point(name, dev);
	if (!p) {
		return -1;
	}

	// 根据设备类型，执行不同的挂载。主要处理磁盘
	switch (device_major(dev)) {
	case DEV_DISK: {
		int err = xfat_load(&p->fat, dev);
		if (err == 0) {
			return 0;
		}
		break;
	}
	case DEV_TTY:
		p->type = DEV_TTY;
		return 0;
	default:
		break;
	}

	// 失败，回收
	free_mount_point(p);
	return -1;
}

/**
 * 将指定的xfat从链表中移除
 */
void fs_mount_remove (int dev) {
	for (int i = 0; i < MOUNT_LIST_MAX_SIZE; i++) {
		mount_point_t * p = mount_list + i;
		if (p->name[0] && (p->dev == dev)) {
			p->name[0] = '\0';
			return;
		}
	}
}

/**
 * 找到指定的挂载点
 */
mount_point_t * fs_mount_find (int dev) {
	for (int i = 0; i < MOUNT_LIST_MAX_SIZE; i++) {
		mount_point_t * p = mount_list + i;
		if (p->name[0] && (p->dev == dev)) {
			return p;
		}
	}

	return (mount_point_t *)0;
}

/**
 * 根据名称找匹配的挂载点
 */
mount_point_t * fs_mount_find_name(const char * path) {
	path = skip_sep(path);
	for (int i = 0; i < MOUNT_LIST_MAX_SIZE; i++) {
		mount_point_t * mp = mount_list + i;
		if (k_strncmp(mp->name, path, MOUT_NAME_MAX_SIZE) == 0) {
			return mp;
		}
	}

	return (mount_point_t *)0;
}

/**
 * 获取挂载点的fat结构
 */
xfat_t * fs_mount_get_fat (int dev) {
	mount_point_t * p = fs_mount_find(dev);
	if (p) {
		return &p->fat;
	}

	return (xfat_t *)0;
}

