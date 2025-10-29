# STM32 Moudle Drivers

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform](https://img.shields.io/badge/Platform-STM32-brightgreen.svg)

一个模块化的STM32传感器和模块驱动库，基于HAL库开发。

## 🚀 特性

- 📚 模块化设计，每个驱动独立
- 📖 详细的文档和示例
- 🔧 基于STM32 HAL库
- 🧪 经过实际测试

## 📦 支持的驱动

### 传感器
- [BH1750](Sensors/BH1750/) - 光照强度传感器
- [DHT11](Sensors/DHT11/) - 温湿度传感器
- [HC-SR04](Sensors/HC-SR04/) - 超声波测距
- [MPU6050](Sensors/MPU6050/) - 运动传感器
- [SHT31](Sensors/SHT31) - 温湿度传感器

### 通信模块  
- [ESP8266](Communication/ESP8266/) - WiFi和MQTT客户端

### 音频模块
- [VS1053](Audio/VS1053/) - 音频解码器

### 输入模块
- [GT911](Input/GT911/) - 电容触摸屏
- [KEY](Input/KEY/) - 多按键扫描

### 显示模块

- [0.96寸OLED](Displays/SSD1306/)
