#include "reg51.h"
#include "oled.h"
#include "intrins.h"
//引脚说明
//p1.0 SCL 显示屏
//p1.1 SDA 显示屏
//p1.2 继电器
//p1.3 蜂鸣器
//p2 点阵

typedef unsigned int uint;	  //对数据类型进行声明定义
typedef unsigned char uchar;
//定义矩阵键盘
#define KEY P2
sbit power=P1^2;
sbit beep=P1^3;
sbit btn=P1^4;
//全局定时函数
uint i0=0,flag=1;
uint time=0;

//密码存储
uchar pwd[][6]={
	{0,0,0,0,0,0},
	{4,4,3,1,0,2}
};
//背景音乐
code uint bg[]={1046,1175,1318,1397,1568,1760,1976,1046};

//指纹模块的一些指令
code unsigned char FPMXX_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //协议包头
code unsigned char FPMXX_Get_Img[6] = {0x01,0x00,0x03,0x01,0x0,0x05};    //获得指纹图像
code unsigned char FP_Search_0_9[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x0,0x13,0x0,0x21}; //搜索0-9号指纹
code unsigned char FP_Img_To_Buffer1[7]={0x01,0x0,0x04,0x02,0x01,0x0,0x08}; //将图像放入到BUFFER1
unsigned char FPMXX_RECEICE_BUFFER[24];

//****************** 指纹解锁模块
 //初始化模块
void UART_init()
{
      TMOD = 0x21;//确定T1工作方式 自动重载8位定时器，收TR1控制
      TH1 = 0xfd;//装初值
      TL1 = 0xfd;
	  TH0 = 0xff;//0.01毫秒中断一次
	  TL0 = 0x9c;
	  TR0 = 0;
	  ET0 = 1;//开中断
      TR1 = 1;//开启定时器
      REN = 1;//允许串行口接受
      SM0 = 0;//方式2
      SM1 = 1;
      ES = 1;//串行口中断打开
	  EA=1;
}
//发送数据
void UART_Send_Byte(unsigned char c)//UART Send a byte
{
	SBUF = c;
	while(!TI);
	TI = 0;
}
//接受数据
unsigned char UART_Receive_Byte()//UART Receive a byteg
{	
	unsigned char dat;
	while(!RI);
	dat = SBUF;
	RI = 0;	
	return (dat);
}
void FPMXX_Cmd_Send_Pack_Head(void) //发送包头
{
		int i;
	
		for(i=0;i<6;i++) //包头
    {
      UART_Send_Byte(FPMXX_Pack_Head[i]);   
    }
}

//FINGERPRINT_获得指纹图像命令
void FPMXX_Cmd_Get_Img()
{
    unsigned char i;

    FPMXX_Cmd_Send_Pack_Head(); //发送通信协议包头
	
    for(i=0;i<6;i++) //发送命令 0x1d
       UART_Send_Byte(FPMXX_Get_Img[i]);
}

//讲图像转换成特征码存放在Buffer1中
void FINGERPRINT_Cmd_Img_To_Buffer1(void)
{
 	    unsigned char i;
    
	       FPMXX_Cmd_Send_Pack_Head(); //发送通信协议包头
           
   		   for(i=0;i<7;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer1[i]);
   		     }
}

//搜索用户
void FINGERPRINT_Cmd_Search_Finger_Admin(void)
{
       unsigned char i;	   
			 
	     FPMXX_Cmd_Send_Pack_Head(); //发送通信协议包头

       for(i=0;i<11;i++)
           {
    	      UART_Send_Byte(FP_Search_0_9[i]);   
   		   }


}

//接收反馈数据缓冲
void FPMXX_Receive_Data(unsigned char ucLength)
{
  unsigned char i;
  for (i=0;i<ucLength;i++)
    FPMXX_RECEICE_BUFFER[i] = UART_Receive_Byte();
}


//延时1s函数
void delay1s(uchar i)   //误差 0us
{
	while(i--){
	    unsigned char a,b,c;
	    for(c=167;c>0;c--)
	        for(b=171;b>0;b--)
	            for(a=16;a>0;a--);
	    _nop_();  //if Keil,require use intrins.h
	}
}

//延时函数
void delay(uint i){
   while(i--);
}
//延时200ms
//延时函数
void delay200ms(uchar i)   //误差 0us
{  
	while(i--){
    unsigned char a,b;
    for(b=173;b>0;b--)
        for(a=143;a>0;a--);
	}
}
//蜂鸣器发声程序
void play(uint dat,uchar times){
	if(dat!=0){
		int j=15000;
		i0=0;
		time=10000/dat+0.5;
		TR0 = 1;//启动定时器	
		delay200ms(times);
		TR0 = 0;//关闭定时器
		beep=1;
	}
}
//显示等待认证
void  showwait(){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,6);//等
	OLED_ShowCHinese(18,3,7);//待
	OLED_ShowCHinese(36,3,8);//认
	OLED_ShowCHinese(54,3,9);//证
	OLED_ShowString(72,3,"...",16);
}
//认证失败
void showerror(){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,8);//认
	OLED_ShowCHinese(18,3,9);//	证
	OLED_ShowCHinese(36,3,24);//失
	OLED_ShowCHinese(54,3,25);//败
	OLED_ShowString(72,3,"!",16);
	play(bg[7],5);

}
//显示认证成功
void  showok(uchar name){
	OLED_ShowString(0,3,"                ",16);
	OLED_ShowCHinese(0,3,10);//成
	OLED_ShowCHinese(18,3,11);//功
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
	play(bg[5],5);//播放提示音
	power=1; //打开继电器
	delay1s(3);
	power=0;

}
//按键处理函数
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
//char转字符
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
//密码显示函数
void showpwd(uchar num,uchar position){
	OLED_ShowCHinese(0,3,22);//密
	OLED_ShowCHinese(18,3,23);//码
	OLED_ShowString(36,3,":",16);
	OLED_ShowChar((44+position*8),3,changechar(num),16);
	delay(20000);
	OLED_ShowChar((44+position*8),3,'*',16);
}
//密码输入处理函数
uchar pwdprocess(){
	uchar keyvalue;
	uint j,i=0;
	//按键输入的位数
	uchar position=1;
	//自动进入死循环，超时或者密码输入完毕后时可以退出
	while(1){
		//超时统计
		i++;
		if(keyvalue=keyprocess()){
			//如果你输入大于9默认不显示
			if(!(--keyvalue<=9)){continue;i=0;}
			pwd[0][position]=keyvalue;
			while(KEY!=0xf0);
			showpwd(pwd[0][position],position);
			position++;
			i=0;
			//密码输入完毕
			if(position==6){
				//比对密码
				for(j=0;j<6;j++){
					if(pwd[0][j]!=pwd[1][j]){showerror();delay1s(1);return 0;}	
				}
				//密码输入成功				
				showok(5); //显示成功
				return 1;	
			}
		}
		//超时处理函数
		if(i>=60000) return 0;		
	}
}

void main(void){
uint i=0;
uchar keyvalue;	
UART_init();//初始化串口
	power=0;
	OLED_Init();			//初始化OLED  
	OLED_Clear(); 				
	OLED_ShowCHinese(3,0,0);//小
	OLED_ShowCHinese(21,0,1);//游
	OLED_ShowCHinese(39,0,2);//指
	OLED_ShowCHinese(57,0,3);//纹
	OLED_ShowCHinese(75,0,5);//锁
	OLED_ShowString(93,0,"v1.0",16);
	showwait();
	beep=1;
		
	while(1) 
	{	
		i++;
		//按键判断
		if(keyvalue=keyprocess()){
			keyvalue--;
			//如果输入的数小于9那么就自动进入密码处理函数
			if(keyvalue<=9){
				pwd[0][0]=keyvalue;
				//显示密码
				OLED_ShowString(0,3,"                ",16);
				showpwd(keyvalue,0);
				if(!pwdprocess()){
					//到这里说明输入错误或者超时
					showwait();
				}else{
					//到这里说说明输入成功！	
					showwait();
				}
			}	 
		}
		if(btn==0) showok(5);
		if(i==10000){
		flag=0;
		//指纹判断
			FPMXX_Cmd_Get_Img(); //获得指纹图像
			FPMXX_Receive_Data(12);//接收12个长度的反馈码
			if(FPMXX_RECEICE_BUFFER[9]==0x00){//接受到指纹
		   	  FINGERPRINT_Cmd_Img_To_Buffer1();
			  FPMXX_Receive_Data(12);
			  if(FPMXX_RECEICE_BUFFER[9]==0x0){
			  	 FINGERPRINT_Cmd_Search_Finger_Admin();//搜索指纹
				 FPMXX_Receive_Data(16);
				 if(FPMXX_RECEICE_BUFFER[9]==0x0){//搜索到了指纹
					 //这里是搜索到了指纹，开始匹配
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