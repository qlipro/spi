/*
 * cmd_field.c
 *
 *  Created on: 2026年3月9日
 *      Author: jt
 */


#include "cmd_field.h"
#include "LCD.h"
#include "cursor.h"
#include <stdio.h>
#include <stdlib.h>

//// 全局变量
//SystemMode_t current_system_mode = SYS_MODE_NORMAL;
//CmdFieldContext_t cmd_ctx;
//HistoryContext_t hist_ctx;

// ==================== 定义指令字段树 ====================

// 第三级：具体操作指令
static const CmdOption_t cmd_level3_page[] = {
    {"Goto Page",   "page>",    0, 0, 1},  // 需要输入页码
    {"Next Page",   "page+",    0, 1, 0},  // 可执行
    {"Prev Page",   "page-",    0, 1, 0},  // 可执行
    {"Draft Page",  "page0",    0, 1, 0},  // 可执行
    {"Back",        "\b",       0, 0, 0}   // 退格返回上一级
};

static const CmdOption_t cmd_level3_clear[] = {
    {"Clear Page",  "clear",    0, 1, 0},
    {"Clear All",   "clearall", 0, 1, 0},
    {"Back",        "\b",       0, 0, 0}
};

static const CmdOption_t cmd_level3_save[] = {
    {"Save Draft",  "savedraft",0, 1, 0},
    {"Save As...",  "saveas>",  0, 0, 1},  // 需要输入文件名
    {"Back",        "\b",       0, 0, 0}
};

// 第二级：功能分类
static const CmdOption_t cmd_level2_options[] = {
    {"Page",        "",         1, 0, 0},  // 有子菜单
    {"Clear",       "",         1, 0, 0},
    {"Save",        "",         1, 0, 0},
    {"File",        "",         1, 0, 0},
    {"Back",        "\b",       0, 0, 0}
};

// 第二级对应的子菜单
static const CmdLevel_t cmd_level3[] = {
    {">Page",   5, cmd_level3_page,  0},   // Page子菜单
    {">Clear",  3, cmd_level3_clear, 0},   // Clear子菜单
    {">Save",   3, cmd_level3_save,  0},   // Save子菜单
    {">File",   0, NULL,             0},   // File子菜单（待实现）
    {">Back",   0, NULL,             0}    // Back选项
};

// 第一级：主菜单
static const CmdOption_t cmd_level1_options[] = {
    {"Command",     "",         1, 0, 0},  // 主指令入口
    {"Edit",        "",         1, 0, 0},
    {"View",        "",         1, 0, 0},
    {"Back",        "\b",       0, 0, 0}
};

// 第一级对应的子菜单
static const CmdLevel_t cmd_level2[] = {
    {">Cmd",    5, cmd_level2_options, 0},   // Command子菜单
    {">Edit",   0, NULL,              0},   // Edit子菜单
    {">View",   0, NULL,              0},   // View子菜单
    {">Back",   0, NULL,              0}    // Back选项
};

// 定义完整的指令层级树
static const CmdLevel_t cmd_tree[] = {
    {"CMD",     4, cmd_level1_options, 0},   // 第一级
    {NULL,      0, NULL,               0},   // 占位
    {NULL,      0, NULL,               0}    // 占位
};
