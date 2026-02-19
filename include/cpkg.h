/*
 * Copyright (C) 2025 lemonade_NingYou
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CPKG_H
#define CPKG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_PATH_LEN 1024 // 最大路径长度
#define CONTROL "control" // 控制文件名
#define META_DIR_NAME "CPKG" // 元数据文件名
#define WORK_DIR_NAME "cpkg-work" // 工作目录名
#define INSTALL_DIR "installed" // 安装目录名

typedef struct {
    char name[MAX_PATH_LEN]; // 包名
    char version[MAX_PATH_LEN]; // 版本号
    char description[MAX_PATH_LEN]; // 描述
    char homepage[MAX_PATH_LEN]; // 主页
    char author[MAX_PATH_LEN]; // 作者
    char license[MAX_PATH_LEN]; // 许可证
    char include_install_path[MAX_PATH_LEN]; // 安装路径
    char lib_install_path[MAX_PATH_LEN]; // 构建路径
    char **include_files; // 头文件列表
    char **lib_files; // 库文件列表
    int include_file_count; // 头文件数量
    int lib_file_count; // 库文件数量
} Control_Info;

typedef struct {
    char magic[4]; // 魔数
    char hash[65]; // 哈希值
    char name[MAX_PATH_LEN]; // 包名
    char version[MAX_PATH_LEN]; // 版本号
    char description[MAX_PATH_LEN]; // 描述
    char homepage[MAX_PATH_LEN]; // 主页
    char author[MAX_PATH_LEN]; // 作者
    char license[MAX_PATH_LEN]; // 许可证
    char include_install_path[3*MAX_PATH_LEN]; // 安装路径
    char lib_install_path[3*MAX_PATH_LEN]; // 构建路径
} CPK_Header;

int check_sudo_privileges(void); // 检查是否有root权限
int tf_choose(const char *msg); // 选择yes或no
int mkdir_p(const char *path, mode_t mode); // 创建目录
int cp_file(const char *src_file, const char *dst_dir); // 复制文件
int rm_rf(const char *del_dir); // 删除目录
int extract_archive(FILE *fp, const char *dest); // 解压tar.gz压缩包
char *archive_create_tgz(const char *src_dir, size_t *out_len); // 创建tar.gz压缩包
CPK_Header *make_Header(Control_Info *ctrl_info); // 创建CPK头文件
char *sha256_mem(const unsigned char *data, size_t len); // 计算哈希值
Control_Info *read_control_info(FILE *fp); // 读取控制文件
void printf_control_info(Control_Info *ctrl_info); // 打印控制信息
off_t get_file_size(const char *path); // 获取文件大小

int install_package(const char *pkg_path);
int remove_package(const char *pkg_name);
int make_build_package(const char *package_path_dir);

#endif // CPKG_H