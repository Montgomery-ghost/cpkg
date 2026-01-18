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

#ifndef PARAM_H
#define PARAM_H

#include <getopt.h>

/** 
 * @brief 长选项结构体 
 */
extern struct option long_options[];

/** 
 * @brief 短选项字符串 
 */
extern const char *short_options;

/** 
 * @brief 解析结果结构体 
 */
typedef struct {
    int action_type;        // 操作类型
    const char *arg;        // 参数（如果有）
    const char *option_name; // 选项名称
    int has_error;          // 是否有错误
    const char *error_msg;  // 错误信息
} ParseResult;

/** 
 * @brief 操作类型枚举 
 */
typedef enum {
    ACTION_HELP,
    ACTION_INSTALL,
    ACTION_REMOVE,
    ACTION_RECORD_AVAIL,
    ACTION_VERIFY,
    ACTION_STATUS,
    ACTION_PRINT_AVAIL,
    ACTION_LISTFILES,
    ACTION_LIST,
    ACTION_SEARCH,
    ACTION_AUDIT,
    ACTION_DEBUG,
    ACTION_PURGE,
    ACTION_UNPACK,
    ACTION_CONFIGURE,
    ACTION_TRIGGERS_ONLY,
    ACTION_GET_SELECTIONS,
    ACTION_SET_SELECTIONS,
    ACTION_CLEAR_SELECTIONS,
    ACTION_UPDATE_AVAIL,
    ACTION_MERGE_AVAIL,
    ACTION_CLEAR_AVAIL,
    ACTION_FORGET_OLD_UNAVAIL,
    ACTION_YET_TO_UNPACK,
    ACTION_PREDEP_PACKAGE,
    ACTION_ADD_ARCHITECTURE,
    ACTION_REMOVE_ARCHITECTURE,
    ACTION_PRINT_ARCHITECTURE,
    ACTION_PRINT_FOREIGN_ARCHITECTURES,
    ACTION_COMPARE_VERSIONS,
    ACTION_FORCE_HELP,
    ACTION_UNKNOWN,
    ACTION_ERROR
} ActionType;

/** 
 * @brief 解析命令行选项 
 * @param argc 参数个数 
 * @param argv 参数数组 
 * @return 解析结果结构体 
 */
ParseResult parse_options(int argc, char *argv[]);

/** 
 * @brief 验证必需的参数 
 * @param optarg 选项参数 
 * @param option_name 选项名称 
 * @return 1 如果参数有效，否则 0 
 */
int validate_required_argument(const char *optarg, const char *option_name);

#endif