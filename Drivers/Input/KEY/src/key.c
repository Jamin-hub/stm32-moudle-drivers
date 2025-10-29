#include "key.h"

#define KEY_NUM         3
#define LONG_PRESS_TIME 1000
#define DEBOUNCE_TIME   20

Key_t keys[KEY_NUM];

/**
 * @brief 初始化按键接口参数
 * @param active_level 按下时的有效电平（true: 高电平, false: 低电平）
 */
void Key_Init(void)
{
  keys[0].port         = KEY_L_GPIO_Port;
  keys[0].pin          = KEY_L_Pin;
  keys[0].active_level = 1;
  keys[0].state        = KEY_IDLE;
  keys[0].last_level   = 0;

  keys[1].port         = KEY_M_GPIO_Port;
  keys[1].pin          = KEY_M_Pin;
  keys[1].active_level = 0;
  keys[1].state        = KEY_IDLE;
  keys[1].last_level   = 1;

  keys[2].port         = KEY_R_GPIO_Port;
  keys[2].pin          = KEY_R_Pin;
  keys[2].active_level = 0;
  keys[2].state        = KEY_IDLE;
  keys[2].last_level   = 1;
}

/**
 * @brief 按键扫描函数
 * @retval result.key_id    按键id
 * @retval result.event     事件类型
 */
KeyEvent_t Key_Scan(void)
{
  KeyEvent_t result = {0, KEY_EVENT_NONE};

  for (uint8_t i = 0; i < KEY_NUM; i++) {
    bool current_level;
    if (HAL_GPIO_ReadPin(keys[i].port, keys[i].pin) == keys[i].active_level) {
      current_level = 1;
    } else
      current_level = 0;

    switch (keys[i].state) {
      case KEY_IDLE:
        if (current_level && !keys[i].last_level) {
          keys[i].state = KEY_PRESSED;
          keys[i].timer = HAL_GetTick();
        }
        break;

      case KEY_PRESSED:
        if (current_level) {
          if (HAL_GetTick() - keys[i].timer >= DEBOUNCE_TIME) {
            keys[i].state = KEY_HOLD;
          }
        } else
          keys[i].state = KEY_IDLE;
        break;

      case KEY_HOLD:
        if (current_level) {
          if (HAL_GetTick() - keys[i].timer >= LONG_PRESS_TIME) {
            result.key_id      = i;
            result.event       = KEY_EVENT_LONG;
            keys[i].state      = KEY_RELEASED;
            keys[i].last_level = current_level;
            return result; // 长按事件
          }
        } else {
          if (HAL_GetTick() - keys[i].timer >= DEBOUNCE_TIME) {
            result.key_id      = i;
            result.event       = KEY_EVENT_SHORT;
            keys[i].state      = KEY_IDLE;
            keys[i].last_level = current_level;
            return result; // 短按事件
          } else
            keys[i].state = KEY_RELEASED;
        }
        break;

      case KEY_RELEASED:
        if (!current_level && (HAL_GetTick() - keys[i].timer >= DEBOUNCE_TIME)) {
          keys[i].state = KEY_IDLE;
        }
        break;

      default: break;
    }
    keys[i].last_level = current_level;
  }
  return result;
}
