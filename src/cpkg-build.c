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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/cpkg.h"
#include "../include/help.h"

/** 
 * @brief 构建软件包
 * @param build_path 构建路径
 * @return 0 成功，其他失败
 */
int build_package(const char* build_path)
{
    // 1. 读取控制文件
    ControlInfo control_info = {0};

    char control_file_path[MAX_PATH_LEN]; // 控制文件路径
    snprintf(control_file_path, sizeof(control_file_path), "%s/%s/%s", build_path, META_DIR, CONTROL_FILE);
    FILE* control_file = fopen(control_file_path, "r");
    if (!control_file) {
        cpk_printf(ERROR, "Failed to open control file: %s\n", control_file_path);
        return 1;
    }

    // 解析控制文件
    if(read_control_file(control_file, &control_info) != 0) 
    {
        cpk_printf(ERROR, "Failed to parse control file: %s\n", control_file_path);
        fclose(control_file);
        return 1;
    }
    fclose(control_file);

    // 打印控制信息
    print_control_info(&control_info);

    // 检查必需字段
    if (strlen(control_info.package) == 0) {
        cpk_printf(ERROR, "Package name is empty!\n");
        return 1;
    }

    // 1. 创建构建目录
    char install_dir[MAX_PATH_LEN]; // 构建目录
    snprintf(install_dir, sizeof(install_dir), "%s/%s-build", build_path, control_info.package);
    if(mkdir_p(install_dir))
    {
        cpk_printf(ERROR, "Failed to create install directory: %s\n", install_dir); 
        return 1;
    }
    
    // 创建control_info结构体的二进制文件，写入安装目录
    char control_info_file_path[MAX_PATH_LEN]; // 控制文件路径
    snprintf(control_info_file_path, sizeof(control_info_file_path), "%s/%s", install_dir, CONTROL_FILE);
    control_file = fopen(control_info_file_path, "wb");
    if (!control_file) 
    {
        cpk_printf(ERROR, "Failed to open control file: %s\n", control_info_file_path);
        return 1;
    }

    // 写入控制文件
    if(fwrite(&control_info, sizeof(ControlInfo), 1, control_file) != 1) 
    {
        cpk_printf(ERROR, "Failed to write control file: %s\n", control_info_file_path);
        fclose(control_file);
        return 1;
    }
    fclose(control_file);

    // 2. 构建软件包
    char include[MAX_PATH_LEN]; // 头文件安装目录
    char lib[MAX_PATH_LEN]; // 库文件安装目录
    snprintf(include, sizeof(include), "%s/include", install_dir);
    snprintf(lib, sizeof(lib), "%s/lib", install_dir);
    if(mkdir_p(include))
    {
        cpk_printf(ERROR, "Failed to create include directory: %s\n", include);
        return 1;
    }
    if(mkdir_p(lib))
    {
        cpk_printf(ERROR, "Failed to create lib directory: %s\n", lib);
        return 1;
    }

    // 复制头文件和库文件到安装目录
    for(int i = 0; i < control_info.include_header_files_num; i++)
    {
        char dst[MAX_PATH_LEN]; // 头文件目标路径
        // 提取头文件名
        const char* header_filename = strrchr(control_info.include_header_files[i], '/');
        if (header_filename) {
            header_filename++; // 跳过 '/'
        } else {
            header_filename = control_info.include_header_files[i]; // 没有 '/'，直接使用文件名
        }
        snprintf(dst, sizeof(dst), "%s/%s", include, header_filename);
        if(copy_file(control_info.include_header_files[i], dst))
        {
            cpk_printf(ERROR, "Failed to copy header file: %s to %s\n", control_info.include_header_files[i], dst);
            return 1;
        }
    }
    for(int i = 0; i < control_info.lib_files_num; i++)
    {
        char dst[MAX_PATH_LEN]; // 库文件目标路径
        // 提取库文件名
        const char* lib_filename = strrchr(control_info.lib_files[i], '/');
        if (lib_filename) {
            lib_filename++; // 跳过 '/'
        } else {
            lib_filename = control_info.lib_files[i]; // 没有 '/'，直接使用文件名
        }
        snprintf(dst, sizeof(dst), "%s/%s", lib, lib_filename);
        if(copy_file(control_info.lib_files[i], dst))
        {
            cpk_printf(ERROR, "Failed to copy lib file: %s to %s\n", control_info.lib_files[i], dst);
            return 1;
        }
    }

    // 3. 开始压缩为 tar.gz 包并重命名为.cpk
    // 注意：build_file 的第一个参数应该是要压缩的目录（install_dir），而不是build_path
    if(build_file(install_dir, control_info.package, build_path))
    {
        cpk_printf(ERROR, "Failed to build package: %s\n", control_info.package);
        return 1;
    }

    // 4. 打印构建成功信息,并清理install_dir
    cpk_printf(INFO, "Build package %s successfully.\n", control_info.package);
    
    // 清理安装目录
    char rm_cmd[MAX_COMMAND_LEN];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", install_dir);
    system(rm_cmd);

    return 0;
}