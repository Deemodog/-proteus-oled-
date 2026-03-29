文件放在仓库，用者自取
通过c52控制OLED12864I2C显示日期时间，原理图如下所示<img width="1895" height="1113" alt="屏幕截图 2026-03-29 165602" src="https://github.com/user-attachments/assets/b5759faf-3123-4857-92d3-bf07ebab3b29" />
这是显示效果<img width="1834" height="1163" alt="image" src="https://github.com/user-attachments/assets/6818ed8c-8531-4512-8d39-d0a775c59735" />
<img width="1311" height="945" alt="image" src="https://github.com/user-attachments/assets/26b108c0-2bdc-4dfc-928c-6b0466675308" />

单片机与OLED12864I2C之间是通过I2C协议进行通信的，I2C（Inter-Integrated Circuit）是一种同步、半双工、双线制的通信总线，下面我们来简单讲讲它的原理以便进行二次开发。

首先介绍1.SCL (Serial Clock - 时钟线)：由主机（你的 51 单片机）控制，就像乐队的指挥，决定了通信的节奏。
2.SDA (Serial Data - 数据线)：用来传输具体的数据（0 或 1），主机和从机都可以控制它。

在发送数据时，当 SCL 为高电平（1）时，SDA 必须保持稳定（无论高低），因为此时从机正在读取数据；只有当 SCL 为低电平（0）时，SDA 才允许改变状态。(唯一的例外是起始信号和停止信号)

代码分为4部分：
```c
###
//1. 起始信号 (Start)
void I2C_Start() {
    SCL = 1; SDA = 1; // 1. 先让两条线都保持高电平（总线空闲状态）
    _nop_();          // 2. 稍微延时，等待电压稳定
    SDA = 0;          // 3. 【关键动作】SCL 在高电平时，把 SDA 拉低，产生起始信号
    _nop_();
    SCL = 0;          // 4. 起始信号发完，立刻把 SCL 拉低，钳住总线准备发数据
}
//当 SCL 为高电平时，SDA 发生从高到低的跳变。

//2.发送一个字节
void I2C_WriteByte(unsigned char dat) {
    unsigned char i;
    for(i=0; i<8; i++) {
        // 1. 准备数据：此时 SCL=0，允许改变 SDA 状态
        if(dat & 0x80) SDA = 1; 
        else SDA = 0;           
        
        // 2. 发送数据：拉高 SCL，从机会在此期间读取 SDA 的电平
        SCL = 1; 
        _nop_();                
        
        // 3. 结束当前 bit：拉低 SCL，准备下一位
        SCL = 0;
        dat <<= 1; // 左移一位
    }
}

//3.应答信号并释放总线
//发送完 8 bit 后，主机必须给出第 9 个时钟脉冲。在此期间，主机必须释放 SDA 数据线
若从机成功接收，会主动将 SDA 拉低作为应答（ACK）。
    SDA = 1;  // 【极其重要】主机主动拉高 SDA，交出控制权！
    SCL = 1;  // 产生第 9 个时钟脉冲
    _nop_();  // 此时从机会将 SDA 拉低以示确认
    SCL = 0;  // 结束脉冲

//4.停止信号
void I2C_Stop() {
    SCL = 0; SDA = 0; // 1. 确保 SCL 和 SDA 均在低电平
    _nop_();
    SCL = 1;          // 2. 拉高 SCL
    SDA = 1;          // 3. 【关键】SCL 为高时抬高 SDA，产生停止信号
    _nop_();
}
```
当然我们还剩余一个数码管没有使用，具体怎么用，就看个人发挥了
