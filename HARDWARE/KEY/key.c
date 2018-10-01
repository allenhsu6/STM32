#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "lcd.h"
#include "led.h"
#include "can.h" 
#include "usart.h"
u8  key=4;
int flag=1,flag1=1,flag2=1,flag3=1,flag4=1,para1_flag=1,para2_flag=1,para3_flag=1,para4_flag=1;
float  speed,rpm,soc,mode, state;
//extern u8 bufid;//全局变量budid 用于识别是哪组报文信息
//extern u8 canbuf9[9];//全局变量canbuf9[9] 用于识别是哪组报文信息 哪些信息

 u8 canbuf[8],canbuf_f0[8],canbuf_f1[8],canbuf_f2[8],canbuf_f3[8],canbuf_f4[8],canbuf_f5[8],canbuf_f6[8],canbuf_f7[8],canbuf8[8],bufid;
//按键初始化函数
u8 jjj=0;
u8 msg[8];
extern u32 bufcan0x180[9];
extern u8 buf18f3[8],buf18f5[8];
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//KEY0-KEY2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //默认下拉	GPIO_Mode_IPU
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4,5

	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0

}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(RIGHT_PRES==0||LEFT_PRES==0||DOWN_PRES==0||WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return RIGHT_PRES;
		else if(KEY1==0)return LEFT_PRES;   //开发板需要修改为0，自己画的板子为1
		else if(KEY2==0)return DOWN_PRES;
		else if(WK_UP==1)return WKUP_PRES;
	//	else if(WK_UP1==1)return WKUP1_PRES;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1; 	    
 	return 0;// 无按键按下
}

void Open_interface_init(void)
{
	int i,j;
	POINT_COLOR=BLUE;//
	LCD_Fill(0,0,480,320,BLACK);//480*320分辨率,初始界面背景为黑色
	//sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。				 	
  LCD_ShowString(20-5,70,300,24,24,"The System Is Initialing");
	delay_ms(50);
	for(i=0;i<20;i++)
	{
		//LCD_ShowString(10*i+50,130,210,24,24,"");
		for(j=0;j<30;j++)
		{
			LCD_Fill(50,130+j,50+10*i,130+j,BLUE);
			
		}
		delay_ms(50);
	}
	//delay_ms(500);//防止闪屏太快
	LCD_Fill(0,0,320,240,BLACK);
	LCD_ShowString(60-15,70,300,24,24,"1 Chinese interface");
  LCD_ShowString_nobackwhite(60-15,130,300,24,24,"2 English interface");	
}


void first_generation_car_Open_interface_init(void)
{
	int i,j;
	POINT_COLOR=BLUE;//
	LCD_Fill(0,0,480,320,BLACK);//480*320分辨率,初始界面背景为黑色
	//sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。				 	
  LCD_ShowString(20-5,70,300,24,24,"The System Is Initialing");
	delay_ms(50);
	for(i=0;i<20;i++)
	{
		//LCD_ShowString(10*i+50,130,210,24,24,"");
		for(j=0;j<30;j++)
		{
			LCD_Fill(50,130+j,50+10*i,130+j,BLUE);
			
		}
		delay_ms(50);
	}
	//delay_ms(500);//防止闪屏太快
	LCD_Fill(0,0,320,240,BLACK);
	LCD_ShowString_nobackwhite(60-15-5,70,300,24,24,"First Generation Car");
  LCD_ShowString_nobackwhite(60-15+10,130,300,24,24,"English Interface");	
}




void first_generation_car_static(void)
{
 POINT_COLOR=RED;
	LCD_ShowString_nobackwhite(0,20,80,50,32,"SOC");    LCD_ShowString_nobackwhite(200,20,100,50,32,"%");
	LCD_ShowString_nobackwhite(0,80,100,50,32,"Speed");  LCD_ShowString_nobackwhite(200,80,100,50,32,"km/h");
	LCD_ShowString_nobackwhite(0,140,200,50,32,"Tachometer");  LCD_ShowString_nobackwhite(300,140,100,50,32,"r/min");
	LCD_ShowString_nobackwhite(0,340,200,50,32,"Vehicle Mode");
	LCD_ShowString_nobackwhite(0,400,220,50,32,"Unmanned State");
}

void first_generation_car_interface_init(void)
{
	//from main.c--while(1),always working


	/////can 接收
		//bufid=Can_Receive_Msg(canbuf);//can数据接收 接受id返回值0-8
		if(bufcan0x180[8]==3)
		{
			for(jjj=0;jjj<8;jjj++)
		  {
				canbuf_f3[jjj]=bufcan0x180[jjj];//canbuf9全局变量
				
		  }
		}
		else if(bufcan0x180[8]==5)
		{
			for(jjj=0;jjj<8;jjj++)
		  {
				canbuf_f5[jjj]=bufcan0x180[jjj];//canbuf9全局变量
		  }
		}
		else if(bufcan0x180[8]==6)
		{
			for(jjj=0;jjj<8;jjj++)
		  {
				canbuf_f6[jjj]=bufcan0x180[jjj];//canbuf9全局变量
		  }
		}
		else if(bufcan0x180[8]==7)
		{
			for(jjj=0;jjj<8;jjj++)
		  {
				canbuf_f7[jjj]=bufcan0x180[jjj];//canbuf9全局变量
		  }
		}
		
	 
		speed=msg[1];//0.6的分辨率 十进制输入
     mode=msg[2];
		state=msg[3];
		rpm=buf18f3[6]*25;//25的分辨率 十进制输入
		//battV=(buf18f5[0]+buf18f5[1]*256)*0.1;//电池电压can报文 0.1的分辨率
	//	battI=(buf18f5[0]+buf18f5[1]*256)*0.1-400;//电池电流can报文
		soc=buf18f5[0];
		//alarm=buf18f5[6];
		
		if(KEY0==0)   //KEY0==0
	{	  
		delay_ms(10);//去抖动 
		if(KEY0==0) flag+=1;
	}
	///first interface
	if(flag%4 == 1)//general
	{
		if(flag4==1)
		{
			LCD_Fill(0,0,800,480,BLACK);
			{
   POINT_COLOR=RED;
	LCD_ShowString_nobackwhite(0,20,80,50,32,"SOC");    LCD_ShowString_nobackwhite(200,20,100,50,32,"%");
	LCD_ShowString_nobackwhite(0,80,100,50,32,"Speed");  LCD_ShowString_nobackwhite(200,80,100,50,32,"km/h");
	LCD_ShowString_nobackwhite(0,140,200,50,32,"Tachometer");  LCD_ShowString_nobackwhite(300,140,100,50,32,"r/min");
	LCD_ShowString_nobackwhite(0,340,200,50,32,"Vehicle Mode");
	LCD_ShowString_nobackwhite(0,400,220,50,32,"Unmanned State");
}
			flag4+=1;
		}
		flag3=1;
//////////////////////////static --no update
   if(para1_flag==1) //biaozhiwei ensure only update one time  -reduce runtime
	 {
		para1_flag++;
		para2_flag=1;
	 }
	POINT_COLOR=RED;
	LCD_ShowNum(100,80,(u32)msg,3,32);		
	LCD_ShowNum(200,140,(u32)rpm,5,32);	
	LCD_ShowNum(100,20,(u32)soc,3,32);//soc剩余电量
	 
	if(mode=1)
	{POINT_COLOR=RED;
		LCD_ShowString(250,340,350,50,32,"Manned Mode");	
	}
  else if(mode=0)
{POINT_COLOR=RED;
	LCD_ShowString(250,340,350,50,32,"Unmanned Mode");	
	}
if(state=1)
{ POINT_COLOR=RED; 
	LCD_ShowString(250,400,350,50,32,"8_character loop");
}
else if(state=0)
{
POINT_COLOR=RED;
	LCD_ShowString(250,400,350,50,32,"High speed tracing");
}
	}
}
