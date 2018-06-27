/**
*单片机通过ESP8266 WiFi模块将温度数据传送给APP
*
*APP发送指令控制单片机的LED灯，以下为APP发送给单片机的指令：
*
*‘1’为开红色led				‘2’为关红色led
*‘3’为开黄色led				‘4’为关黄色led
*‘5’为开蓝色led				‘6’为关蓝色led
*‘7’为发送温度数据		‘8’为停止发送温度数据
*
*/

#include <reg52.h>
#define uchar unsigned char 
#define uint unsigned int 

//红色LED
sbit RedLED=P2^0;
//黄色LED
sbit YellowLED=P2^3;
//蓝色LED
sbit BlueLED=P2^6;
//定义DS18B20的信号线端口
sbit DQ = P2^7; 
//接收数组
uchar receiveTable[10];
//温度存储数组
uchar dis_buffer[4];  
//发送温度数据标志位
uint send;
//用于串口中断数组
uint i;
//用于温度缓存数组
uchar m;
//用于定时器0
uchar n;

/**
*延时函数
*/
void delay(uint x)
{
	while(x)   
  x--;
}

/**
*DS18B20初始化函数
*/
void Init_DS18B20(void)
{
  unsigned char x=0;
  DQ = 1;         //DQ复位
  delay(8);       //稍做延时
	DQ = 0;         //单片机将DQ拉低
	delay(80);     	//精确延时 大于 480us
	DQ = 1;         //拉高总线
	delay(14);
	x=DQ;           //稍做延时后 如果x=0则初始化成功 x=1则初始化失败
	delay(20);
}

/**
*从18B20中读一个字节
*/
uchar ReadOneChar(void)
{
	uchar i=0;
	uchar dat = 0;
	for (i=8;i>0;i--)
  {
		DQ = 0;   // 给脉冲信号
		dat>>=1;
		DQ = 1;   // 给脉冲信号
		if(DQ)
		dat|=0x80;  
		delay(8);
	}
	return(dat);
}

/**
*向18B20中写一个字节
*/
void Write_OneChar(uchar dat)
{
	uchar i=0;
	for (i=8; i>0; i--)
	{
		DQ = 0;     //给脉冲信号
		DQ = dat & 0x01;
		delay(5); 
		DQ = 1;     //给脉冲信号
		dat >>= 1;
	}
	delay(4);
}

/**
*从18B20中读取一个字节
*/
int Read_Temperature(void)
{
	 uchar i = 0,t = 0,a,b;
	 int temp;
	 Init_DS18B20();
	 Write_OneChar(0xcc);   // 跳过读序号列号的操作
	 Write_OneChar(0x44);   // 启动温度转换
	 Init_DS18B20();
	 Write_OneChar(0xcc);   //跳过读序号列号的操作
	 Write_OneChar(0xbe);   //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
	 i = ReadOneChar();           //读取温度值低位
	 t = ReadOneChar();           //读取温度值高位
	 a = i & 0x0f;
	 b = t;
	 i = i >> 4;             //低位右移4位，舍弃小数部分
	 t = t << 4;             //高位左移4位，舍弃符号位
	 t = t | i;
	 temp = (t + a * 0.0625) * 100; //得到一个比实际温度扩到100倍的值，主要是为了更好的显示和传输          
	 return(temp);                  //返回温度值
}

/**
*初始化定时器0
*/
void timer0_init(void)                      
{
	 TMOD = 0x21; 	//定时器0工作在方式1，定时器1工作在方式2
	 TH0 = 0x4B;		//定时器0溢出时间50ms
	 TL0 = 0xFF;
	 EA = 1;				//开总中断
	 ET0 = 1;				//开定时器0中断
	 TR0 = 1;				//开定时器0
}

/**
*延时函数：单位 ms 
*/
void delayms(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)
		for(j=110;j>0;j--);
}

/**
*串口发送一个字节
*/
void sendChar(uchar a)
{
	SBUF = a;
	while(TI==0);
	TI=0;
}

/**
*串口发送字符串
*/
void sendString(uchar *s)
{
	while(*s!='\0')
	{
		sendChar(*s);
		s++;
	}
}

/**
*初始化ESP8266模块
*/
void initEsp()
{
	TMOD=0x21;		//定时器1工作在方式2
	TH1 = 0xfd;		//波特率9600
	TL1 = 0xfd;
	SM0=0;				//串口工作在方式1
	SM1=1;
	EA = 1;				//开总中断
	REN = 1;			//使能串口
	TR1 = 1;			//定时器1开始计时
	delayms(50);
	sendString("AT+CWMODE=2\r\n");		//station模式
	delayms(50);	
	sendString("AT+CIOBAUD=9600\r\n");//设置波特率9600
	delayms(50);	
	sendString("AT+CIPMUX=1\r\n");		//允许多连接
	delayms(50);	
	sendString("AT+CIPSERVER=1\r\n");	//建立TCP Server
	delayms(50);	
	ES = 1;				//开串口中断
}


void main()
{
		RedLED=0;
		YellowLED=0;
		BlueLED=0;
		i=0;
		send=0;
		initEsp();
		timer0_init();           //调用T0初始化函数
		while(1)
		{
			while(send)
			{
				delayms(2);					//发送温度数据
				sendString( "AT+CIPSTART=1,\"TCP\",\"192.168.4.2\",5000\r\n");
				delayms(2);
				sendString("AT+CIPSEND=1,4\r\n");
				delayms(2);
				for(m = 0;m <= 3;m++)
				{
					sendChar(dis_buffer[m] + 48);
				}			
				delayms(1000);			//发送间隔1s
			}
		}
}

/**
*定时器0中断，采集温度，采集频率2Hz
*/
void timer0_isr(void) interrupt 1
{
	int temp; 
  TR0 = 0;
  TH0 = 0x4B;												//重新赋初值
  TL0 = 0xFF;
  TR0 = 1;
  n++;
	if(n>=10)													//定时器时间为50ms，利用n来达到记时500ms，10*50
	{
		n = 0;
    temp = Read_Temperature();			//将温度数据存入数组
    dis_buffer[0] = temp / 1000;
    dis_buffer[1] = temp % 1000 / 100;
    dis_buffer[2] = temp % 100 / 10;
    dis_buffer[3] = temp % 10;
	}
 
}

/**
*串口中断，接收数据
*/
void uart() interrupt 4
{
	if(RI == 1)   
  {
		RI = 0;     									//清除串口接收标志位
		receiveTable[i]=SBUF;					//将接收到的数据存入缓存数组
		if(receiveTable[0]=='+')			//判断收到的数据格式是否合法
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
			switch(receiveTable[9])			//数组中第十个字符为收到的数据
			{
				case '1':									//‘1’为开红色led
					RedLED=0;
					break;
				case '2':									//‘2’为关红色led
					RedLED=1;
					break;
				case '3':									//‘3’为开黄色led
					YellowLED=0;
					break;
				case '4':									//‘4’为关黄色led
					YellowLED=1;
					break;
				case '5':									//‘5’为开蓝色led
					BlueLED=0;
					break;
				case '6':									//‘6’为关蓝色led
					BlueLED=1;
					break;
				case '7':									//‘7’为发送温度数据
					send=1;
					break;
				case '8':									//‘8’为停止发送温度数据
					send=0;
					break;
			}
		}
  }
}