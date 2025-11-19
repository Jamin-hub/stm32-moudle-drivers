# 状态机按键扫描

##  功能
可以多按键扫描，检测所按下按键`id`和触发事件`event`   

`id`为0、1、2、3、4的按键，检测按键短按事件为`KEY_EVENT_SHORT`，长按事件为`KEY_EVENT_LONG`    

##  使用
1. **修改 `key.h`中定义按键数量、按键长按时间、按键消抖时间**
~~~c
#define KEY_NUM         3     // 按键数量
#define LONG_PRESS_TIME 1000  // 长按时间
#define DEBOUNCE_TIME   20    // 消抖时间
~~~

2. **添加按键接口参数**
首先，修改`key.c`中初始化按键代码

|变量|含义|
|----|:---|
|`keys[x].port`|GPIO端口|
|`keys[x].pin`|GPIO引脚|
|`keys[x].active_level`|按下时的有效电平（true: 高电平, false: 低电平）|
|`keys[x].last_level`|取反`active_level`|

~~~c
/**
 * @brief 初始化按键接口参数
 * @param active_level 有效电平
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
~~~
3. **代码例程**

~~~c
#include "key.h"

KeyEvent_t key;

void main(void)
{
  Key_Init();
  while (1)
  {
    key = Key_Scan();
    if (key.event != KEY_EVENT_NONE)
    {
      switch (key.event)
      {
      case KEY_EVENT_SHORT:
        printf("key %d short press\n", key.key_id);
        break;

      case KEY_EVENT_LONG:
        printf("key %d long press\n", key.key_id);
        break;

      default:
        break;
      }
    }
  }
}
~~~
