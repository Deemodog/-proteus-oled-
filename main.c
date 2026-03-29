#include <reg52.h>
#include <intrins.h>

// 类型重定义
typedef unsigned char uchar;
typedef unsigned int uint;

// 管脚定义
sbit SCL = P1^3;
sbit SDA = P1^4;

// --- 全局变量 ---
uchar sec = 0, min = 30, hour = 12; // 默认时间
uint  year = 2026;                  // 默认日期
uchar month = 3, day = 29, week = 7; 
uchar timer_count = 0;
bit time_update_flag = 0;
bit date_update_flag = 1; // 初始为1，确保开机显示日期

// 每月天数表（平年）
uchar code Days_In_Month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// --- 标准 8x16 提纯字库 (含 0-9, :, -, W) ---
const uchar code F8X16[] = {
    0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00, // 0
    0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // 1
    0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00, // 2
    0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00, // 3
    0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00, // 4
    0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00, // 5
    0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00, // 6
    0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00, // 7
    0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00, // 8
    0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00, // 9
    0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00, // 10 [:]
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00, // 11 [-]
    0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x3F,0x00,0x03,0x0C,0x03,0x00,0x3F,0x00  // 12 [W]
};


void I2C_Start() {
    SCL = 1; SDA = 1; _nop_();
    SDA = 0; _nop_();
    SCL = 0;
}

void I2C_Stop() {
    SCL = 0; SDA = 0; _nop_();
    SCL = 1; SDA = 1; _nop_();
}

void I2C_WriteByte(uchar dat) {
    uchar i;
    for(i=0; i<8; i++) {
        if(dat & 0x80) SDA = 1; else SDA = 0;
        SCL = 1; _nop_(); SCL = 0;
        dat <<= 1;
    }
    SDA = 1; SCL = 1; _nop_(); SCL = 0; 
}


void OLED_WriteCmd(uchar cmd) {
    I2C_Start();
    I2C_WriteByte(0x78);
    I2C_WriteByte(0x00);
    I2C_WriteByte(cmd);
    I2C_Stop();
}

void OLED_WriteData(uchar dat) {
    I2C_Start();
    I2C_WriteByte(0x78);
    I2C_WriteByte(0x40);
    I2C_WriteByte(dat);
    I2C_Stop();
}

void OLED_SetPos(uchar x, uchar y) {
    OLED_WriteCmd(0xb0 + y);
    OLED_WriteCmd(((x & 0xf0) >> 4) | 0x10);
    OLED_WriteCmd((x & 0x0f));
}

void OLED_Clear() {
    uchar i, n;
    for(i=0; i<8; i++) {
        OLED_WriteCmd(0xb0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for(n=0; n<128; n++) OLED_WriteData(0);
    }
}

void OLED_Init() {
    OLED_WriteCmd(0xAE); OLED_WriteCmd(0x20); OLED_WriteCmd(0x10);
    OLED_WriteCmd(0xB0); OLED_WriteCmd(0xC8); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10); OLED_WriteCmd(0x40); OLED_WriteCmd(0x81);
    OLED_WriteCmd(0xFF); OLED_WriteCmd(0xA1); OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F); OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00); OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0xF0); OLED_WriteCmd(0xD9); OLED_WriteCmd(0x22);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12); OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);
    OLED_Clear();
}


void OLED_ShowChar(uchar x, uchar y, uchar num) {
    uchar i;
    OLED_SetPos(x, y);
    for(i=0; i<8; i++) OLED_WriteData(F8X16[num*16 + i]); 
    OLED_SetPos(x, y+1);
    for(i=0; i<8; i++) OLED_WriteData(F8X16[num*16 + i + 8]);
}

bit Is_Leap_Year(uint y) {
    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) return 1;
    return 0;
}

void Display_Date() {
    OLED_ShowChar(24, 0, year/1000);
    OLED_ShowChar(32, 0, (year%1000)/100);
    OLED_ShowChar(40, 0, (year%100)/10);
    OLED_ShowChar(48, 0, year%10);
    OLED_ShowChar(56, 0, 11); // [-]
    OLED_ShowChar(64, 0, month/10);
    OLED_ShowChar(72, 0, month%10);
    OLED_ShowChar(80, 0, 11); // [-]
    OLED_ShowChar(88, 0, day/10);
    OLED_ShowChar(96, 0, day%10);
    // 星期 W:X
    OLED_ShowChar(48, 4, 12); // W
    OLED_ShowChar(56, 4, 10); // :
    OLED_ShowChar(64, 4, week);
}

// --- 定时器中断 ---
void Timer0_Init() {
    TMOD |= 0x01; TH0 = 0x3C; TL0 = 0xB0;
    EA = 1; ET0 = 1; TR0 = 1;
}

void Timer0_ISR() interrupt 1 {
    uchar max_days;
    TH0 = 0x3C; TL0 = 0xB0;
    timer_count++;
    if(timer_count >= 20) {
        timer_count = 0;
        sec++;
        if(sec >= 60) {
            sec = 0; min++;
            if(min >= 60) {
                min = 0; hour++;
                if(hour >= 24) {
                    hour = 0; day++; week++;
                    if(week > 7) week = 1;
                    
                    max_days = Days_In_Month[month];
                    if(month == 2 && Is_Leap_Year(year)) max_days = 29;
                    
                    if(day > max_days) {
                        day = 1; month++;
                        if(month > 12) {
                            month = 1; year++;
                        }
                    }
                    date_update_flag = 1;
                }
            }
        }
        time_update_flag = 1;
    }
}

// --- 主程序 ---
void main() {
    OLED_Init();
    Timer0_Init();
    
    while(1) {
        if(date_update_flag) {
            date_update_flag = 0;
            Display_Date(); // 跨天刷新日期和星期
        }
        if(time_update_flag) {
            time_update_flag = 0;
            // 刷新中间的时间
            OLED_ShowChar(32, 2, hour/10);
            OLED_ShowChar(40, 2, hour%10);
            OLED_ShowChar(48, 2, 10);
            OLED_ShowChar(56, 2, min/10);
            OLED_ShowChar(64, 2, min%10);
            OLED_ShowChar(72, 2, 10);
            OLED_ShowChar(80, 2, sec/10);
            OLED_ShowChar(88, 2, sec%10);
        }
    }
}