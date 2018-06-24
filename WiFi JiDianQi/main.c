/**
* 通过wifi控制继电器，从而控制插座
*
*/

#include <reg52.h>
#define uchar unsigned char 
#define uint unsigned int 
uchar temp;
sbit JDQ=P1^0;

void delayms(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)
		for(j=110;j>0;j--);
}
//send one char
void sendChar(uchar a)
{
	SBUF = a;
	while(TI==0);
	TI=0;
	
}

//send String
void sendString(uchar *s)
{
	while(*s!='\0')
	{
		sendChar(*s);
		s++;
	}
		
}

//初始化ESP8266模块
void initEsp()
{
	TMOD=0x20;		//定时器1工作在方式2
	TH1 = 0xfd;		//波特率9600
	TL1 = 0xfd;
	SM0=0;				//串口工作在方式1
	SM1=1;
	EA = 1;				//开总中断
	REN = 1;			//使能串口
	TR1 = 1;			//定时器1开始计时
	delayms(200);
	sendString("AT+CWMODE=2\r\n");		//station模式
	delayms(200);	
	sendString("AT+CIOBAUD=9600\r\n");//设置波特率9600
	delayms(200);	
	sendString("AT+CIPMUX=1\r\n");		//允许多连接
	delayms(200);	
	sendString("AT+CIPSERVER=1\r\n");	//建立TCP Server
	delayms(200);	
	ES = 1;				//开串口中断
}

uchar receiveTable[10];
uint i;
void main()
{
	i=0;
	JDQ=0;
	initEsp();
	while(1);
	
}

void uart() interrupt 4
{
	if(RI == 1)   
  {
    RI = 0;     //清除串口接收标志位
		receiveTable[i]=SBUF;
		if(receiveTable[0]=='+')
		{
			i++;
		}
		else
		{
			i=0;
		}
		if(i==10)
		{
			i=0;
			switch(receiveTable[9])
			{
				case '0':
					JDQ=0;
					break;
				case '1':
					JDQ=1;
					break;
				
			}
		}
    
  }
}