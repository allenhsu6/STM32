#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
  	 


#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_5)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)//读取按键2 
#define WK_UP   GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//读取按键3(WK_UP) 

 
#define WK_UP1   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 

#define RIGHT_PRES 	1	//RIGHT_PRES按下
#define LEFT_PRES	2	//LEFT_PRES按下
#define DOWN_PRES	3	//DOWN_PRES按下
#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)

#define WKUP1_PRES   5	//KEY_UP按下(即WK_UP/KEY_UP)
void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数	
void KEY_choose(void);
void First_level_interface_init(void);
void Open_interface_init(void);
void Second_level_interface_init(void);
void first_generation_car_Open_interface_init(void);
void first_generation_car_interface_init(void);
void first_generation_car_static(void);
void display_General_information(void);
void display_Motor_information(void);
void display_Battery_information(void);
void display_Position_information(void);
void display_Parameter_information(void);
void display_Set_information(void);
void display_Exit(void);

#endif
