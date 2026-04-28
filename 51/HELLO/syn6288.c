#include <reg52.h>
//#include "syn6288.h"
//#include "delay.h"
//#include "uart.h"
#include <string.h>

/**************??????*********************/
unsigned char SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //????
unsigned char SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //????
unsigned char SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //????
unsigned char SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //????
unsigned char SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //??POWER DOWN ????

unsigned char receive_ok = 0;  //?????
unsigned char command;         //????????

//????1ms
void  delay(unsigned int uldata)
{
    unsigned int j  =  0;
    unsigned int g  =  0;
    for (j = 0; j < uldata; j++)
        for (g = 0; g < 110; g++);
}


//??1???

void UART1_Init(void)
{
    SCON = 0x50;        //8???,?????
    TMOD |= 0x20;        //?????1?8???????
    TL1 = 0xFD;        //??????
    TH1 = 0xFD;        //????????
    ET1 = 0;        //?????1??
    TR1 = 1;        //?????1
    ES = 1;
    EA = 1;
    RI = 0;
    TI = 0;
}
/*?????????*/
void UART1_SendByte(unsigned char DAT)
{
    SBUF = DAT;
    while(TI == 0);
    TI=0;
}

/*?????????*/
void UART1_SendString(unsigned char *DAT, unsigned char len)
{
    unsigned char i;
    for(i = 0; i < len; i++)
    {
        UART1_SendByte(*DAT++);
    }
}


void SYN_FrameInfo(unsigned char Music, unsigned char *HZdata)
{
    /****************???????**********************************/
    unsigned char  Frame_Info[50];
    unsigned char  HZ_Length;
    unsigned char  ecc  = 0;              //??????
    unsigned int i = 0;
    HZ_Length = strlen((char*)HZdata);             //?????????

    /*****************???????**************************************/
    Frame_Info[0] = 0xFD ;             //????FD
    Frame_Info[1] = 0x00 ;             //???????????
    Frame_Info[2] = HZ_Length + 3;         //???????????
    Frame_Info[3] = 0x01 ;             //?????:??????
    Frame_Info[4] = 0x01 | Music << 4 ; //??????:??????

    /*******************?????***************************************/
    for(i = 0; i < 5; i++)                   //????????5?????
    {
        ecc = ecc ^ (Frame_Info[i]);        //????????????
    }

    for(i = 0; i < HZ_Length; i++)           //????????????
    {
        ecc = ecc ^ (HZdata[i]);                 //????????????
    }
    /*******************?????***************************************/
    memcpy(&Frame_Info[5], HZdata, HZ_Length);
    Frame_Info[5 + HZ_Length] = ecc;
    UART1_SendString(Frame_Info, 5 + HZ_Length + 1);
}


void  main(void)
{
    UART1_Init();
    SYN_FrameInfo(0, "[v12][t4]开机"); //??? ??
    delay(2000);
    receive_ok = 1;
    while(1)
    {
        //??????2?(0:?????  1-15:??????)
        //m[0~16]:0???????,16????????
        //v[0~16]:0???????,16??????
        //t[0~5]:0??????,5??????
        //??????????????

        /*????*/
        switch(command)
        {
            case 'n':       
            receive_ok = 0;
            SYN_FrameInfo(0, "[v16][t3]前方禁止鸣笛");  
            delay(2000);
            command = 0;
            receive_ok = 1;
            break;
            case 'x':              
            receive_ok = 1;        
            SYN_FrameInfo(0, "[v16][t5]前方禁止停车");
            delay(2000);
            command = 0;
            receive_ok = 1;         
            break;
						case 'e':              
            receive_ok = 1;        
            SYN_FrameInfo(0, "[v16][t5]前方限速80");
            delay(2000);
            command = 0;
            receive_ok = 1;         
						break;
            default :
            receive_ok = 1;
            break;
         }        
        //????,??????,????????
        //YS_SYN_Set(SYN_SuspendCom);
    }

}


//??1??
void Uart1_Isr() interrupt 4
{
    if(receive_ok == 1)    //?????????
    {
        command = SBUF;    //????
        while(!RI);
        RI=0;
        receive_ok = 0;    
    }    

}
