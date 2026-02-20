# gpio_init 使用说明

## 支持范围
- 端口：GPIO_PORT_A、GPIO_PORT_B、GPIO_PORT_C、GPIO_PORT_D、GPIO_PORT_E
- 引脚：0~15

## 函数原型
```c
int gpio_init(uint8_t port, uint8_t pin, gpio_mode_t mode, gpio_pull_t pull, gpio_speed_t speed);
```

## 错误码
- 0：成功
- -5（GPIO_EIO）：寄存器读回异常
- -19（GPIO_ENODEV）：端口非法
- -22（GPIO_EINVAL）：参数非法
- -34（GPIO_ERANGE）：PIN 越界

## 常用宏速查
| 宏 | 说明 |
| --- | --- |
| GPIO_OUT_PP(port, pin) | 推挽输出 | 
| GPIO_OUT_OD(port, pin) | 开漏输出 | 
| GPIO_IN_FLOAT(port, pin) | 浮空输入 | 
| GPIO_IN_PU(port, pin) | 上拉输入 | 
| GPIO_IN_PD(port, pin) | 下拉输入 |

## 使用示例
```c
GPIO_OUT_PP(GPIO_PORT_A, 0);
GPIO_IN_PU(GPIO_PORT_B, 7);
GPIO_OUT_OD(GPIO_PORT_C, 15);
```
