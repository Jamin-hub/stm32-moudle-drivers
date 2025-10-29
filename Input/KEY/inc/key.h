#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#include <stdbool.h>

// 按键状态枚举
typedef enum {
  KEY_IDLE,    // 空闲
  KEY_PRESSED, // 按下（去抖中）
  KEY_HOLD,    // 持续按下
  KEY_RELEASED // 释放（去抖中）
} KeyState;

// 按键事件枚举
typedef enum {
  KEY_EVENT_NONE,  // 无事件
  KEY_EVENT_SHORT, // 短按
  KEY_EVENT_LONG   // 长按
} KeyEvent;

// 按键事件返回结构体
typedef struct {
  uint8_t  key_id; // 按键ID
  KeyEvent event;  // 事件类型
} KeyEvent_t;

// 按键结构体
typedef struct {
  GPIO_TypeDef *port;         // GPIO端口
  uint16_t      pin;          // GPIO引脚
  bool          active_level; // 按下时的有效电平（true: 高电平, false: 低电平）
  KeyState      state;        // 当前状态
  uint32_t      timer;        // 计时器
  bool          last_level;   // 上次电平
} Key_t;

void       Key_Init(void);
KeyEvent_t Key_Scan(void);

#endif
