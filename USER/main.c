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
	unsigned int Serial; 		// ��Ϣ��							4 byte
	short int SystemState; // ��λ��״̬�� ��������					2 byte
	short int BrakeSingal; // ɲ�����źţ� ������ʾ					2 byte
	unsigned char end1;  // �س�										���ù�
	unsigned char end2;  // ����

};

struct ToCom{

    short int soc;          // ���ص����ź�
    float VCU_velocity;     // ���س���
    unsigned int pwm;       // ң������ȫ����Ϣ
    short int abnormal;     // �����쳣�����ϴ�1

};


 int main(void)
 {
 	// ������ǰ�涨��
 	u8 key;
	u8 i=0,t=0,len=0;
	u8 cnt=0;
	u8 canbuf[8];  // u8�ǰ����ֽڶ����  canbuf�Ǹ�����
	u8 res;
	u8 mode=CAN_Mode_LoopBack;//CAN����ģʽ;CAN_Mode_Normal(0)����ͨģʽ��CAN_Mode_LoopBack(1)������ģʽ

	struct COM msg;         // ��com��
	struct ToCom msgToCom;  // ����com
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
				// msg.GPS_velocity = (float)USART_RX_BUF[2];
                strncpy(temp, USART_RX_BUF+2, 4); // ��src ��ַ��ʼ�������ĸ�
                msg.GPS_velocity = atof(temp);

                //msg.AIM_velocity = &USART_RX_BUF[6];
                strncpy(temp, USART_RX_BUF+6, 4);
                msg.AIM_velocity = atof(temp);

                strncpy(temp, USART_RX_BUF+10, 2);
                msg.CarModel = atoi(temp);

                strncpy(temp, USART_RX_BUF+12, 2);
                msg.BodyModel = atoi(temp);

                strncpy(temp, USART_RX_BUF+14, 4);
                msg.Serial = atoi(temp);

                strncpy(temp, USART_RX_BUF+18, 2);
                msg.SystemState = atoi(temp);

                strncpy(temp, USART_RX_BUF+20, 2);
                msg.BrakeSingal = atoi(temp);
			}

			LCD_ShowxNum(60,210,msg.head2,3,16,0X80);	//��ʾ����
		 	LCD_ShowxNum(60,230,msg.head1,3,16,0X80);	//��ʾ����

            printf("\r\n�ɹ������msg��");
            printf("\r\n��msg�е�GPS_velocityΪ�� %f\r\n",msg.GPS_velocity);
			printf("\r\n��msg�е�AIM_velocityΪ�� %f\r\n",msg.AIM_velocity);
            printf("\r\n��msg�е�CarModelΪ�� %d\r\n",    msg.CarModel);
            printf("\r\n��msg�е�BodyModelΪ�� %d\r\n",   msg.BodyModel);
            printf("\r\n��msg�е�SerialΪ�� %d\r\n",      msg.Serial);
            printf("\r\n��msg�е�SystemStateΪ�� %d\r\n", msg.SystemState);
            printf("\r\n��msg�е�BrakeSingleΪ�� %d\r\n", msg.BrakeSingal);



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



