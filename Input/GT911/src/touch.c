#include "touch.h"
#include "lcd.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"

_m_tp_dev tp_dev = {
    TP_Init,
    GT911_Scan,
    0,
    0,
    0,
    0,
};

/**
 * @brief 绘制一个大的点（2x2像素矩形）
 * @param x 点的x坐标
 * @param y 点的y坐标
 * @param color 点的颜色
 * @return 无返回值
 */
void TP_Draw_Big_Point(uint16_t x, uint16_t y, uint16_t color)
{
  LCD_DrawPoint(x, y, color); // 中心点
  LCD_DrawPoint(x + 1, y, color);
  LCD_DrawPoint(x, y + 1, color);
  LCD_DrawPoint(x + 1, y + 1, color);
}

/**
 * @brief 触摸屏初始化函数
 * @param 无
 * @return uint8_t 初始化状态
 *         - 0: 初始化成功
 * @note 该函数用于初始化GT911电容触摸屏，通过循环等待确保触摸屏初始化完成，
 *       并配置触摸屏设备的扫描函数和触摸类型。
 */
uint8_t TP_Init(void)
{
  // 等待GT911触摸屏初始化成功，如果失败则延时500ms后重试
  while (!GT911_Init()) {
    HAL_Delay(500);
  }

  tp_dev.scan = GT911_Scan; // 扫描函数指向GT911触摸屏扫描
  tp_dev.touchtype |= 0X80; // 电容屏
  return 0;
}

const uint16_t POINT_COLOR_TBL[CT_MAX_TOUCH] = {RED, GREEN, BLUE, BROWN, GRED};

void Load_Drow_Dialog(void)
{
  LCD_ShowString(lcddev.width - 24, 0, "RST", RED, WHITE, 16, 0); // 显示清屏区域
}

/**
 * 在LCD屏幕上绘制一条直线
 * @param x1 起始点X坐标
 * @param y1 起始点Y坐标
 * @param x2 终止点X坐标
 * @param y2 终止点Y坐标
 * @param color 直线颜色
 */
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
  // 初始化变量
  uint16_t t;
  int      xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int      incx, incy, uRow, uCol;

  // 计算坐标增量
  delta_x = x2 - x1;
  delta_y = y2 - y1;
  uRow    = x1;
  uCol    = y1;

  // 设置X轴方向增量
  if (delta_x > 0)
    incx = 1;
  else if (delta_x == 0)
    incx = 0; // 垂直线
  else {
    incx    = -1;
    delta_x = -delta_x;
  }

  // 设置Y轴方向增量
  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0; // 水平线
  else {
    incy    = -1;
    delta_y = -delta_y;
  }

  // 选取基本增量坐标轴
  if (delta_x > delta_y)
    distance = delta_x;
  else
    distance = delta_y;

  // Bresenham算法绘制直线
  for (t = 0; t <= distance + 1; t++) {
    TP_Draw_Big_Point(uRow, uCol, color); // 画点
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}

/**
 * @brief 触摸屏测试函数
 * @details 该函数用于测试触摸屏功能，支持多点触控绘图
 *          初始化触摸屏和LCD，循环检测触摸事件并在屏幕上绘制轨迹
 * @param void 无参数
 * @return void 无返回值
 */
void ctp_test(void)
{
  // 触摸点索引和循环计数器
  uint8_t t = 0;
  uint8_t i = 0;

  // 存储每个触摸点最后位置的数组，[点号][0:横坐标 1:纵坐标]
  uint16_t lastpos[5][2];

  // 初始化触摸屏设备
  tp_dev.init();

  // 清空LCD屏幕为白色背景
  LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

  // 主循环：持续检测触摸并绘制
  while (1) {
    // 在屏幕右上角显示"RST"文本（重置按钮区域）
    LCD_ShowString(lcddev.width - 24, 0, (uint8_t *)"RST", RED, WHITE, 16, 0);

    // 扫描触摸屏状态
    tp_dev.scan(0);

    // 遍历所有可能的触摸点（最多5个）
    for (t = 0; t < CT_MAX_TOUCH; t++) {
      // 检查当前触摸点是否有触摸事件
      if ((tp_dev.sta) & (1 << t)) {
        // 短暂延时去抖动
        HAL_Delay(1);

        // 判断触摸坐标是否在LCD显示范围内
        if (tp_dev.x[t] < lcddev.width && tp_dev.y[t] < lcddev.height) {
          // 如果是该点的首次触摸，记录当前位置
          if (lastpos[t][0] == 0XFFFF) {
            lastpos[t][0] = tp_dev.x[t];
            lastpos[t][1] = tp_dev.y[t];
          }

          // 在上次位置和当前位置之间绘制线条，使用对应颜色
          lcd_draw_bline(lastpos[t][0], lastpos[t][1], tp_dev.x[t], tp_dev.y[t], POINT_COLOR_TBL[t]);

          // 更新该触摸点的最后位置记录
          lastpos[t][0] = tp_dev.x[t];
          lastpos[t][1] = tp_dev.y[t];

          // 检测是否触摸到屏幕右上角的"RST"区域（但未实现具体功能）
          if (tp_dev.x[t] > (lcddev.width - 24) && tp_dev.y[t] < 16) {
          }
        }

      } else {
        // 当前点无触摸事件，标记为无效位置
        lastpos[t][0] = 0XFFFF;
      }
    }

    // 循环延时和计数器递增
    HAL_Delay(5);
    i++;
  }
}