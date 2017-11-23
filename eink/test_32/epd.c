/*******************************************************************************
  EPD DRIVER FOR STM32F2/4 w/ FSMC
  By ZephRay(zephray@outlook.com)
*******************************************************************************/
#include "epd.h"

/******************************************************************************/

unsigned char EPD_FB[600] ; //1bpp Framebuffer
unsigned char EPD_PICTURE[24] ; //fixed address of image

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
 
 //字体
//extern const unsigned char Font_Ascii_8X16E[];
//extern const unsigned char Font_Ascii_24X48E[];
//extern const unsigned char Font_Ascii_12X24E[];
//extern const unsigned char WQY_ASCII_24[];


#ifndef USE_H_SCREEN

//Init waveform, basiclly alternating between black and white
#define FRAME_INIT_LEN     33

const unsigned char wave_init[FRAME_INIT_LEN]=
{
  0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
  0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
  0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
  0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
};

//line delay for different shades of grey
//note that delay is accumulative
//it means that if it's the level 4 grey
//the total line delay is 90+90+90+90=360
//this could be used as a method of rough gamma correction
//these settings only affect 4bpp mode
const unsigned char timA[16] =
{
// 1  2  3  4  5  6  7  8  9 10 11  12  13  14  15
  90,90,90,90,90,90,90,90,90,90,120,120,120,120,200
};

#define timB 20

//this only affect 1bpp mode
#define clearCount 4


#define setCount 4

//HCSAN模式
#else
#define FRAME_INIT_LEN     200
const unsigned char wave_init[FRAME_INIT_LEN]=
{
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //1
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //2
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //3
    
0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,  //4
0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,  //5
0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,  //6  
0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,  //7  
    
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //8  
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //9  
0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,  //10       
};


const unsigned char timA[16] =
{
// 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
  30,20,20,15,15,15,15,15,20,20,20,20,20,40,50
};

#define timB 40

#define clearCount 2
#define setCount 2

#endif


#define CLEAR_CNT   5


unsigned char g_dest_data[200];//Line data buffer



void DelayCycle(unsigned long x)
{
  while (x--)
  {
      usleep_range(1, 2);
  }
}

//Us delay that is not accurate
void Delay_Us(unsigned long x)
{
  unsigned long a;
  
  while (x--)
  {
    a = 17;
    while (a--)
    {
        usleep_range(1, 2);
    }
  }
}

void EPD_GPIO_Init(void)
{
    int  err = 0;
    bool val = false;
    val  = gpio_is_valid(__GPIO_EPD_XCL);
    val &= gpio_is_valid(__GPIO_EPD_EN_S);
    val &= gpio_is_valid(__GPIO_EPD_EN_D);
    if (!val) {
        printk("epd port not valid.\n");
        return -1;
    }
    err  = gpio_request(__GPIO_EPD_XCL,  "xcl");
    err |= gpio_request(__GPIO_EPD_EN_S, "en_s");
    err |= gpio_request(__GPIO_EPD_EN_D, "en_d");
    if (err != 0) {
        printk("epd port requst failed. %d\n", err);
        return -1;
    }

    gpio_direction_output(__GPIO_EPD_XCL, 0);
    gpio_direction_output(__GPIO_EPD_EN_S, 0);
    gpio_direction_output(__GPIO_EPD_EN_D, 0);


    //all pins output low default
    hc595_write16(0x0000);

    printk("epd port init successful.\n");
}


void EPD_Power_On_Vvdd(void)
{

}


void EPD_Power_Off_Vpos(void)
{
	EPD_EN_P_L();
}


void EPD_Power_On_Vpos(void)
{
	EPD_EN_P_H();
}



void EPD_Power_Off_Vneg(void)
{
	EPD_EN_N_L();
}


void EPD_Power_OnVneg(void)
{
	EPD_EN_N_H();
}


void EPD_Init(void)
{
  unsigned long i;
  
  EPD_GPIO_Init();	
	
  EPD_SHR_L();
  EPD_GMODE1_H();
  EPD_GMODE2_H();
  EPD_XRL_H();


  EPD_LE_L();
  EPD_CL_L();
  EPD_OE_L();
  EPD_SPH_H();
  EPD_SPV_H();
  EPD_CKV_L();
  
  for (i=0;i<600;i++)
  {
    EPD_FB[i]=0x55;
  }
}

/* 发送一行数据 */
void EPD_Send_Row_Data(u8 *pArray)  
{
    unsigned char i;

    //锁住数据
    EPD_LE_H();   
    EPD_CL_H();
    EPD_CL_L();
    EPD_CL_H();
    EPD_CL_L();
    
    EPD_LE_L();  
    EPD_CL_H();
    EPD_CL_L();
    EPD_CL_H();
    EPD_CL_L();

    EPD_OE_H();

    //开始一行数据
    EPD_SPH_L();                                          

    for (i=0;i<200;i++)
    {
//        GPIOC->ODR = pArray[i];
        __EPD_DATA(pArray[i]);
        EPD_CL_H();
        //DelayCycle(1);
        EPD_CL_L();
    }

    EPD_SPH_H();

    EPD_CL_H();
    EPD_CL_L();
    EPD_CL_H();
    EPD_CL_L();

    //输出数据
    EPD_CKV_L();
    EPD_OE_L();

    EPD_CL_H();
    EPD_CL_L();
    EPD_CL_H();
    EPD_CL_L();

    EPD_CKV_H();     
}


/* 跳过一行 */
void EPD_SkipRow(void)  
{
  unsigned char i;
  unsigned short a;
  
//  a = GPIOC->IDR & 0xFF00;
  
  EPD_LE_H(); 
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_LE_L();
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_OE_H();
  
  EPD_SPH_L();                                          
  
  for (i=0;i<200;i++)
  {
//    GPIOC->ODR = a;
    __EPD_DATA(a);
    EPD_CL_H();
    //DelayCycle(1);
    EPD_CL_L();
  }
  
  EPD_SPH_H();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_CKV_L();
  EPD_OE_L();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_CKV_H(); 
}

/* 慢速发送一行 */
void EPD_Send_Row_Data_Slow(u8 *pArray,unsigned char timingA,unsigned char timingB)  
{
  unsigned char i;
  unsigned short a;
  
//  a = GPIOC->IDR & 0xFF00;
  
  EPD_OE_H();
  
  EPD_SPH_L();                                          
  
  for (i=0;i<200;i++)
  {
//    GPIOC->ODR = pArray[i];
    __EPD_DATA(pArray[i]);
    EPD_CL_H();
    //DelayCycle(1);
    EPD_CL_L();
  }
  
  EPD_SPH_H();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_LE_H(); 
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_LE_L();
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_CKV_H();
  
  DelayCycle(timingA);
  
  EPD_CKV_L();
  
  DelayCycle(timingB);
  
  EPD_OE_L();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();  
  
}


/* CKV心跳 */
void EPD_Vclock_Quick(void)
{
  unsigned char i;
  
  for (i=0;i<2;i++)
  {
    EPD_CKV_L();
    DelayCycle(20);
    EPD_CKV_H();
    DelayCycle(20);
  }
}

/* 开始一帧数据 */
void EPD_Start_Scan(void)
{ 
  EPD_GMODE1_H();
  EPD_GMODE2_H();
    
  EPD_SPV_H();

  EPD_Vclock_Quick();
 
  EPD_SPV_L();

  EPD_Vclock_Quick();
  
  EPD_SPV_H();
  
  EPD_Vclock_Quick();
}

/* 结束一帧 */
void EPD_Stop_Scan(void)
{
  EPD_GMODE1_L();
  EPD_GMODE2_L();        
}



/* 显示清屏 */
void EPD_Clear(uint8_t mode)
{
    unsigned int line,frame,k;
    uint8_t data1, data2;
    
    switch (mode) {
        case CLEAR_MODE_BLACK:
            data1 = BYTE_BLACK;
            data2 = BYTE_BLACK;
            break;
        case CLEAR_MODE_WHITE:
            data1 = BYTE_WHITE;
            data2 = BYTE_WHITE;
            break;
        case CLEAR_MODE_STREAK:
            data1 = BYTE_WHITE;
            data2 = BYTE_BLACK;
            break;        
    }
    
    for(frame=0; frame < CLEAR_CNT; frame++)			
    {
        EPD_Start_Scan();
        for(line=0; line<600; line++)
        {
            for(k=0;k<200;k++) {
                
                if (k % 2)
                    g_dest_data[k]= data1;	
                else 
                    g_dest_data[k]= data2;	
                //g_dest_data[k]= BYTE_BLACK;
            } 
            EPD_Send_Row_Data( &g_dest_data[0] ); 
        }
        EPD_Stop_Scan();
    }
}



/* 显示数组 */
void EPD_Show_Array(const uint8_t *array)
{
    unsigned short line,frame,i;

    for(frame=0; frame < CLEAR_CNT; frame++)			
    {
        EPD_Start_Scan();
        for(line=0; line<600; line++)
        {
            for(i=0;i<100;i++) {
                g_dest_data[i]= array[i];	
            }
            EPD_Send_Row_Data( g_dest_data );
        }
    }
}



/* 局部刷新 */
void EPD_Part_Refresh(const uint8_t *array)
{
    unsigned short line,frame,i;
    
    uint32_t off_part1 = 300;
    uint32_t off_part2 = 600;

    for(frame=0; frame < CLEAR_CNT; frame++)			
    {
        EPD_Start_Scan();
        /* part 1 */
        for(line=0; line<off_part1; line++)   //600列
        {
            for(i=0;i<200;i++) {        //400行
                //g_dest_data[i]= array[i];	
                g_dest_data[i]= BYTE_WHITE;	
            }
            //EPD_SkipRow();
            EPD_Send_Row_Data( g_dest_data );
        }
        
        /* part 2*/
        for(line=off_part1; line<off_part2; line++)   //600列
        {
            for(i=0;i<200;i++) {        //400行
                //g_dest_data[i]= array[i];
                g_dest_data[i]= BYTE_BLACK;	
            }
            EPD_Send_Row_Data( g_dest_data );
        }
//        
//        /* part 3 */
//        for(line=off_part2; line<600; line++)   //600列
//        {
//            for(i=0;i<200;i++) {        //400行
//                //g_dest_data[i]= array[i];	
//                g_dest_data[i]= BYTE_WHITE;	
//            }
//            //EPD_SkipRow();
//            EPD_Send_Row_Data( g_dest_data );
//        }
    }
}




/* 画线函数 */
void EPD_EncodeLine_Pic(u8 *new_pic, u8 frame)//Encode data for grayscale image
{
	int i,j;
        unsigned char k,d;
	
        j=0;
	for(i=0; i<200; i++)
	{
          d = 0;
          k = new_pic[j++];
          if ((k&0x0F)>frame) d |= 0x10;
          if ((k>>4)>frame)   d |= 0x40;
          k = new_pic[j++];
          if ((k&0x0F)>frame) d |= 0x01;
          if ((k>>4)>frame)   d |= 0x04;
          g_dest_data[i] = d;	
	}	
}



void EPD_DispPic(void)//Display image in grayscale mode
{
  unsigned short frame;
  signed long line;
  unsigned long i;
  unsigned char *ptr;
  
  ptr = (unsigned char *)EPD_PICTURE;
  
  for(frame=0; frame<15; frame++)					
  {
    EPD_Start_Scan();
    for (i=0;i<200;i++) g_dest_data[i]=0x00;
    for(line=0; line<70; line++)
    {
      EPD_Send_Row_Data_Slow(g_dest_data,timA[frame],timB);
    }
    for(line=(530-1); line>=0; line--)
    {
      EPD_EncodeLine_Pic(ptr + line*400, frame);		
      EPD_Send_Row_Data_Slow(g_dest_data,timA[frame],timB);		
    }	
    
    EPD_Send_Row_Data( g_dest_data );				
  }
}





void EPD_DispPicture(void)
{
    unsigned short frame;
    signed long line;
    unsigned long i;
    unsigned long j = 0;
    unsigned char *ptr;

    ptr = (unsigned char *)EPD_FB;
  
    for(frame=0; frame < CLEAR_CNT; frame++)			
    {
        j = 0;
        EPD_Start_Scan();
        for(line=0; line<600; line++)
        {
            for(i=0;i<200;i++) {
                g_dest_data[i]= ptr[j];	
                j++;
            }
            EPD_Send_Row_Data( g_dest_data );
        }
    }
}



//Encode data for monochrome image
//From white to image
void EPD_EncodeLine_To(u8 *new_pic)
{
	int i,j;
        unsigned char k,d;
	
        j=0;
	for(i=0; i<100; i++)
	{
          k = new_pic[i];
          d = 0;
          if (k&0x01) d |= 0x40;
          if (k&0x02) d |= 0x10;
          if (k&0x04) d |= 0x04;
          if (k&0x08) d |= 0x01;
          g_dest_data[j++] = d;
          d = 0;
          if (k&0x10) d |= 0x40;
          if (k&0x20) d |= 0x10;
          if (k&0x40) d |= 0x04;
          if (k&0x80) d |= 0x01;
          g_dest_data[j++] = d;
	}	
}

//Encode data for clearing a monochrome image
//From image to black
void EPD_EncodeLine_From(u8 *new_pic)
{
	int i,j;
        unsigned char k,d;
	
        j=0;
	for(i=0; i<100; i++)
	{
          k = ~new_pic[i];
          d = 0;
          if (k&0x01) d |= 0x40;
          if (k&0x02) d |= 0x10;
          if (k&0x04) d |= 0x04;
          if (k&0x08) d |= 0x01;
          g_dest_data[j++] = d;
          d = 0;
          if (k&0x10) d |= 0x40;
          if (k&0x20) d |= 0x10;
          if (k&0x40) d |= 0x04;
          if (k&0x80) d |= 0x01;
          g_dest_data[j++] = d;
	}	
}

//Display image in monochrome mode
void EPD_DispScr(unsigned int startLine, unsigned int lineCount)
{
  unsigned short frame;
  signed short line;
  unsigned long i;
  unsigned char *ptr;
  unsigned long skipBefore,skipAfter;
  
  ptr = EPD_FB;
  
  skipBefore = 600-startLine-lineCount;
  skipAfter = startLine;
  
  for(frame=0; frame<setCount; frame++)					
  {
    EPD_Start_Scan();
    for(line=0;line<skipBefore;line++)
    {
      EPD_EncodeLine_To(ptr);
      EPD_SkipRow();
    }
    for(line=(lineCount-1); line>=0; line--)
    {
      EPD_EncodeLine_To(ptr + (line+startLine)*100);
      EPD_Send_Row_Data( g_dest_data );
    }
    EPD_Send_Row_Data( g_dest_data );
    for(line=0;line<skipAfter;line++)
    {
      EPD_EncodeLine_To(ptr);
      EPD_SkipRow();
    }
    EPD_Send_Row_Data( g_dest_data );					
  }
  
}

//Clear image in monochrome mode
void EPD_ClearScr(unsigned int startLine, unsigned int lineCount)
{
  unsigned short frame;
  signed short line;
  unsigned long i;
  unsigned char *ptr;
  unsigned long skipBefore,skipAfter;
  
  ptr = EPD_FB;
  
  skipBefore = 600-startLine-lineCount;
  skipAfter = startLine;
  
  for(frame=0; frame<clearCount; frame++)					
  {
    EPD_Start_Scan();
    for(line=0;line<skipBefore;line++)
    {
      EPD_EncodeLine_From(ptr);
      EPD_SkipRow();
    }
    for(line=(lineCount-1); line>=0; line--)
    {
      EPD_EncodeLine_From(ptr + (line+startLine)*100);
      EPD_Send_Row_Data( g_dest_data );
    }
    EPD_Send_Row_Data( g_dest_data );
    for(line=0;line<skipAfter;line++)
    {
      EPD_EncodeLine_From(ptr);
      EPD_SkipRow();
    }
    EPD_Send_Row_Data( g_dest_data );					
  }
  
  for(frame=0; frame<4; frame++)					
  {
    EPD_Start_Scan();
    for(line=0;line<skipBefore;line++)
    {
      EPD_SkipRow();
    }
    for(line=(lineCount-1); line>=0; line--)
    {
      for(i=0;i<200;i++)  g_dest_data[i]=0xaa;
      EPD_Send_Row_Data( g_dest_data );
    }
    EPD_Send_Row_Data( g_dest_data );
    for(line=0;line<skipAfter;line++)
    {;
      EPD_SkipRow();
    }
    EPD_Send_Row_Data( g_dest_data );					
  }
}


void EPD_ClearFB(unsigned char c)
{
  unsigned long i;
  
  for (i=0;i<60000;i++)
    EPD_FB[i]=c;
}


void EPD_SetPixel(unsigned short x,unsigned short y,unsigned char color)
{
  unsigned short x_bit;
  unsigned long x_byte;  
  
  if ((x<800)&&(y<600))
  {
    x_byte=x/8+y*100;
    x_bit=x%8;             
  
    EPD_FB[x_byte] &= ~(1 << x_bit);
    EPD_FB[x_byte] |= (color << x_bit);
  }
}

void EPD_XLine(unsigned short x0,unsigned short y0,unsigned short x1,unsigned short color)
{
  unsigned short i,xx0,xx1;
  
  xx0=MIN(x0,x1);
  xx1=MAX(x0,x1);
  for (i=xx0;i<=xx1;i++)
  {
    EPD_SetPixel(i,y0,color);
  }
}

void EPD_YLine(unsigned short x0,unsigned short y0,unsigned short y1,unsigned short color)
{
  unsigned short i,yy0,yy1;
  
  yy0=MIN(y0,y1);
  yy1=MAX(y0,y1);
  for (i=yy0;i<=yy1;i++)
  {
    EPD_SetPixel(x0,yy1,color);
  }
}

void EPD_Line(unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1,unsigned short color)
{
  int temp;
  int dx,dy;               //定义起点到终点的横、纵坐标增加值
  int s1,s2,status,i;
  int Dx,Dy,sub;

  dx=x1-x0;
  if(dx>=0)                 //X的方向是增加的
    s1=1;
  else                     //X的方向是降低的
    s1=-1;     
  dy=y1-y0;                 //判断Y的方向是增加还是降到的
  if(dy>=0)
    s2=1;
  else
    s2=-1;

  Dx=abs(x1-x0);             //计算横、纵标志增加值的绝对值
  Dy=abs(y1-y0);
  if(Dy>Dx)                 //               
  {                     //以45度角为分界线，靠进Y轴是status=1,靠近X轴是status=0 
    temp=Dx;
    Dx=Dy;
    Dy=temp;
    status=1;
  } 
  else
    status=0;

/********判断垂直线和水平线********/
  if(dx==0)                   //横向上没有增量，画一条水平线
    EPD_YLine(x0,y0,y1,color);
  if(dy==0)                   //纵向上没有增量，画一条垂直线
    EPD_XLine(x0,y0,x1,color);


/*********Bresenham算法画任意两点间的直线********/ 
  sub=2*Dy-Dx;                 //第1次判断下个点的位置
  for(i=0;i<Dx;i++)
  { 
    EPD_SetPixel(x0,y0,color);           //画点 
    if(sub>=0)                               
    { 
      if(status==1)               //在靠近Y轴区，x值加1
        x0+=s1; 
      else                     //在靠近X轴区，y值加1               
        y0+=s2; 
      sub-=2*Dx;                 //判断下下个点的位置 
    } 
    if(status==1)
      y0+=s2; 
    else       
      x0+=s1; 
    sub+=2*Dy;   
  } 
}


void EPD_PutChar_16(unsigned short x, unsigned short y, unsigned short chr, unsigned char color)
{
//  unsigned short x1,y1;
//  unsigned short ptr;
//  
//  ptr=(chr-0x20)*16;
//  for (y1=0;y1<16;y1++)
//  {
//    for (x1=0;x1<8;x1++)
//    {
//      if ((Font_Ascii_8X16E[ptr]>>x1)&0x01)
//        EPD_SetPixel(x+x1,y+y1,color);
//      else
//        EPD_SetPixel(x+x1,y+y1,1-color);
//    }
//    ptr++;
//  }
}

void EPD_PutChar_24(unsigned short x, unsigned short y, unsigned short chr, unsigned char color)
{
//  unsigned short x1,y1,b;
//  unsigned short ptr;
//  
//  ptr=(chr-0x20)*48;
//  for (y1=0;y1<24;y1++)
//  {
//      for (b=0;b<8;b++)
//        if ((Font_Ascii_12X24E[ptr]<<b)&0x80)
//          EPD_SetPixel(x+b,y+y1,color); 
//        else
//          EPD_SetPixel(x+b,y+y1,1-color);
//      ptr++; 
//      for (b=0;b<4;b++)
//        if ((Font_Ascii_12X24E[ptr]<<b)&0x80)
//          EPD_SetPixel(x+8+b,y+y1,color); 
//        else
//          EPD_SetPixel(x+8+b,y+y1,1-color);
//      ptr++; 
//  }
}

void EPD_PutChar_48(unsigned short x, unsigned short y, unsigned short chr, unsigned char color)
{
//  unsigned short x1,y1,b;
//  unsigned short ptr;
//  
//  ptr=(chr-0x20)*144;
//  for (y1=0;y1<48;y1++)
//  {
//    for (x1=0;x1<24;x1+=8)
//    {
//      for (b=0;b<8;b++)
//        if ((Font_Ascii_24X48E[ptr]>>b)&0x01)
//          EPD_SetPixel(x+x1+b,y+y1,color); 
//        else
//          EPD_SetPixel(x+x1+b,y+y1,1-color);
//      ptr++;
//    }  
//  }
}

void EPD_String_16(unsigned short x,unsigned short y,unsigned char *s,unsigned char color)
{
  unsigned short x1;
  
  x1=0;
  while(*s)
  {
    if (*s<128)
    {
      EPD_PutChar_16(x+x1,y,*s++,color);
      x+=8;
    }
  }
}

void EPD_String_24(unsigned short x,unsigned short y,unsigned char *s,unsigned char color)
{
  unsigned short x1;
  
  x1=0;
  while(*s)
  {
    if (*s<128)
    {
      EPD_PutChar_24(x+x1,y,*s++,color);
      x+=12;
    }
  }
}

void EPD_FillRect(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,unsigned char color)
{
  unsigned short x,y;
  for (x=x1;x<x2;x++)
    for (y=y1;y<y2;y++)
      EPD_SetPixel(x,y,color);
}


uint16_t wave[4] = {0x55aa, 0xaa55, 0x55aa, 0xaa55};

static int  __init empty_init(void)
{
    int i = 0;
    int err = 0;

    err = hc595_init();

    EPD_Power_On_Vvdd();
    EPD_Init();
    msleep(200);

    EPD_Power_OnVneg();
    msleep(200);

    EPD_Power_On_Vpos();
    msleep(200);

    EPD_Clear(CLEAR_MODE_BLACK);

    // for (i = 0; i < 100; i++) {
        // hc595_write16(i | (i << 8));
        // gpio_set_value(__GPIO_EPD_XCL, i % 2);
        // usleep_range(1000, 1500);
        // msleep(200);
    // }

    printk("---dirver init---\r\n");
    return 0;
}

static void  __exit empty_exit(void)
{
    hc595_write16(0x0000);
    hc595_uninit();

    gpio_free(__GPIO_EPD_XCL);
    gpio_free(__GPIO_EPD_EN_S);
    gpio_free(__GPIO_EPD_EN_D);
    printk("epd port uninit.\n");

    printk("---dirver exit---\r\n");
}

module_init(empty_init);
module_exit(empty_exit);

MODULE_LICENSE("GPL");



