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
#include "../include/cpkg.h"
#include "../include/help.h"

/**
 * @brief 安装单个软件包
 * @param pkg_path 软件包路径
 * @return 0 表示成功，非0表示失败
 */
int install_package(const char *pkg_path) 
{ 
    // 检查软件包文件是否存在
    FILE *pkg_file = fopen(pkg_path, "rb");
    if (!pkg_file) 
    {
        cpk_printf(ERROR, "Package file '%s' not found.\n", pkg_path);
        return 1; // 返回1表示失败
    }

    fclose(pkg_file); // 关闭软件包文件
    return 0;
}