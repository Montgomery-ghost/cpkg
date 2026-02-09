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

#define _POSIX_C_SOURCE 200809L  // 启用strdup, popen等POSIX函数

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <archive.h>
#include <archive_entry.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <stdbool.h>
#include <glob.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../include/help.h"
#include "../include/cpkg.h"

/**
 * @brief 检察 sudo 权限
 * @return 0 表示成功，非0表示失败
 */
int check_sudo_privileges(void) 
{
    if (geteuid() != 0) 
    {
        return 1; // 权限不足
    }
    return 0; // 成功
}

/**
 * @brief 创建目录树
 * @param path 目录路径
 * @return 0 表示成功，非0表示失败
 */
int mkdir_p(const char* path)
{
    if (!path || strlen(path) == 0) {
        return 1;
    }
    
    char* dir_copy = strdup(path);
    if (!dir_copy) {
        return 1;
    }
    
    char* current_pos = dir_copy;
    while (*current_pos != '\0') 
    {
        if (*current_pos == '/') 
        {
            *current_pos = '\0';
            if (strlen(dir_copy) > 0 && access(dir_copy, F_OK) != 0) 
            {
                if (mkdir(dir_copy, 0755) != 0) 
                {
                    free(dir_copy);
                    return 1;
                }
            }
            *current_pos = '/';
        }
        current_pos++;
    }
    
    // 创建最后一级目录
    if (strlen(dir_copy) > 0 && access(dir_copy, F_OK) != 0) 
    {
        if (mkdir(dir_copy, 0755) != 0) 
        {
            free(dir_copy);
            return 1;
        }
    }
    
    free(dir_copy);
    return 0;
}

/**
 * @brief 复制文件
 * @param src_path 源文件路径
 * @param dst_path 目标文件路径
 * @return 0 成功，非0失败
 */
int copy_file(const char* src_path, const char* dst_path)
{
    if (!src_path || !dst_path) {
        return 1;
    }
    
    FILE* src_file = fopen(src_path, "rb");
    if (!src_file) {
        return 1;
    }
    
    FILE* dst_file = fopen(dst_path, "wb");
    if (!dst_file) 
    {
        fclose(src_file);
        return 1;
    }
    
    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) 
    {
        if (fwrite(buffer, 1, bytes_read, dst_file) != bytes_read) 
        {
            fclose(src_file);
            fclose(dst_file);
            return 1;
        }
    }
    
    fclose(src_file);
    fclose(dst_file);
    return 0;
}

/**
 * @brief 计算文件的SHA256哈希值
 * @param file_path 文件路径
 * @param hash_buffer 哈希值输出缓冲区
 * @return 0 成功，其他失败
 */
int calculate_hash(const char* file_path, unsigned char* hash_buffer)
{
    if (!file_path || !hash_buffer) {
        return 1;
    }
    
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        return 1;
    }
    
    EVP_MD_CTX* md_context = EVP_MD_CTX_new();
    if (!md_context) 
    {
        fclose(file);
        return 1;
    }
    
    if (EVP_DigestInit_ex(md_context, EVP_sha256(), NULL) != 1) 
    {
        fclose(file);
        EVP_MD_CTX_free(md_context);
        return 1;
    }
    
    unsigned char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) 
    {
        if (EVP_DigestUpdate(md_context, buffer, bytes_read) != 1) 
        {
            fclose(file);
            EVP_MD_CTX_free(md_context);
            return 1;
        }
    }
    
    unsigned int hash_length = SHA256_DIGEST_LENGTH;
    if (EVP_DigestFinal_ex(md_context, hash_buffer, &hash_length) != 1) 
    {
        fclose(file);
        EVP_MD_CTX_free(md_context);
        return 1;
    }
    
    fclose(file);
    EVP_MD_CTX_free(md_context);
    return 0;
}

/**
 * @brief 手动实现通配符扩展
 * @param pattern 通配符模式
 * @param file_list 文件列表输出
 * @param max_files 最大文件数
 * @return 匹配的文件数量，-1表示错误
 */
int expand_wildcard(const char* pattern, char file_list[MAX_FILES][MAX_LINE_LEN], int max_files)
{
    if (!pattern) {
        return 0;
    }
    
    // 简单实现：如果pattern包含通配符，使用ls命令
    if (strchr(pattern, '*') || strchr(pattern, '?')) 
    {
        char command[MAX_PATH_LEN * 2];
        snprintf(command, sizeof(command), "ls -1 %s 2>/dev/null", pattern);
        
        FILE* ls_output = popen(command, "r");
        if (!ls_output) {
            return -1;
        }
        
        char file_name[MAX_LINE_LEN];
        int file_count = 0;
        
        while (fgets(file_name, sizeof(file_name), ls_output) && file_count < max_files) 
        {
            // 去除换行符
            size_t len = strlen(file_name);
            if (len > 0 && file_name[len-1] == '\n') {
                file_name[len-1] = '\0';
            }
            
            if (strlen(file_name) > 0) 
            {
                // 使用strncpy并确保以null结尾
                strncpy(file_list[file_count], file_name, MAX_LINE_LEN - 1);
                file_list[file_count][MAX_LINE_LEN - 1] = '\0';
                file_count++;
            }
        }
        
        pclose(ls_output);
        return file_count;
    } 
    else 
    {
        // 没有通配符，直接检查文件是否存在
        if (access(pattern, F_OK) == 0) 
        {
            strncpy(file_list[0], pattern, MAX_LINE_LEN - 1);
            file_list[0][MAX_LINE_LEN - 1] = '\0';
            return 1;
        }
        return 0;
    }
}

/**
 * @brief 递归获取目录中的所有文件
 * @param dir_path 目录路径
 * @param file_list 文件列表输出
 * @param max_files 最大文件数
 * @param file_count 当前文件计数
 * @return 匹配的文件数量
 */
int get_files_recursive(const char* dir_path, char file_list[MAX_FILES][MAX_LINE_LEN], int max_files, int file_count)
{
    DIR* dir = opendir(dir_path);
    if (!dir) {
        return file_count;
    }
    
    struct dirent* entry;
    char full_path[MAX_PATH_LEN];
    
    while ((entry = readdir(dir)) != NULL && file_count < max_files) 
    {
        // 跳过"."和".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat path_stat;
        if (stat(full_path, &path_stat) != 0) {
            continue;
        }
        
        if (S_ISDIR(path_stat.st_mode)) 
        {
            // 递归处理子目录
            file_count = get_files_recursive(full_path, file_list, max_files, file_count);
        } 
        else if (S_ISREG(path_stat.st_mode)) 
        {
            // 添加普通文件
            strncpy(file_list[file_count], full_path, MAX_LINE_LEN - 1);
            file_list[file_count][MAX_LINE_LEN - 1] = '\0';
            file_count++;
        }
    }
    
    closedir(dir);
    return file_count;
}

/**
 * @brief 安全的字符串复制，避免截断警告
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param dest_size 目标缓冲区大小
 */
static inline void safe_strcpy(char* dest, const char* src, size_t dest_size)
{
    if (dest_size == 0) return;
    
    size_t src_len = strlen(src);
    if (src_len >= dest_size) {
        // 截断并添加终止符
        memcpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    } else {
        strcpy(dest, src);
    }
}

/**
 * @brief 解析文件列表字符串，包括通配符扩展
 * @param list_str 列表字符串
 * @param file_list 文件列表输出
 * @param max_files 最大文件数
 * @return 解析的文件数量
 */
int parse_file_list(const char* list_str, char file_list[MAX_FILES][MAX_LINE_LEN], int max_files)
{
    if (!list_str || strlen(list_str) == 0) {
        return 0;
    }
    
    char* list_copy = strdup(list_str);
    if (!list_copy) {
        return 0;
    }
    
    int file_count = 0;
    
    // 检查是否有花括号
    if (strstr(list_copy, "{") != NULL && strstr(list_copy, "}") != NULL) 
    {
        // 处理花括号内的多个文件
        char* start = strchr(list_copy, '{');
        char* end = strchr(list_copy, '}');
        if (start && end && end > start) 
        {
            *end = '\0';
            char* content = start + 1;
            
            // 按逗号分割
            char* token = strtok(content, ",");
            while (token && file_count < max_files) 
            {
                // 去除token前后的空格和引号
                char* tok = token;
                while (*tok == ' ') tok++;
                char* tok_end = tok + strlen(tok) - 1;
                while (tok_end > tok && *tok_end == ' ') *tok_end-- = '\0';
                
                if (*tok == '"' && *tok_end == '"') {
                    *tok_end = '\0';
                    tok++;
                }
                
                // 检查通配符
                if (strchr(tok, '*') || strchr(tok, '?')) 
                {
                    int expanded_count = expand_wildcard(tok, &file_list[file_count], max_files - file_count);
                    if (expanded_count > 0) {
                        file_count += expanded_count;
                    } else {
                        // 如果没有匹配，直接添加原始路径
                        safe_strcpy(file_list[file_count], tok, MAX_LINE_LEN);
                        file_count++;
                    }
                } 
                else 
                {
                    safe_strcpy(file_list[file_count], tok, MAX_LINE_LEN);
                    file_count++;
                }
                
                token = strtok(NULL, ",");
            }
        }
    } 
    else 
    {
        // 单个文件或通配符，去除可能的引号
        char* clean_value = list_copy;
        while (*clean_value == ' ') clean_value++;
        char* end_value = clean_value + strlen(clean_value) - 1;
        while (end_value > clean_value && *end_value == ' ') *end_value-- = '\0';
        
        if (*clean_value == '"' && *end_value == '"') {
            *end_value = '\0';
            clean_value++;
        }
        
        // 检查通配符
        if (strchr(clean_value, '*') || strchr(clean_value, '?')) 
        {
            int expanded_count = expand_wildcard(clean_value, file_list, max_files);
            if (expanded_count > 0) {
                file_count += expanded_count;
            } else {
                // 如果没有匹配，直接添加原始路径
                safe_strcpy(file_list[file_count], clean_value, MAX_LINE_LEN);
                file_count++;
            }
        } 
        else 
        {
            safe_strcpy(file_list[file_count], clean_value, MAX_LINE_LEN);
            file_count++;
        }
    }
    
    free(list_copy);
    return file_count;
}

/**
 * @brief 压缩文件夹为 tar.gz 格式，并重命名为 .cpk 文件
 * @param src_dir 源文件夹路径
 * @param package_name 包名
 * @param output_dir 输出目录
 * @return 0 表示成功，非0表示失败
 */
int build_file(const char* src_dir, const char* package_name, const char* output_dir)
{
    if (!src_dir || !package_name || !output_dir) {
        return 1;
    }
    
    char output_path[MAX_PATH_LEN];
    snprintf(output_path, sizeof(output_path), "%s/%s.cpk", output_dir, package_name);
    
    struct archive* archive = archive_write_new();
    if (!archive) {
        return 1;
    }
    
    archive_write_set_format_pax_restricted(archive);
    archive_write_add_filter_gzip(archive);
    
    if (archive_write_open_filename(archive, output_path) != ARCHIVE_OK)
    {
        archive_write_free(archive);
        return 1;
    }
    
    // 构建CPK_Header结构体头部
    CPK_Header header;
    memset(&header, 0, sizeof(CPK_Header));
    memcpy(header.magic, CPK_MAGIC, CPK_MAGIC_LEN); // 设置魔术字
    
    // 这里应该计算整个包的哈希，但原逻辑是计算控制文件的哈希
    // 保持原逻辑不变
    char control_file_path[MAX_PATH_LEN];
    snprintf(control_file_path, sizeof(control_file_path), "%s/%s", src_dir, CONTROL_FILE);
    
    unsigned char hash_value[SHA256_DIGEST_LENGTH];
    if (calculate_hash(control_file_path, hash_value) != 0)
    {
        archive_write_close(archive);
        archive_write_free(archive);
        return 1;
    }
    
    // 复制哈希值到header
    memcpy(header.hash, hash_value, SHA256_DIGEST_LENGTH);
    
    // 写入CPK_Header结构体头部
    struct archive_entry* header_entry = archive_entry_new();
    archive_entry_set_pathname(header_entry, "CPKG_HEADER");
    archive_entry_set_size(header_entry, sizeof(CPK_Header));
    archive_entry_set_filetype(header_entry, AE_IFREG);
    archive_entry_set_perm(header_entry, 0644);
    archive_write_header(archive, header_entry);
    archive_write_data(archive, &header, sizeof(CPK_Header));
    archive_entry_free(header_entry);
    
    // 递归获取目录中的所有文件
    char file_list[MAX_FILES][MAX_LINE_LEN];
    int file_count = get_files_recursive(src_dir, file_list, MAX_FILES, 0);
    
    // 添加文件到压缩包
    for (int i = 0; i < file_count; i++) 
    {
        const char* file_path = file_list[i];
        struct stat file_stat;
        
        if (stat(file_path, &file_stat) != 0) {
            continue;
        }
        
        // 获取相对路径
        const char* relative_path = file_path + strlen(src_dir);
        if (*relative_path == '/') {
            relative_path++;
        }
        
        struct archive_entry* entry = archive_entry_new();
        archive_entry_set_pathname(entry, relative_path);
        archive_entry_copy_stat(entry, &file_stat);
        
        if (archive_write_header(archive, entry) == ARCHIVE_OK) 
        {
            if (S_ISREG(file_stat.st_mode)) 
            {
                FILE* file = fopen(file_path, "rb");
                if (file) 
                {
                    char buffer[8192];
                    size_t bytes_read;
                    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) 
                    {
                        archive_write_data(archive, buffer, bytes_read);
                    }
                    fclose(file);
                }
            }
        }
        
        archive_entry_free(entry);
    }
    
    archive_write_close(archive);
    archive_write_free(archive);
    
    return 0;
}

/**
 * @brief 解析控制文件
 * @param control_file 控制文件指针
 * @param control_info 输出参数，存储解析后的控制信息
 * @return 0 表示成功，非0表示失败
 */
int read_control_file(FILE* control_file, ControlInfo* control_info) 
{
    if (!control_file || !control_info) {
        return 1;
    }
    
    memset(control_info, 0, sizeof(ControlInfo));
    
    char line[MAX_LINE_LEN];
    
    while (fgets(line, sizeof(line), control_file)) 
    {
        // 去除换行符
        line[strcspn(line, "\n")] = '\0';
        
        // 跳过空行和注释
        char* trimmed_line = line;
        while (*trimmed_line == ' ' || *trimmed_line == '\t') trimmed_line++;
        
        if (*trimmed_line == '\0' || *trimmed_line == '#') {
            continue;
        }
        
        // 查找冒号分隔符
        char* colon_pos = strchr(trimmed_line, ':');
        if (!colon_pos) {
            continue; // 无效行
        }
        
        // 提取字段名和值
        char field_name[MAX_LINE_LEN];
        char field_value[MAX_LINE_LEN];
        
        strncpy(field_name, trimmed_line, colon_pos - trimmed_line);
        field_name[colon_pos - trimmed_line] = '\0';
        
        char* value_start = colon_pos + 1;
        while (*value_start == ' ' || *value_start == '\t') value_start++;
        
        // 复制值，但截断注释部分
        strncpy(field_value, value_start, MAX_LINE_LEN - 1);
        field_value[MAX_LINE_LEN - 1] = '\0';
        
        // 移除行内注释
        char* comment_pos = strchr(field_value, '#');
        if (comment_pos) {
            *comment_pos = '\0';
        }
        
        // 去除尾部空格
        char* end = field_value + strlen(field_value) - 1;
        while (end > field_value && (*end == ' ' || *end == '\t')) {
            *end-- = '\0';
        }
        
        // 去除字段名尾部的空白
        char* field_name_end = field_name + strlen(field_name) - 1;
        while (field_name_end > field_name && (*field_name_end == ' ' || *field_name_end == '\t')) {
            *field_name_end-- = '\0';
        }
        
        // 处理各个字段，支持多种字段名变体
        if (strcmp(field_name, "package") == 0 || strcmp(field_name, "packet") == 0) {
            safe_strcpy(control_info->package, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "version") == 0) {
            safe_strcpy(control_info->version, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "description") == 0) {
            safe_strcpy(control_info->description, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "auther") == 0 || strcmp(field_name, "author") == 0 || 
                 strcmp(field_name, "maintainer") == 0) {
            safe_strcpy(control_info->maintainer, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "license") == 0) {
            safe_strcpy(control_info->license, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "homepage") == 0) {
            safe_strcpy(control_info->homepage, field_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "include_header_files") == 0 || 
                 strcmp(field_name, "include") == 0 ||
                 strcmp(field_name, "includes") == 0) 
        {
            control_info->include_header_files_num = parse_file_list(field_value, 
                control_info->include_header_files, MAX_FILES);
        }
        else if (strcmp(field_name, "lib_files") == 0 || 
                 strcmp(field_name, "lib") == 0 ||
                 strcmp(field_name, "libs") == 0) 
        {
            control_info->lib_files_num = parse_file_list(field_value, 
                control_info->lib_files, MAX_FILES);
        }
        else if (strcmp(field_name, "include_install_dir") == 0 || 
                 strcmp(field_name, "include_path") == 0 ||
                 strcmp(field_name, "include_dir") == 0) 
        {
            // 去除引号
            char* cleaned_value = field_value;
            if (field_value[0] == '"' && field_value[strlen(field_value)-1] == '"') {
                field_value[strlen(field_value)-1] = '\0';
                cleaned_value = field_value + 1;
            }
            safe_strcpy(control_info->include_install_dir, cleaned_value, MAX_LINE_LEN);
        }
        else if (strcmp(field_name, "lib_install_dir") == 0 || 
                 strcmp(field_name, "lib_path") == 0 ||
                 strcmp(field_name, "lib_dir") == 0) 
        {
            // 去除引号
            char* cleaned_value = field_value;
            if (field_value[0] == '"' && field_value[strlen(field_value)-1] == '"') {
                field_value[strlen(field_value)-1] = '\0';
                cleaned_value = field_value + 1;
            }
            safe_strcpy(control_info->lib_install_dir, cleaned_value, MAX_LINE_LEN);
        }
    }
    
    return 0;
}

/**
 * @brief 打印控制信息
 * @param control_info 控制信息结构体指针
 */
void print_control_info(ControlInfo* control_info) 
{
    if (!control_info) {
        return;
    }
    
    printf("Package: %s\n", control_info->package);
    printf("Version: %s\n", control_info->version);
    printf("Description: %s\n", control_info->description);
    printf("Maintainer: %s\n", control_info->maintainer);
    printf("License: %s\n", control_info->license);
    printf("Homepage: %s\n", control_info->homepage);
    printf("Include header files (%d):\n", control_info->include_header_files_num);
    for (int i = 0; i < control_info->include_header_files_num; i++) 
    {
        printf("  [%d] %s\n", i, control_info->include_header_files[i]);
    }
    printf("Library files (%d):\n", control_info->lib_files_num);
    for (int i = 0; i < control_info->lib_files_num; i++) 
    {
        printf("  [%d] %s\n", i, control_info->lib_files[i]);
    }
    printf("Include install directory: %s\n", control_info->include_install_dir);
    printf("Library install directory: %s\n", control_info->lib_install_dir);
}