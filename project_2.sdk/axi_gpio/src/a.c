/*
 * a.c
 *
 *  Created on: 2021年2月7日
 *      Author: 72721
 */


#include "xparameters.h"
#include "xgpiops.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "sleep.h"
#include "stdio.h"
//宏定义
#define SCUGIC_ID XPAR_SCUGIC_0_DEVICE_ID //中断控制器ID
#define GPIOPS_ID XPAR_XGPIOPS_0_DEVICE_ID //PS 端GPIO 器件ID
#define AXI_GPIO_ID XPAR_AXI_GPIO_0_DEVICE_ID //PL 端AXI GPIO 器件ID
#define GPIO_INT_ID XPAR_FABRIC_GPIO_0_VEC_ID //PL 端AXI GPIO 中断ID

#define MIO_LED 23 //PS LED 连接到MIO0
#define KEY_CHANNEL 1 //PL 按键使用AXI GPIO 通道1
#define KEY_MASK XGPIO_IR_CH1_MASK //通道1 的位定义
//函数声明
void instance_init(); //初始化器件驱动

void axi_gpio_handler(void *CallbackRef); //中断服务函数
//全局变量
XScuGic scugic_inst; //中断控制器驱动实例
XScuGic_Config * scugic_cfg_ptr; //中断控制器配置信息
XGpioPs gpiops_inst; //PS 端GPIO 驱动实例
XGpioPs_Config * gpiops_cfg_ptr; //PS 端GPIO 配置信息
XGpio axi_gpio_inst; //PL 端AXI GPIO 驱动实例
int led_value = 1; //LED 显示状态

int main()
{
printf("AXI GPIO INTERRUPT TEST!\n");
//初始化各器件驱动
instance_init();
//配置PS GPIO
XGpioPs_SetDirectionPin(&gpiops_inst, MIO_LED, 1); //设置PS GPIO 为输出
XGpioPs_SetOutputEnablePin(&gpiops_inst, MIO_LED, 1); //使能LED 输出
XGpioPs_WritePin(&gpiops_inst, MIO_LED, led_value); //点亮LED
//配置PL AXI GPIO
XGpio_SetDataDirection(&axi_gpio_inst, KEY_CHANNEL, 1); //设置AXI GPIO 通道1 为输入
XGpio_InterruptEnable(&axi_gpio_inst, KEY_MASK); //使能通道1 中断
XGpio_InterruptGlobalEnable(&axi_gpio_inst); //使能AXI GPIO 全局中断
//设置中断优先级和触发类型(高电平触发)
XScuGic_SetPriorityTriggerType(&scugic_inst, GPIO_INT_ID, 0xA0, 0x1);
//关联中断ID 和中断处理函数
XScuGic_Connect(&scugic_inst, GPIO_INT_ID, axi_gpio_handler, &axi_gpio_inst);
//使能AXI GPIO 中断
XScuGic_Enable(&scugic_inst, GPIO_INT_ID);
//设置并打开中断异常处理功能
Xil_ExceptionInit();
Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
(Xil_ExceptionHandler)XScuGic_InterruptHandler, &scugic_inst);
Xil_ExceptionEnable();
while(1);
return 0;
}


//初始化各器件驱动
void instance_init()
{
//初始化中断控制器驱动
scugic_cfg_ptr = XScuGic_LookupConfig(SCUGIC_ID);
XScuGic_CfgInitialize(&scugic_inst, scugic_cfg_ptr, scugic_cfg_ptr->CpuBaseAddress);
//初始化PS 端GPIO 驱动
gpiops_cfg_ptr = XGpioPs_LookupConfig(GPIOPS_ID);
XGpioPs_CfgInitialize(&gpiops_inst, gpiops_cfg_ptr, gpiops_cfg_ptr->BaseAddr);
//初始化PL 端AXI GPIO 驱动
XGpio_Initialize(&axi_gpio_inst, AXI_GPIO_ID);
}
//PL 端AXI GPIO 中断服务(处理)函数
void axi_gpio_handler(void *CallbackRef)
{
int key_value = 1;
XGpio *GpioPtr = (XGpio *)CallbackRef;
print("Interrupt Detected!\n");
XGpio_InterruptDisable(GpioPtr, KEY_MASK); //关闭AXI GPIO 中断使能
key_value = XGpio_DiscreteRead(GpioPtr, KEY_CHANNEL); //读取按键数据
if(key_value == 0){ //判断按键按下
led_value = ~led_value;
XGpioPs_WritePin(&gpiops_inst, MIO_LED, led_value); //改变LED 显示状态
}
sleep(1); //延时1s,按键消抖
XGpio_InterruptClear(GpioPtr, KEY_MASK); //清除中断
XGpio_InterruptEnable(GpioPtr, KEY_MASK); //使能AXI GPIO 中断
}

