/*
 * a.c
 *
 *  Created on: 2021��2��7��
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
//�궨��
#define SCUGIC_ID XPAR_SCUGIC_0_DEVICE_ID //�жϿ�����ID
#define GPIOPS_ID XPAR_XGPIOPS_0_DEVICE_ID //PS ��GPIO ����ID
#define AXI_GPIO_ID XPAR_AXI_GPIO_0_DEVICE_ID //PL ��AXI GPIO ����ID
#define GPIO_INT_ID XPAR_FABRIC_GPIO_0_VEC_ID //PL ��AXI GPIO �ж�ID

#define MIO_LED 23 //PS LED ���ӵ�MIO0
#define KEY_CHANNEL 1 //PL ����ʹ��AXI GPIO ͨ��1
#define KEY_MASK XGPIO_IR_CH1_MASK //ͨ��1 ��λ����
//��������
void instance_init(); //��ʼ����������

void axi_gpio_handler(void *CallbackRef); //�жϷ�����
//ȫ�ֱ���
XScuGic scugic_inst; //�жϿ���������ʵ��
XScuGic_Config * scugic_cfg_ptr; //�жϿ�����������Ϣ
XGpioPs gpiops_inst; //PS ��GPIO ����ʵ��
XGpioPs_Config * gpiops_cfg_ptr; //PS ��GPIO ������Ϣ
XGpio axi_gpio_inst; //PL ��AXI GPIO ����ʵ��
int led_value = 1; //LED ��ʾ״̬

int main()
{
printf("AXI GPIO INTERRUPT TEST!\n");
//��ʼ������������
instance_init();
//����PS GPIO
XGpioPs_SetDirectionPin(&gpiops_inst, MIO_LED, 1); //����PS GPIO Ϊ���
XGpioPs_SetOutputEnablePin(&gpiops_inst, MIO_LED, 1); //ʹ��LED ���
XGpioPs_WritePin(&gpiops_inst, MIO_LED, led_value); //����LED
//����PL AXI GPIO
XGpio_SetDataDirection(&axi_gpio_inst, KEY_CHANNEL, 1); //����AXI GPIO ͨ��1 Ϊ����
XGpio_InterruptEnable(&axi_gpio_inst, KEY_MASK); //ʹ��ͨ��1 �ж�
XGpio_InterruptGlobalEnable(&axi_gpio_inst); //ʹ��AXI GPIO ȫ���ж�
//�����ж����ȼ��ʹ�������(�ߵ�ƽ����)
XScuGic_SetPriorityTriggerType(&scugic_inst, GPIO_INT_ID, 0xA0, 0x1);
//�����ж�ID ���жϴ�����
XScuGic_Connect(&scugic_inst, GPIO_INT_ID, axi_gpio_handler, &axi_gpio_inst);
//ʹ��AXI GPIO �ж�
XScuGic_Enable(&scugic_inst, GPIO_INT_ID);
//���ò����ж��쳣������
Xil_ExceptionInit();
Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
(Xil_ExceptionHandler)XScuGic_InterruptHandler, &scugic_inst);
Xil_ExceptionEnable();
while(1);
return 0;
}


//��ʼ������������
void instance_init()
{
//��ʼ���жϿ���������
scugic_cfg_ptr = XScuGic_LookupConfig(SCUGIC_ID);
XScuGic_CfgInitialize(&scugic_inst, scugic_cfg_ptr, scugic_cfg_ptr->CpuBaseAddress);
//��ʼ��PS ��GPIO ����
gpiops_cfg_ptr = XGpioPs_LookupConfig(GPIOPS_ID);
XGpioPs_CfgInitialize(&gpiops_inst, gpiops_cfg_ptr, gpiops_cfg_ptr->BaseAddr);
//��ʼ��PL ��AXI GPIO ����
XGpio_Initialize(&axi_gpio_inst, AXI_GPIO_ID);
}
//PL ��AXI GPIO �жϷ���(����)����
void axi_gpio_handler(void *CallbackRef)
{
int key_value = 1;
XGpio *GpioPtr = (XGpio *)CallbackRef;
print("Interrupt Detected!\n");
XGpio_InterruptDisable(GpioPtr, KEY_MASK); //�ر�AXI GPIO �ж�ʹ��
key_value = XGpio_DiscreteRead(GpioPtr, KEY_CHANNEL); //��ȡ��������
if(key_value == 0){ //�жϰ�������
led_value = ~led_value;
XGpioPs_WritePin(&gpiops_inst, MIO_LED, led_value); //�ı�LED ��ʾ״̬
}
sleep(1); //��ʱ1s,��������
XGpio_InterruptClear(GpioPtr, KEY_MASK); //����ж�
XGpio_InterruptEnable(GpioPtr, KEY_MASK); //ʹ��AXI GPIO �ж�
}

