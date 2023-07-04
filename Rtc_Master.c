// PIC16F877A Configuration Bit Settings

// 'C' source line config statements
// CONFIG
#pragma config FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF     // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 20000000
#define RS RC2
#define EN RC1
void init(void); //initialisation function
void i2c_init(const unsigned long); //i2c initialisation 
void i2c_write(unsigned char); //i2c write function
int i2c_read(int); //i2c read function
void i2c_start(); //i2c start
void i2c_wait(); //i2c wait
void i2c_stop(); //i2c stop
void lcd_command(unsigned char); //lcd command
void lcd_data(unsigned char); //lcd data
int bcd_2_dec(int); //bcd_2_dec conversion
int dec_2_bcd(int); //dec_2_bcd conversion
void settime(void); // setting initial time
void update(void); //
char msg1[5]={"TIME:"};
char msg2[5]={"DATE:"};
int i,j,k,l;
int sec=55;
int min=59;
int hour=23;
int date=31;
int month=12;
int year =22;
char sec1,sec2,min1,min2,hour1,hour2,date1,date2,month1,month2,year1,year2;
void main()
{
    init();
    i2c_init(100); //I2C Speed : 100 kbps
    settime();
    while(1)
    {
        update();
        sec1=sec/10;
        sec2=sec%10;
        min1=min/10;
        min2=min%10;
        hour1=hour/10;
        hour2=hour%10;
        date1=date/10;
        date2=date%10;
        month1=month/10;
        month2=month%10;
        year1=year/10;
        year2=year%10;
        
        lcd_command(0x80);
        for(i=0;i<5;i++) lcd_data(msg1[i]);
        lcd_data(hour1+'0');
        lcd_data(hour2+'0');
        lcd_data(0x2D);
        lcd_data(min1+'0');
        lcd_data(min2+'0');
        lcd_data(0x2D);
        lcd_data(sec1+'0');
        lcd_data(sec2+'0');
        lcd_command(0xC0);
        for(i=0;i<5;i++) lcd_data(msg2[i]);
        lcd_data(date1+'0');
        lcd_data(date2+'0');
        lcd_data(0x2D);
        lcd_data(month1+'0');
        lcd_data(month2+'0');
        lcd_data(0x2D);
        lcd_data(year1+'0');
        lcd_data(year2+'0');
    __delay_ms(500);
    }
    
}

void init(void)
{
    TRISD=0x00;//0000 0000
    TRISC=0x18;//0001 1000 RC3 and RC4 set as input
    PORTD=0x00;//0000 0000
    //lcd initialisation
    lcd_command(0x38);
      __delay_ms(5);
      lcd_command(0x38);
      __delay_ms(5);
      lcd_command(0x38);
      __delay_ms(5);
      lcd_command(0x38);
      __delay_ms(5);
      lcd_command(0x0C);
      __delay_ms(5);
      lcd_command(0x06);
      __delay_ms(5);
      lcd_command(0x01);
      __delay_ms(5);
}
void lcd_command(unsigned char i)
{
    RS=0;
    PORTD=i;
    EN=1;
    //__delay_ms)(1);
    EN=0;
    __delay_ms(10);
}
void lcd_data(unsigned char i)
{
    RS=1;
    PORTD=i;
    EN=1;
    //__delay_ms)(1);
    EN=0;
    __delay_ms(10);
}
void i2c_init(const unsigned long feq_k)
{
    //SSPCON5 = 1 : Enables the serial port and configures the SDA and SCL pins as the serial port pins
    //SSPCON(3:0) = 1000 : I2C Master mode, clock = FOSC/(4 * (SSPADD + 1))
    SSPCON=0x28; //0010 1000 
    //Initializing SSPSTAT to 0000 0000
    SSPSTAT=0x00;
    //Initializing IDLE state for ack, start, stop signals
    SSPCON2=0x00;
    //I2C Master mode is set
   SSPADD = (_XTAL_FREQ/(4*feq_k*100))-1; //Value to be loaded in SSPADD reg for 100 kbps I2C speed
}

void i2c_wait() 
{
    // 0x1F = 0001 1111, 0x40 = 0000 0100
    //Check if (ACKEN, RCEN, PEN, RSEN, SEN) bits are idle or transmit not in progress
    while(SSPCON2 & 0x1F || SSPSTAT & 0x04);
}

void i2c_start() //I2C start bit
{
   i2c_wait(); //Wait until the bus becomes idle
    SEN=1; //Enable start bit
}
void i2c_stop()
{
    i2c_wait(); //Wait until the bus becomes idle
    PEN=1; //Enable parity bit
}
void i2c_write(unsigned char temp)
{
    i2c_wait(); //Wait until the bus becomes idle
    SSPBUF=temp; //load value into SSPBUF register for transmission
}
int i2c_read(int ack)
{
    int value;
    i2c_wait();
    RCEN=1;
    i2c_wait();
    value=SSPBUF;
    i2c_wait();
    ACKDT=(ack)?0:1;
    ACKEN=1;
    return value;
}
int dec_2_bcd(int temp)
{ 
return ((temp/10)<<4)+(temp%10);
}
int bcd_2_dec(int temp)
{ 
return ((temp>>4)*10)+(temp&0x0F);
}

void settime(void ) //To set initial time
{
    i2c_start(); //start bit
    i2c_write(0xD0); //write device address_DS3231
    i2c_write(0); //0 to indicate write operation
    i2c_write(dec_2_bcd(sec)); //write sec : Address 00h
    i2c_write(dec_2_bcd(min)); //write min : Address 01h
    i2c_write(dec_2_bcd(hour)); //write hour : Address 02h
    i2c_write(1); //dummy values given (since Monday, Tuesday etc...info not used) : Address 03h
    i2c_write(dec_2_bcd(date)); //write date : Address 04h
    i2c_write(dec_2_bcd(month)); //write month : Address 05h
    i2c_write(dec_2_bcd(year)); //write year : Address 06h
    i2c_stop(); //Stop bit
}

void update(void)
{
    i2c_start();
    i2c_write(0xD0);
    i2c_write(0);
    i2c_stop();
    i2c_start();
    i2c_write(0xD1);
    sec=(bcd_2_dec(i2c_read(1)));
    min=(bcd_2_dec(i2c_read(1)));
    hour=(bcd_2_dec(i2c_read(1)));
    (i2c_read(1));
    date=(bcd_2_dec(i2c_read(1)));
    month=(bcd_2_dec(i2c_read(1)));
    year=(bcd_2_dec(i2c_read(1)));
    i2c_stop();
    i2c_start();
    i2c_write(0xD1);
    i2c_read(1);
    i2c_stop();
}