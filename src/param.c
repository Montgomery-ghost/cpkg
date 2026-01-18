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
#include <getopt.h>
#include "../include/cpkg.h"
#include "../include/param.h"

// 短选项字符串定义
const char *short_options = "hi:r:AVs:p:L:l:S:C:D:P";

// 长选项结构体数组
struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"install", required_argument, 0, 'i'},
    {"unpack", required_argument, 0, 0},
    {"record-avail", required_argument, 0, 'A'},
    {"configure", required_argument, 0, 0},
    {"triggers-only", required_argument, 0, 0},
    {"remove", required_argument, 0, 'r'},
    {"purge", required_argument, 0, 'P'},
    {"verify", required_argument, 0, 'V'},
    {"get-selections", optional_argument, 0, 0},
    {"set-selections", no_argument, 0, 0},
    {"clear-selections", no_argument, 0, 0},
    {"update-avail", required_argument, 0, 0},
    {"merge-avail", required_argument, 0, 0},
    {"clear-avail", no_argument, 0, 0},
    {"forget-old-unavail", no_argument, 0, 0},
    {"status", required_argument, 0, 's'},
    {"print-avail", required_argument, 0, 'p'},
    {"listfiles", required_argument, 0, 'L'},
    {"list", optional_argument, 0, 'l'},
    {"search", required_argument, 0, 'S'},
    {"audit", optional_argument, 0, 'C'},
    {"yet-to-unpack", no_argument, 0, 0},
    {"predep-package", no_argument, 0, 0},
    {"add-architecture", required_argument, 0, 0},
    {"remove-architecture", required_argument, 0, 0},
    {"print-architecture", no_argument, 0, 0},
    {"print-foreign-architectures", no_argument, 0, 0},
    {"compare-versions", required_argument, 0, 0},
    {"force-help", no_argument, 0, 0},
    {"debug", optional_argument, 0, 'D'},
    {0, 0, 0, 0}
};

/**
 * @brief 
 */
int validate_required_argument(const char *optarg, const char *option_name) 
{
    return (optarg != NULL && strlen(optarg) > 0);
}

/**
 * @brief 
 */
ParseResult parse_options(int argc, char *argv[]) 
{
    ParseResult result = {ACTION_UNKNOWN, NULL, NULL, 0, NULL};
    int opt;
    int option_index = 0;
    
    // 重置 getopt
    opterr = 0;  // 重要：禁止 getopt 自动打印错误信息
    optind = 1;
    
    // 解析命令行选项
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) 
    {
        if (opt == 0) 
        {
            // 处理长选项但没有短选项的情况
            const char *name = long_options[option_index].name;
            result.option_name = name;
            
            // 映射长选项到操作类型
            if (strcmp(name, "unpack") == 0) 
            {
                result.action_type = ACTION_UNPACK;
                if (!validate_required_argument(optarg, name)) 
                {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } 
            else if (strcmp(name, "configure") == 0) 
            {
                result.action_type = ACTION_CONFIGURE;
                if (!validate_required_argument(optarg, name)) 
                {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } 
            else if (strcmp(name, "triggers-only") == 0) 
            {
                result.action_type = ACTION_TRIGGERS_ONLY;
                if (!validate_required_argument(optarg, name)) 
                {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "get-selections") == 0) {
                result.action_type = ACTION_GET_SELECTIONS;
            } else if (strcmp(name, "set-selections") == 0) {
                result.action_type = ACTION_SET_SELECTIONS;
            } else if (strcmp(name, "clear-selections") == 0) {
                result.action_type = ACTION_CLEAR_SELECTIONS;
            } else if (strcmp(name, "update-avail") == 0) {
                result.action_type = ACTION_UPDATE_AVAIL;
                if (!validate_required_argument(optarg, name)) {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "merge-avail") == 0) {
                result.action_type = ACTION_MERGE_AVAIL;
                if (!validate_required_argument(optarg, name)) {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "clear-avail") == 0) {
                result.action_type = ACTION_CLEAR_AVAIL;
            } else if (strcmp(name, "forget-old-unavail") == 0) {
                result.action_type = ACTION_FORGET_OLD_UNAVAIL;
            } else if (strcmp(name, "yet-to-unpack") == 0) {
                result.action_type = ACTION_YET_TO_UNPACK;
            } else if (strcmp(name, "predep-package") == 0) {
                result.action_type = ACTION_PREDEP_PACKAGE;
            } else if (strcmp(name, "add-architecture") == 0) {
                result.action_type = ACTION_ADD_ARCHITECTURE;
                if (!validate_required_argument(optarg, name)) {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "remove-architecture") == 0) {
                result.action_type = ACTION_REMOVE_ARCHITECTURE;
                if (!validate_required_argument(optarg, name)) {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "print-architecture") == 0) {
                result.action_type = ACTION_PRINT_ARCHITECTURE;
            } else if (strcmp(name, "print-foreign-architectures") == 0) {
                result.action_type = ACTION_PRINT_FOREIGN_ARCHITECTURES;
            } else if (strcmp(name, "compare-versions") == 0) {
                result.action_type = ACTION_COMPARE_VERSIONS;
                if (!validate_required_argument(optarg, name)) {
                    result.has_error = 1;
                    result.error_msg = "requires an argument";
                    return result;
                }
            } else if (strcmp(name, "force-help") == 0) {
                result.action_type = ACTION_FORCE_HELP;
            }
            result.arg = optarg;
            return result;
            
        } else if (opt == '?') {
            // 未知选项或缺少必要参数
            result.action_type = ACTION_ERROR;
            result.has_error = 1;
            result.error_msg = "unknown option or missing argument";
            return result;
            
        } else {
            // 处理有短选项的情况
            result.option_name = (opt == 'h') ? "help" : 
                                (opt == 'i') ? "install" :
                                (opt == 'r') ? "remove" :
                                (opt == 'A') ? "record-avail" :
                                (opt == 'V') ? "verify" :
                                (opt == 's') ? "status" :
                                (opt == 'p') ? "print-avail" :
                                (opt == 'L') ? "listfiles" :
                                (opt == 'l') ? "list" :
                                (opt == 'S') ? "search" :
                                (opt == 'C') ? "audit" :
                                (opt == 'D') ? "debug" :
                                (opt == 'P') ? "purge" : "unknown";
            
            // 映射短选项到操作类型
            switch (opt) {
                case 'h':
                    result.action_type = ACTION_HELP;
                    break;
                case 'i':
                    result.action_type = ACTION_INSTALL;
                    if (!validate_required_argument(optarg, "--install")) {
                        result.has_error = 1;
                        result.error_msg = "requires a package file argument";
                        return result;
                    }
                    break;
                case 'r':
                    result.action_type = ACTION_REMOVE;
                    if (!validate_required_argument(optarg, "--remove")) {
                        result.has_error = 1;
                        result.error_msg = "requires a package name argument";
                        return result;
                    }
                    break;
                case 'A':
                    result.action_type = ACTION_RECORD_AVAIL;
                    if (!validate_required_argument(optarg, "--record-avail")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 'V':
                    result.action_type = ACTION_VERIFY;
                    if (!validate_required_argument(optarg, "--verify")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 's':
                    result.action_type = ACTION_STATUS;
                    if (!validate_required_argument(optarg, "--status")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 'p':
                    result.action_type = ACTION_PRINT_AVAIL;
                    if (!validate_required_argument(optarg, "--print-avail")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 'L':
                    result.action_type = ACTION_LISTFILES;
                    if (!validate_required_argument(optarg, "--listfiles")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 'l':
                    result.action_type = ACTION_LIST;
                    break;
                case 'S':
                    result.action_type = ACTION_SEARCH;
                    if (!validate_required_argument(optarg, "--search")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                case 'C':
                    result.action_type = ACTION_AUDIT;
                    break;
                case 'D':
                    result.action_type = ACTION_DEBUG;
                    break;
                case 'P':
                    result.action_type = ACTION_PURGE;
                    if (!validate_required_argument(optarg, "--purge")) {
                        result.has_error = 1;
                        result.error_msg = "requires an argument";
                        return result;
                    }
                    break;
                default:
                    result.action_type = ACTION_UNKNOWN;
                    result.has_error = 1;
                    result.error_msg = "unknown option";
                    return result;
            }
            result.arg = optarg;
            return result;
        }
    }
    
    // 如果没有找到任何选项
    result.action_type = ACTION_UNKNOWN;
    return result;
}