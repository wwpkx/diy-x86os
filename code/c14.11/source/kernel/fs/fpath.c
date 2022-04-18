/**
 * 文件系统路径处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "fs/fpath.h"

static const char path_sep = '/';           // 路径分隔符

/**
 * 跳至下一有效名称
 */
const char * fpath_next(const char * path) {
	if (path == (const char *)0) {
		return (const char *)0;
	}

	// 定位到下一个分隔符的后边。可能是有效的名称或者字符串结束
	while (*path && *path++ != path_sep) {}
	return path;
}

