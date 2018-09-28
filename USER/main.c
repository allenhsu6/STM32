#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "can.h"

typedef struct COM *Massage;

struct COM{

	unsigned char head1; // 0xAA   									1 byte
	unsigned char head2; // 0x55
	float GPS_velocity; // gps返回的车辆当前真实速度，用以显示			4 byte
	float AIM_velocity; // 通过计算得出的速度目标值					4 byte
	short int CarModel; // 0：无人状态 1：有人状态 显示				2 byte
	short int BodyModel; // 0为直线赛 1为8字绕环 2为高速循迹 显示		2 byte
	unsigned int Serial; 		// 消息列							4 byte
	short int SystemState; // 上位机状态， 用于鸣笛					2 byte
	short int BrakeSingal; // 刹车灯信号， 用于显示					2 byte
	unsigned char end1;  // 回车										不用管
	unsigned char end2;  // 换行

};



 int main(void)
 {
 	// 必须在前面定义
		u8 key;
	u8 i=0,t=0;
	u8 cnt=0;
	u8 canbuf[8];  // u8是按照字节读入的  canbuf是个数组
	u8 res;
	u8 mode=CAN_Mode_LoopBack;//CAN工作模式;CAN_Mode_Normal(0)：普通模式，CAN_Mode_LoopBack(1)：环回模式
	Massage msg; // COM口传入的结构体指针


	delay_init();	    	//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	LED_Init();		  		//初始化与LED连接的硬件接口
	LCD_Init();			   	//初始化LCD	
	KEY_Init();				//按键初始化		 	


	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);//CAN初始化环回模式,波特率500Kbps    

 	POINT_COLOR=RED;//设置字体为红色
	LCD_ShowString(60,130,200,16,16,"LoopBack Mode");
	LCD_ShowString(60,150,200,16,16,"KEY0:Send WK_UP:Mode");//显示提示信息		
	POINT_COLOR=BLUE;//设置字体为蓝色	  
	LCD_ShowString(60,170,200,16,16,"Count:");			//显示当前计数值	
	LCD_ShowString(60,190,200,16,16,"Send Data:");		//提示发送的数据	
	LCD_ShowString(60,250,200,16,16,"Receive Data:");	//提示接收到的数据		


	while(1)
	{
		key=KEY_Scan(0);

		if((USART_RX_STA&0x8000) && key==KEY0_PRES)
		{
			len=USART_RX_STA&0x3fff;   //得到此次接收到的数据长度
			printf("\r\n您发送的消息为:%d\r\n\r\n",len);  //  查看所谓数据长度是指字节数吗？

			// 总共22个 存放在msg结构体中
			if(len == 22){
				msg->head1 = USART_RX_BUF[0];
				msg->head2 = USART_RX_BUF[1];
				msg->GPS_velocity = (float)USART_RX_BUF[2];
				msg->AIM_velocity = (float)USART_RX_BUF[6];
				msg->CarModel = (short int)USART_RX_BUF[10];
				msg->BodyModel = (short int)USART_RX_BUF[12];
				msg->Serial = (unsigned int)USART_RX_BUF[14];
				msg->SystemState = (short int)USART_RX_BUF[18];
				msg->BrakeSingal = (short int)USART_RX_BUF[20];
			}

			for(i=0;i<8;i++)
			{
				if(i<4)LCD_ShowxNum(60+i*32,210,USART_RX_BUF[i],3,16,0X80);	//显示数据
				else LCD_ShowxNum(60+(i-4)*32,230,USART_RX_BUF[i],3,16,0X80);	//显示数据
			}

			printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;

			/**
			 * 将usartbuf中的数据发送给can端
			 */
			res = Can_Send_Msg(USART_RX_BUF,8);


			if(res)LCD_ShowString(60+80,190,200,16,16,"Failed");		//提示发送失败
			else LCD_ShowString(60+80,190,200,16,16,"OK    ");
		}


		/**
		 * 这块上键控制模式变换
		 */
		else if(key==WKUP_PRES)//WK_UP按下，改变CAN的工作模式
		{
			mode=!mode;
  			CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,mode);//CAN普通模式初始化, 波特率500Kbps
			POINT_COLOR=RED;//设置字体为红色 
			if(mode==0)//普通模式，需要2个开发板
			{
				LCD_ShowString(60,130,200,16,16,"Nnormal Mode ");
			}else //回环模式,一个开发板就可以测试了.
			{
 				LCD_ShowString(60,130,200,16,16,"LoopBack Mode");
			}
 			POINT_COLOR=BLUE;//设置字体为蓝色
		}



		key=Can_Receive_Msg(canbuf);  // 这个操作使得数据存放在canbuf中，并返回是否接受成功标志key

		/**
		 * 像串口发送canbuf中的数据
		 */



        if(key)//接收到有数据
		{
			LCD_Fill(60,270,130,310,WHITE);//清除之前的显示
 			for(i=0;i<key;i++)
			{
				if(i<4)LCD_ShowxNum(60+i*32,270,canbuf[i],3,16,0X80);	//显示数据
				else LCD_ShowxNum(60+(i-4)*32,290,canbuf[i],3,16,0X80);	//显示数据
				USART_SendData(USART1, canbuf[i]);//向串口1发送数据
			}
		}


		t++;
		delay_ms(10);
		if(t==20)
		{
			LED0=!LED0;//提示系统正在运行	
			t=0;
			cnt++;
			LCD_ShowxNum(60+48,170,cnt,3,16,0X80);	//显示数据
		}
	}
}



