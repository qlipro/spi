/*
 * cmd_field.h
 *
 *  Created on: 2026年3月9日
 *      Author: jt
 */

#ifndef INC_CMD_FIELD_H_
#define INC_CMD_FIELD_H_

#include "stdint.h"
#include "string.h"

// 指令字段最大层级
#define MAX_CMD_LEVELS      5
// 每层最大选项数
#define MAX_OPTIONS_PER_LEVEL   10
// 指令缓冲区大小
#define CMD_BUFFER_SIZE     50

// 指令模式状态
typedef enum {
    CMD_MODE_NORMAL = 0,    // 正常输入模式
    CMD_MODE_INPUT,         // 指令输入模式
    CMD_MODE_COMPLETE       // 指令完成待执行
} CmdMode_t;

// 指令字段选项结构
typedef struct {
    const char *text;           // 选项显示文本
    const char *value;          // 选项实际值（用于拼接指令）
    uint8_t has_submenu;        // 是否有子菜单
    uint8_t is_executable;      // 是否为可执行指令（最后一级）
} CmdOption_t;

// 指令层级结构
typedef struct {
    const char *prompt;                     // 当前层级提示符
    uint8_t option_count;                    // 选项数量
    const CmdOption_t *options;              // 选项数组
    uint8_t default_option;                   // 默认选项索引
} CmdLevel_t;

// 指令字段上下文
typedef struct {
    CmdMode_t mode;                          // 当前模式
    uint8_t current_level;                    // 当前层级
    uint8_t current_option[MAX_CMD_LEVELS];   // 每层选中的选项索引
    char cmd_buffer[CMD_BUFFER_SIZE];         // 构建的指令缓冲区
    uint8_t cmd_len;                          // 指令长度

    // 编码器相关
    int16_t last_position;                     // 上次编码器位置
    uint32_t last_rotate_time;                  // 上次旋转时间
    uint8_t timer_active;                       // 定时器激活标志

    // 显示相关
    uint8_t preview_x;                          // 预览X坐标
    uint8_t preview_y;                          // 预览Y坐标
    uint8_t mode_indicator_x;                    // 模式指示X坐标
    uint8_t mode_indicator_y;                    // 模式指示Y坐标

    // 回调函数
    void (*cmd_complete_callback)(const char *cmd);  // 指令完成回调
} CmdFieldContext_t;

// 全局指令上下文
extern CmdFieldContext_t cmd_ctx;

#endif /* INC_CMD_FIELD_H_ */
