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
#include <stdint.h>
#include <openssl/sha.h>

// ========================== 魔术字定义 ==========================
#define CPK_MAGIC "CPK\x01"             // CPK 魔术字
#define CPK_MAGIC_LEN 4                 // 魔术字长度  
#define CPK_HEADER_SIZE sizeof(CPK_Header)  // 头部大小

// ========================== 通用常量 ==========================
#define MAX_PATH_LEN 1024               // 最大路径长度
#define MAX_LINE_LEN 4096                // 最大行长度,应该没人能写这么长吧？
#define MAX_FILES 256                // 最大文件数量，超出的人你在写linux?
#define CPK_INCLUDE_DIR "/usr/include" // 默认 include 安装目录
#define CPK_LIB_DIR "/usr/lib"         // 默认 lib 安装目录
#define WORK_DIR "/usr/bin/cpkg"       // 工作目录
#define INSTALL_DIR "installed"   // 软件包安装目录
#define INSTALLED_LOG_FILE "installed.file"   // 已安装包文件
#define MAX_COMMAND_LEN 4096            // 最大命令长度
#define META_DIR "CPKG"               // 元数据目录
#define CONTROL_FILE "control"          // 包含软件包最基本的元数据

// ========================== 结构体定义 ==========================
typedef struct {
    int include_header_files_num; // 头文件数量
    int lib_files_num; // 库文件数量
    char package[MAX_LINE_LEN];    // 软件包名
    char version[MAX_LINE_LEN]; // 软件包版本
    char description[MAX_LINE_LEN]; // 软件包描述
    char maintainer[MAX_LINE_LEN]; // 维护者信息
    char license[MAX_LINE_LEN]; // 许可证类型
    // char dependencies[MAX_LINE_LEN]; // 依赖关系，逗号分隔
    char homepage[MAX_LINE_LEN]; // 主页链接
    // char section[MAX_LINE_LEN]; // 软件包分类
    // char architecture[MAX_LINE_LEN]; // 软件包架构
    char include_header_files[MAX_FILES][MAX_LINE_LEN]; // 头文件位置
    char lib_files[MAX_FILES][MAX_LINE_LEN]; // 库文件位置
    //char bin_files[MAX_FILES][MAX_LINE_LEN]; // 可执行文件位置
    // char data_files[MAX_FILES][MAX_LINE_LEN]; // 数据文件位置
    char include_install_dir[MAX_LINE_LEN]; // 头文件安装地点
    char lib_install_dir[MAX_LINE_LEN]; // 库文件安装地点
} ControlInfo;

// ========================== 数据结构 ===========================
// ========================== 数据结构 ===========================
typedef struct {
    char magic[CPK_MAGIC_LEN]; // 魔术字
    unsigned char hash[SHA256_DIGEST_LENGTH]; // 哈希值
    ControlInfo control_info; // 控制信息
} CPK_Header;

// ======================== 内部函数声明 =========================
int check_sudo_privileges(void); // 检查 sudo 权限
int mkdir_p(const char* path); // 创建目录树
int copy_file(const char* src_path_file, const char* dst_path_file); // 复制文件
int build_file(const char* src_path_file, const char* dst_path_file, const char* build_path); // 构建文件
int read_control_file(FILE* control_file, ControlInfo* control_info); // 解析控制文件
void print_control_info(ControlInfo* control_info); // 打印控制信息

// ========================== 函数声明 ===========================
int info_package(const char *pkg_path);         // 查询软件包信息
int install_package(const char *pkg_path);      // 安装单个软件包
int remove_package(const char *pkg_name);       // 移除单个软件包
int build_package(const char *build_path);        // 构建软件包

#endif // CPKG_H