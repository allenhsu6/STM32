#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "can.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct COM{

	unsigned char head1; // 0x30   ��Ӧͼ��0	    					1 byte
	unsigned char head2; // 0x31   ��Ӧͼ��1
	float GPS_velocity; // gps���صĳ�����ǰ��ʵ�ٶȣ�������ʾ			4 byte
	float AIM_velocity; // ͨ������ó����ٶ�Ŀ��ֵ					4 byte
	short int CarModel; // 0������״̬ 1������״̬ ��ʾ				2 byte
	short int BodyModel; // 0Ϊֱ���� 1Ϊ8���ƻ� 2Ϊ����ѭ�� ��ʾ		2 byte
	int Serial; 		// ��Ϣ��							4 byte
	short int SystemState; // ��λ��״̬�� ��������					2 byte
	short int BrakeSingal; // ɲ�����źţ� ������ʾ					2 byte
	unsigned char end1;  // �س�										���ù�
	unsigned char end2;  // ����

};

struct ToCom{
	u8 head_1;
	u8 head_2;
	u8 end_1;
	u8 end2;
    short int soc;          // ���ص����ź�
    float VCU_velocity;     // ���س���
    short int channel_1;       // ң������ȫ����Ϣ
	  short int channel_2;       // ң������ȫ����Ϣ
	  short int channel_3;       // ң������ȫ����Ϣ
	  short int channel_4;       // ң������ȫ����Ϣ
	  short int channel_5;       // ң������ȫ����Ϣ
	  short int channel_6;       // ң������ȫ����Ϣ
	  short int channel_7;       // ң������ȫ����Ϣ
	  short int channel_8;       // ң������ȫ����Ϣ
    short int abnormal;     // �����쳣�����ϴ�1
};


void buffproc(u8 *tembuff);
void makebuff( char *procbuff,const struct ToCom temp);

struct COM msg;         // ��com��
struct ToCom msgToCom;  // ����com

 int main(void)
 {
 	// ������ǰ�涨��
 	u8 key;
	u8 i=0,t=0,len=0;
	u8 cnt=0;
	u8 canbuf[8];  // u8�ǰ����ֽڶ����  canbuf�Ǹ�����
	u8 res;
	u8 mode=CAN_Mode_LoopBack;//CAN����ģʽ;CAN_Mode_Normal(0)����ͨģʽ��CAN_Mode_LoopBack(1)������ģʽ

	
	u8 temp[4] = {""};   // ������ʱ��ſ�������

	delay_init();	    	//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();			   	//��ʼ��LCD	
	KEY_Init();				//������ʼ��		 	

	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);//CAN��ʼ������ģʽ,������500Kbps    

 	POINT_COLOR=RED;//��������Ϊ��ɫ
	LCD_ShowString(60,130,200,16,16,"LoopBack Mode");
	LCD_ShowString(60,150,200,16,16,"KEY0:Send WK_UP:Mode");//��ʾ��ʾ��Ϣ		
	POINT_COLOR=BLUE;//��������Ϊ��ɫ	  
	LCD_ShowString(60,170,200,16,16,"Count:");			//��ʾ��ǰ����ֵ	
	LCD_ShowString(60,190,200,16,16,"Send Data:");		//��ʾ���͵�����	
	LCD_ShowString(60,250,200,16,16,"Receive Data:");	//��ʾ���յ�������		


	while(1)
	{
		key=KEY_Scan(0);

		if((USART_RX_STA&0x8000))
		{
			len=USART_RX_STA&0x3fff;   //�õ��˴ν��յ������ݳ���
			printf("\r\n�����͵���Ϣ����Ϊ: %d λ\r\n",len);  //  ���ݳ���
			
			buffproc(USART_RX_BUF);

			LCD_ShowxNum(60,210,msg.GPS_velocity,3,16,0X80);	//��ʾ����
		 	// LCD_ShowxNum(60,230,msg.head1,3,16,0X80);	//��ʾ����

            printf("\r\n��msg�е�GPS_velocityΪ�� %f\r\n",msg.GPS_velocity);
			
			printf("%f\r\n",msg.AIM_velocity);
            printf("%d\r\n",    msg.CarModel);
            printf("%d\r\n",   msg.BodyModel);
            printf("%d\r\n",      msg.Serial);
            printf("%d\r\n", msg.SystemState);
            printf("%d\r\n", msg.BrakeSingal);

            USART_RX_STA=0;

			/**
			 * ��usartbuf�е����ݷ��͸�can��
			 */
			res = Can_Send_Msg(USART_RX_BUF,8);


			if(res)LCD_ShowString(60+80,190,200,16,16,"Failed");		//��ʾ����ʧ��
			else LCD_ShowString(60+80,190,200,16,16,"OK    ");
		}


		/**
		 * ����ϼ�����ģʽ�任
		 */
		else if(key==WKUP_PRES)//WK_UP���£��ı�CAN�Ĺ���ģʽ
		{
			mode=!mode;
  			CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,mode);//CAN��ͨģʽ��ʼ��, ������500Kbps
			POINT_COLOR=RED;//��������Ϊ��ɫ 
			if(mode==0)//��ͨģʽ����Ҫ2��������
			{
				LCD_ShowString(60,130,200,16,16,"Nnormal Mode ");
			}else //�ػ�ģʽ,һ��������Ϳ��Բ�����.
			{
 				LCD_ShowString(60,130,200,16,16,"LoopBack Mode");
			}
 			POINT_COLOR=BLUE;//��������Ϊ��ɫ
		}



		key=Can_Receive_Msg(canbuf);  // �������ʹ�����ݴ����canbuf�У��������Ƿ���ܳɹ���־key

		/**
		 * �򴮿ڷ���canbuf�е�����
		 */

        if(key)//���յ�������
		{
			LCD_Fill(60,270,130,310,WHITE);//���֮ǰ����ʾ
 			for(i=0;i<key;i++)
			{
				USART_SendData(USART1, canbuf[i]);//�򴮿�1��������

				if(i<4)LCD_ShowxNum(60+i*32,270,canbuf[i],3,16,0X80);	//��ʾ����
				else LCD_ShowxNum(60+(i-4)*32,290,canbuf[i],3,16,0X80);	//��ʾ����
			}
		}


		t++;
		delay_ms(10);
		if(t==20)
		{
			LED0=!LED0;//��ʾϵͳ��������	
			t=0;
			cnt++;
			LCD_ShowxNum(60+48,170,cnt,3,16,0X80);	//��ʾ����
		}
	}
}

void buffproc(u8 *tembuff){
	int startflag =0,endflag = 0;
	int startindex = 0,endindex = 0;
	int i =0;
	for (i=0;i<(sizeof(tembuff)-1);i++){
			if ( tembuff[i] == 0xAA && tembuff[i+1] == 0x55){
				startflag = 1;
				startindex = i;}
			if (startflag == 1)
			{
				msg.GPS_velocity = *( float *)& (tembuff[startindex+2]);
				msg.AIM_velocity = *( float *)& (tembuff[startindex+6]);
				msg.CarModel = *( short int *)& (tembuff[startindex+10]);
				msg.BodyModel = *( short int *)& (tembuff[startindex+12]);
				msg.Serial = *( int *)& (tembuff[startindex+14]);
				msg.SystemState = *( short int *)& (tembuff[startindex+18]);
				msg.BrakeSingal = *( short int *)& (tembuff[startindex+20]);
			}
		}
	return;
	}

	
void makebuff( char *procbuff,const struct ToCom temp){
    //header
    procbuff[0] = 0xAA;
    procbuff[1] = 0x55;
    //gpsspeed
    procbuff[2] = *((( char *)&temp.VCU_velocity)+0);
    procbuff[3] = *((( char *)&temp.VCU_velocity)+1);
    procbuff[4] = *((( char *)&temp.VCU_velocity)+2);
    procbuff[5] = *((( char *)&temp.VCU_velocity)+3);
    //pwm 1-8
    procbuff[6] = *((( char *)&temp.channel_1)+0);
    procbuff[7] = *((( char *)&temp.channel_1)+1);
	
    procbuff[8] = *((( char *)&temp.channel_2)+0);
    procbuff[9] = *((( char *)&temp.channel_2)+1);

    procbuff[10] = *((( char *)&temp.channel_3)+0);
    procbuff[11] = *((( char *)&temp.channel_3)+1);
    //automode
    procbuff[12] = *((( char *)&temp.channel_4)+0);
    procbuff[13] = *((( char *)&temp.channel_4)+1);
    //index
    procbuff[14] = *((( char *)&temp.channel_5)+0);
    procbuff[15] = *((( char *)&temp.channel_5)+1);
	
    procbuff[16] = *((( char *)&temp.channel_6)+0);
    procbuff[17] = *((( char *)&temp.channel_6)+1);
    //systemstatus
    procbuff[18] = *((( char *)&temp.channel_7)+0);
    procbuff[19] = *((( char *)&temp.channel_7)+1);
    //brake
    procbuff[20] = *((( char *)&temp.channel_8)+0);
    procbuff[21] = *((( char *)&temp.channel_8)+1);
	
	procbuff[22] = *((( char *)&temp.abnormal)+0);
    procbuff[23] = *((( char *)&temp.abnormal)+1);
    //end
    procbuff[24] = 0x0d;
    procbuff[25] = 0x0a;

    return;
}

	

