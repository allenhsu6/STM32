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

	unsigned char head1; // 0xAA   									1 byte
	unsigned char head2; // 0x55
	float GPS_velocity; // gps���صĳ�����ǰ��ʵ�ٶȣ�������ʾ			4 byte
	float AIM_velocity; // ͨ������ó����ٶ�Ŀ��ֵ					4 byte
	short int CarModel; // 0������״̬ 1������״̬ ��ʾ				2 byte
	short int BodyModel; // 0Ϊֱ���� 1Ϊ8���ƻ� 2Ϊ����ѭ�� ��ʾ		2 byte
	unsigned int Serial; 		// ��Ϣ��							4 byte
	short int SystemState; // ��λ��״̬�� ��������					2 byte
	short int BrakeSingal; // ɲ�����źţ� ������ʾ					2 byte
	unsigned char end1;  // �س�										���ù�
	unsigned char end2;  // ����

}msg;

struct ToCom{

    short int soc;          // ���ص����ź�
    float VCU_velocity;     // ���س���
    unsigned int pwm;       // ң������ȫ����Ϣ
    short int abnormal;     // �����쳣�����ϴ�1

}msgToCom;


 int main(void)
 {
 	// ������ǰ�涨��
 	u8 key;
	u8 i=0,t=0;
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

		if((USART_RX_STA&0x8000) && key==KEY0_PRES)
		{
			len=USART_RX_STA&0x3fff;   //�õ��˴ν��յ������ݳ���
			printf("\r\n�����͵���Ϣ����Ϊ: %d λ\r\n",len);  //  ���ݳ���

			// �ܹ�22�� �����msg�ṹ����
			if(len == 22){
				msg.head1 = USART_RX_BUF[0];
				msg.head2 = USART_RX_BUF[1];
				msg.GPS_velocity = (float)USART_RX_BUF[2];
                strncpy(temp, USART_RX_BUF+2, 4); // ��src ��ַ��ʼ�������ĸ�
                msg.GPS_velocity = atoi(temp);

                msg.AIM_velocity = &USART_RX_BUF[6];
                msg.CarModel = (short int)USART_RX_BUF[10];
				msg.BodyModel = (short int)USART_RX_BUF[12];
				msg.Serial = (unsigned int)USART_RX_BUF[14];
				msg.SystemState = (short int)USART_RX_BUF[18];
				msg.BrakeSingal = (short int)USART_RX_BUF[20];
			}

			LCD_ShowxNum(60,210,msg.head2,3,16,0X80);	//��ʾ����
		 	LCD_ShowxNum(60,230,msg.head1,3,16,0X80);	//��ʾ����

            printf("\r\n�ɹ������msg��");
            printf("\r\n��msg�е�GPS_velocityΪ�� %f\r\n",msg.GPS_velocity);  //  ���ݳ���
			printf("\r\n��msg�е�AIM_velocityΪ�� %f\r\n",msg.AIM_velocity);  //  ���ݳ���

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



