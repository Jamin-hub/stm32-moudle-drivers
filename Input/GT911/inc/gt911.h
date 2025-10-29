#ifndef __GT911_H_
#define __GT911_H_

#include "touch.h"
#include "i2c.h"

#define TOUCH_MAX         5

#define GT911_I2C_TIMEOUT 10
#define GT911_I2C         hi2c2

#define GT911_IIC_ADDR    0x5D // IIC read address, should be 0x29
// #define GT911_IIC_WADDR 0xBA	//IIC write address, should be 0x28

#define GT911_READ_ADDR   0x814E // touch point information
#define GT911_ID_ADDR     0x8140 // ID of touch IC

uint8_t GT911_WriteReg(uint16_t RegAddr, uint8_t *pRegBuf, uint8_t ucLen);

uint8_t GT911_ReadReg(uint16_t RegAddr, uint8_t *pRegBuf, uint8_t ucLen);

void    GT911_reset(void);
uint8_t GT911_Init(void);
uint8_t Touch_Get_Count(void);
uint8_t GT911_Scan(uint8_t mode);

#endif
