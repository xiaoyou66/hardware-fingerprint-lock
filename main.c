#include "reg51.h"
#include "oled.h"
#include "intrins.h"
//����˵��
//p1.0 SCL ��ʾ��
//p1.1 SDA ��ʾ��
//p1.2 �̵���
//p1.3 ������
//p2 ����

typedef unsigned int uint;	  //���������ͽ�����������
typedef unsigned char uchar;
//����������
#define KEY P2
sbit power=P1^2;
sbit beep=P1^3;
sbit btn=P1^4;
//ȫ�ֶ�ʱ����
uint i0=0,flag=1;
uint time=0;

//����洢
uchar pwd[][6]={
	{0,0,0,0,0,0},
	{4,4,3,1,0,2}
};
//��������
code uint bg[]={1046,1175,1318,1397,1568,1760,1976,1046};

//ָ��ģ���һЩָ��
code unsigned char FPMXX_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //Э���ͷ
code unsigned char FPMXX_Get_Img[6] = {0x01,0x00,0x03,0x01,0x0,0x05};    //���ָ��ͼ��
code unsigned char FP_Search_0_9[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x0,0x13,0x0,0x21}; //����0-9��ָ��
code unsigned char FP_Img_To_Buffer1[7]={0x01,0x0,0x04,0x02,0x01,0x0,0x08}; //��ͼ����뵽BUFFER1
unsigned char FPMXX_RECEICE_BUFFER[24];

//****************** ָ�ƽ���ģ��
 //��ʼ��ģ��
void UART_init()
{
      TMOD = 0x21;//ȷ��T1������ʽ �Զ�����8λ��ʱ������TR1����
      TH1 = 0xfd;//װ��ֵ
      TL1 = 0xfd;
	  TH0 = 0xff;//0.01�����ж�һ��
	  TL0 = 0x9c;
	  TR0 = 0;
	  ET0 = 1;//���ж�
      TR1 = 1;//������ʱ��
      REN = 1;//�����пڽ���
      SM0 = 0;//��ʽ2
      SM1 = 1;
      ES = 1;//���п��жϴ�
	  EA=1;
}
//��������
void UART_Send_Byte(unsigned char c)//UART Send a byte
{
	SBUF = c;
	while(!TI);
	TI = 0;
}
//��������
unsigned char UART_Receive_Byte()//UART Receive a byteg
{	
	unsigned char dat;
	while(!RI);
	dat = SBUF;
	RI = 0;	
	return (dat);
}
void FPMXX_Cmd_Send_Pack_Head(void) //���Ͱ�ͷ
{
		int i;
	
		for(i=0;i<6;i++) //��ͷ
    {
      UART_Send_Byte(FPMXX_Pack_Head[i]);   
    }
}

//FINGERPRINT_���ָ��ͼ������
void FPMXX_Cmd_Get_Img()
{
    unsigned char i;

    FPMXX_Cmd_Send_Pack_Head(); //����ͨ��Э���ͷ
	
    for(i=0;i<6;i++) //�������� 0x1d
       UART_Send_Byte(FPMXX_Get_Img[i]);
}

//��ͼ��ת��������������Buffer1��
void FINGERPRINT_Cmd_Img_To_Buffer1(void)
{
 	    unsigned char i;
    
	       FPMXX_Cmd_Send_Pack_Head(); //����ͨ��Э���ͷ
           
   		   for(i=0;i<7;i++)   //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer1[i]);
   		     }
}

//�����û�
void FINGERPRINT_Cmd_Search_Finger_Admin(void)
{
       unsigned char i;	   
			 
	     FPMXX_Cmd_Send_Pack_Head(); //����ͨ��Э���ͷ

       for(i=0;i<11;i++)
           {
    	      UART_Send_Byte(FP_Search_0_9[i]);   
   		   }


}

//���շ������ݻ���
void FPMXX_Receive_Data(unsigned char ucLength)
{
  unsigned char i;
  for (i=0;i<ucLength;i++)
    FPMXX_RECEICE_BUFFER[i] = UART_Receive_Byte();
}


//��ʱ1s����
void delay1s(uchar i)   //��� 0us
{
	while(i--){
	    unsigned char a,b,c;
	    for(c=167;c>0;c--)
	        for(b=171;b>0;b--)
	            for(a=16;a>0;a--);
	    _nop_();  //if Keil,require use intrins.h
	}
}

//��ʱ����
void delay(uint i){
   while(i--);
}
//��ʱ200ms
//��ʱ����
void delay200ms(uchar i)   //��� 0us
{  
	while(i--){
    unsigned char a,b;
    for(b=173;b>0;b--)
        for(a=143;a>0;a--);
	}
}
//��������������
void play(uint dat,uchar times){
	if(dat!=0){
		int j=15000;
		i0=0;
		time=10000/dat+0.5;
		TR0 = 1;//������ʱ��	
		delay200ms(times);
		TR0 = 0;//�رն�ʱ��
		beep=1;
	}
}
//��ʾ�ȴ���֤
void  showwait(){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,6);//��
	OLED_ShowCHinese(18,3,7);//��
	OLED_ShowCHinese(36,3,8);//��
	OLED_ShowCHinese(54,3,9);//֤
	OLED_ShowString(72,3,"...",16);
}
//��֤ʧ��
void showerror(){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,8);//��
	OLED_ShowCHinese(18,3,9);//	֤
	OLED_ShowCHinese(36,3,24);//ʧ
	OLED_ShowCHinese(54,3,25);//��
	OLED_ShowString(72,3,"!",16);
	play(bg[7],5);

}
//��ʾ��֤�ɹ�
void  showok(uchar name){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,10);//��
	OLED_ShowCHinese(18,3,11);//��
	OLED_ShowString(36,3,"!  ",16);
	switch(name){
		case 1:
			OLED_ShowCHinese(62,3,1);//xx
			OLED_ShowCHinese(80,3,12);//xx
			break;
		case 2:
			OLED_ShowCHinese(62,3,13);//xx
			OLED_ShowCHinese(80,3,14);//xx
			OLED_ShowCHinese(98,3,15);//xx
			break;
		case 3:
			OLED_ShowCHinese(62,3,16);//xx
			OLED_ShowCHinese(80,3,17);//xx
			OLED_ShowCHinese(98,3,18);//xx
			break;
		case 4:
			OLED_ShowCHinese(62,3,19);//xx
			OLED_ShowCHinese(80,3,20);//xx
			break;
		default:
			break;
	}
	play(bg[5],5);//������ʾ��
	power=1; //�򿪼̵���
	delay1s(3);
	power=0;

}
//����������
uchar keyprocess(){
	uchar keyvalue=0;
	KEY=0x0f;
	if(KEY!=0x0f){
		delay(1000);
		if(KEY!=0xff){
			switch(KEY){
			 	case(0x07):keyvalue=1;break;
				case(0x0b):keyvalue=2;break;
				case(0x0d):keyvalue=3;break;
				case(0x0e):keyvalue=4;break;
			}
			KEY=0xf0;
			switch(KEY){
			 	case(0x70):keyvalue=keyvalue;break;
				case(0xb0):keyvalue=keyvalue+4;break;
				case(0xd0):keyvalue=keyvalue+8;break;
				case(0xe0):keyvalue=keyvalue+12;break;
			}	
		}
	}
	return keyvalue;	
} 
//charת�ַ�
char changechar(uchar i){
	switch(i){
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		default:return '*';
	}

}
//������ʾ����
void showpwd(uchar num,uchar position){
	OLED_ShowCHinese(0,3,22);//��
	OLED_ShowCHinese(18,3,23);//��
	OLED_ShowString(36,3,":",16);
	OLED_ShowChar((44+position*8),3,changechar(num),16);
	delay(20000);
	OLED_ShowChar((44+position*8),3,'*',16);
}
//�������봦����
uchar pwdprocess(){
	uchar keyvalue;
	uint j,i=0;
	//���������λ��
	uchar position=1;
	//�Զ�������ѭ������ʱ��������������Ϻ�ʱ�����˳�
	while(1){
		//��ʱͳ��
		i++;
		if(keyvalue=keyprocess()){
			//������������9Ĭ�ϲ���ʾ
			if(!(--keyvalue<=9)){continue;i=0;}
			pwd[0][position]=keyvalue;
			while(KEY!=0xf0);
			showpwd(pwd[0][position],position);
			position++;
			i=0;
			//�����������
			if(position==6){
				//�ȶ�����
				for(j=0;j<6;j++){
					if(pwd[0][j]!=pwd[1][j]){showerror();delay1s(1);return 0;}	
				}
				//��������ɹ�				
				showok(5); //��ʾ�ɹ�
				return 1;	
			}
		}
		//��ʱ������
		if(i>=60000) return 0;		
	}
}

void main(void){
uint i=0;
uchar keyvalue;	
UART_init();//��ʼ������
	power=0;
	OLED_Init();			//��ʼ��OLED  
	OLED_Clear(); 				
	OLED_ShowCHinese(3,0,0);//С
	OLED_ShowCHinese(21,0,1);//��
	OLED_ShowCHinese(39,0,2);//ָ
	OLED_ShowCHinese(57,0,3);//��
	OLED_ShowCHinese(75,0,5);//��
	OLED_ShowString(93,0,"v1.0",16);
	showwait();
	beep=1;
		
	while(1) 
	{	
		i++;
		//�����ж�
		if(keyvalue=keyprocess()){
			keyvalue--;
			//����������С��9��ô���Զ��������봦����
			if(keyvalue<=9){
				pwd[0][0]=keyvalue;
				//��ʾ����
				OLED_ShowString(0,3,"                ",16);
				showpwd(keyvalue,0);
				if(!pwdprocess()){
					//������˵�����������߳�ʱ
					showwait();
				}else{
					//������˵˵������ɹ���	
					showwait();
				}
			}	 
		}
		if(btn==0) showok(5);
		if(i==10000){
		flag=0;
		//ָ���ж�
			FPMXX_Cmd_Get_Img(); //���ָ��ͼ��
			FPMXX_Receive_Data(12);//����12�����ȵķ�����
			if(FPMXX_RECEICE_BUFFER[9]==0x00){//���ܵ�ָ��
		   	  FINGERPRINT_Cmd_Img_To_Buffer1();
			  FPMXX_Receive_Data(12);
			  if(FPMXX_RECEICE_BUFFER[9]==0x0){
			  	 FINGERPRINT_Cmd_Search_Finger_Admin();//����ָ��
				 FPMXX_Receive_Data(16);
				 if(FPMXX_RECEICE_BUFFER[9]==0x0){//��������ָ��
					 //��������������ָ�ƣ���ʼƥ��
					 switch(FPMXX_RECEICE_BUFFER[11]){
					 	case 0x01:showok(1);break;
						case 0x02:showok(2);break;
						case 0x03:showok(3);break;
						case 0x04:showok(4);break;
						default:showok(5);break;
					 }
					 showwait();
				 }else{
				 	showerror();
					delay1s(1);
					showwait();
				 }
			  }
		  }
		  i=0;
		 flag=1;
	}
 }	  	
}


void Timer0Interrupt(void) interrupt 1
{  
	TH0 = 0xff;
	TL0 = 0x9c;	
	i0++;
	if(i0>=time){
		beep=!beep;
		i0=0;
	}
}
  void ser() interrupt 4
 {   

	 if(flag) RI = 0;
}	