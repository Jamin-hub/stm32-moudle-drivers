#include "gt911.h"
#include "gpio.h"
#include "lcd.h"

#include <string.h>
#include <stdio.h>

/**
 * @brief 向GT911寄存器写入数据
 * 通过I2C通信，向GT911的指定寄存器写入指定长度的数据。
 * @param _usRegAddr 寄存器地址
 * @param _pRegBuf 数据缓冲区指针
 * @param _ucLen 数据长度
 */
uint8_t GT911_WriteReg(uint16_t RegAddr, uint8_t *pRegBuf, uint8_t ucLen)
{
  uint8_t data[2 + ucLen];
  data[0] = RegAddr >> 8;
  data[1] = RegAddr & 0xFF;

  memcpy(&data[2], pRegBuf, ucLen);

  if (HAL_I2C_Master_Transmit(&GT911_I2C, GT911_IIC_ADDR << 1, data, 2 + ucLen, 100) != HAL_OK)
    return 0;

  return 1;
}

/**
 * @brief 读取GT911寄存器
 * 通过I2C接口读取GT911指定寄存器的值，并将读取到的数据保存到缓冲区中。
 * @param _usRegAddr 寄存器地址
 * @param _pRegBuf 存储读取数据的缓冲区指针
 * @param _ucLen 读取数据的长度
 */
uint8_t GT911_ReadReg(uint16_t RegAddr, uint8_t *pRegBuf, uint8_t ucLen)
{
  uint8_t regAddr[2] = {RegAddr >> 8, RegAddr & 0xFF};

  // 先发送寄存器地址
  if (HAL_I2C_Master_Transmit(&GT911_I2C, GT911_IIC_ADDR << 1, regAddr, 2, HAL_MAX_DELAY) != HAL_OK) {
    return HAL_ERROR;
  }
  // 然后读取数据
  if (HAL_I2C_Master_Receive(&GT911_I2C, GT911_IIC_ADDR << 1, pRegBuf, ucLen, HAL_MAX_DELAY) != HAL_OK) {
    return HAL_ERROR;
  }
  return HAL_OK;
}

/**
 * @brief GT911_reset
 */
void GT911_reset(void)
{
  HAL_GPIO_WritePin(T_RST_GPIO_Port, T_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(T_RST_GPIO_Port, T_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
}

/**
 * @brief GT911 init, including GPIO init, sync, and version check
 * @retval 1: success
 */
uint8_t GT911_Init(void)
{
  uint8_t touchIC_ID[4];
  GT911_reset();
  GT911_ReadReg(GT911_ID_ADDR, touchIC_ID, 4);
  //	touchIC_ID[0] = Touch_Get_Count();
  if (touchIC_ID[0] == '9') {
    printf("Touch ID: %s \r\n", touchIC_ID);
    return 1;
  } else {
    printf("Touch Error\r\n");
    return 0;
  }
}

/**
 * @brief 获取触摸点数量
 * @param 无
 * @return uint8_t 触摸点数量(0-15)
 */
uint8_t Touch_Get_Count(void)
{
  // 读取触摸芯片寄存器，获取当前触摸点数量
  uint8_t count[1] = {0};
  GT911_ReadReg(GT911_READ_ADDR, count, 1); // read touch data
  // 返回触摸点数量(低4位有效)
  return (count[0] & 0x0f);
}

/**
 * @brief 获取触摸点坐标
 * @param 无
 * @return uint8_t 触摸点数量(0-15)
 */
const uint16_t TPX[] = {0x8150, 0x8158, 0x8160, 0x8168, 0x8170}; // 电容屏触摸点数据地址（1~5）

/**
 * @brief   扫描GT911触摸屏控制器，获取触摸状态和坐标数据
 * @param   mode: 触摸屏工作模式（输入参数，用于读取寄存器）
 * @return  uint8_t:
 *          - 1 表示成功读取到有效的触摸数据
 *          - 0 表示未检测到有效触摸或数据无效
 *
 * @note    此函数通过轮询方式与GT911通信，定期读取触摸点信息，并根据屏幕方向调整坐标。
 *          使用静态变量t控制扫描频率以降低CPU占用率。支持最多5个触摸点的识别。
 */
uint8_t GT911_Scan(uint8_t mode)
{
  uint8_t        buf[4];
  uint8_t        i   = 0;
  uint8_t        res = 0;
  uint8_t        temp;
  uint8_t        tempsta;
  static uint8_t t = 0; // 控制查询间隔,从而降低CPU占用率

  t++;

  // 控制扫描频率：每10次调用才进行一次实际检测，减少CPU资源消耗
  if ((t % 10) == 0 || t < 10) {
    GT911_ReadReg(GT911_READ_ADDR, &mode, 1);

    // 检查是否有触摸事件发生且触摸点数量在合理范围内
    if (mode & 0X80 && ((mode & 0XF) < 6)) {
      temp = 0;
      GT911_WriteReg(GT911_READ_ADDR, &temp, 1); // 清除中断标志位
    }

    // 判断是否有点被按下并且点数小于6
    if ((mode & 0XF) && ((mode & 0XF) < 6)) {
      temp    = 0XFF << (mode & 0XF); // 将点的个数转换为1的位数,匹配tp_dev.sta定义
      tempsta = tp_dev.sta;           // 保存当前的tp_dev.sta值

      // 更新触摸状态：设置哪些点是有效的，并标记为按下状态
      tp_dev.sta = (~temp) | TP_PRES_DOWN | TP_CATH_PRES;

      tp_dev.x[4] = tp_dev.x[0]; // 保存触点0的数据作为备份
      tp_dev.y[4] = tp_dev.y[0];

      // 遍历所有可能的触摸点并读取其坐标
      for (i = 0; i < 5; i++) {
        if (tp_dev.sta & (1 << i)) // 触摸有效?
        {
          GT911_ReadReg(TPX[i], buf, 4); // 读取XY坐标值

          // 根据屏幕方向配置对原始坐标做相应变换
          if (DFT_SCAN_DIR == U2D_L2R) // 横屏
          {
            tp_dev.y[i] = 320 - ((uint16_t)buf[1] << 8) - buf[0];
            tp_dev.x[i] = (((uint16_t)buf[3] << 8) + buf[2]);
          } else if (DFT_SCAN_DIR == R2L_U2D) {
            tp_dev.x[i] = ((uint16_t)buf[1] << 8) + buf[0];
            tp_dev.y[i] = ((uint16_t)buf[3] << 8) + buf[2];
          } else if (DFT_SCAN_DIR == L2R_D2U) {
            tp_dev.x[i] = 320 - (((uint16_t)buf[1] << 8) + buf[0]);
            tp_dev.y[i] = 480 - (((uint16_t)buf[3] << 8) + buf[2]);
          } else {
            tp_dev.y[i] = ((uint16_t)buf[1] << 8) + buf[0];
            tp_dev.x[i] = 480 - (((uint16_t)buf[3] << 8) + buf[2]);
          }
        }
      }

      res = 1;

      // 数据合法性检查：如果第一个点超出LCD范围
      if (tp_dev.x[0] > lcddev.width || tp_dev.y[0] > lcddev.height) {
        if ((mode & 0XF) > 1) // 其他点存在有效数据，则复制第二点到第一点
        {
          tp_dev.x[0] = tp_dev.x[1];
          tp_dev.y[0] = tp_dev.y[1];
          t           = 0; // 触发后重置计数器，保证后续持续采样
        } else             // 否则丢弃本次数据，恢复旧数据
        {
          tp_dev.x[0] = tp_dev.x[4];
          tp_dev.y[0] = tp_dev.y[4];
          mode        = 0X80;
          tp_dev.sta  = tempsta; // 恢复之前的触摸状态
        }
      } else
        t = 0; // 若数据合法，重置计数器以便继续监控
    }
  }

  // 处理无触摸情况下的状态更新
  if ((mode & 0X8F) == 0X80) // 无触摸点按下
  {
    if (tp_dev.sta & TP_PRES_DOWN) // 之前是被按下的
    {
      tp_dev.sta &= ~(1 << 7); // 标记按键松开
    } else                     // 之前就没有被按下
    {
      tp_dev.x[0] = 0xffff;
      tp_dev.y[0] = 0xffff;
      tp_dev.sta &= 0XE0; // 清除点有效标记
    }
  }

  // 计数器溢出保护，避免长时间运行导致溢出问题
  if (t > 240)
    t = 10;

  return res;
}
