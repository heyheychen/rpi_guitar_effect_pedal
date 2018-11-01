#include <stdio.h>
#include <bcm2835.h>
#include <math.h>
#include "lcd1602.h"
#include <pthread.h>

//define input pins
#define PUSH1             RPI_GPIO_P1_08      //GPIO14
#define PUSH2             RPI_V2_GPIO_P1_38   //GPIO20
#define TOGGLE_SWITCH     RPI_V2_GPIO_P1_32   //GPIO12
#define FOOT_SWITCH       RPI_GPIO_P1_10      //GPIO15
#define LED               RPI_V2_GPIO_P1_36   //GPIO16

uint32_t read_timer = 0;
uint32_t input_signal = 0; 
uint32_t output_signal = 0;
uint8_t FOOT_SWITCH_val;
uint8_t TOGGLE_SWITCH_val;
uint8_t PUSH1_val;
uint8_t PUSH2_val;


//systme param
uint32_t lcd_timer = 0;
char pot1_text[3];
char pot2_text[3];
char screen_text[16];
uint32_t pot1 = 0;
int32_t pot2 = 0;
uint32_t pot1_prev = 0;
uint32_t pot2_prev = 0;

//effect parameter
int8_t effect_selection = 0;
uint32_t sine_table[40] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0};
uint32_t sine_counter = 0;
//overdrive
uint32_t drive = 60;
//delay
#define delay_max 800000
#define delay_min 0
uint32_t delay_buffer[delay_max];
uint32_t delay_counter = 0;
uint32_t delay_depth = 50000;

//echo
#define echo_max 800000
#define echo_min 0
uint32_t echo_buffer[delay_max];
uint32_t echo_counter = 0;
uint32_t echo_depth = 50000;

//reverb
#define reverb_max 800000
#define reverb_min 0
uint32_t reverb_buffer1[reverb_max];
uint32_t reverb_buffer2[reverb_max];
uint32_t reverb_buffer3[reverb_max];
uint32_t reverb_counter1 = 0;
uint32_t reverb_counter2 = 0;
uint32_t reverb_counter3 = 0;
uint32_t reverb_depth1 = 10000;
uint32_t reverb_depth2 = 5000;
uint32_t reverb_depth3 = 2500;

//octave
#define octave_max 800000
#define octave_min 0
uint32_t octave_buffer[octave_max];
uint32_t octave_store_counter = 0;
uint32_t octave_play_counter= 0;
uint32_t octave_value = 0;
uint32_t octave_depth = 10000;
uint32_t octave_divider = 0;

//tremolo
uint32_t tremolo_sine_table[]= {0x800,0x80c,0x819,0x826,0x833,0x840,0x84d,0x85a,0x866,0x873,0x880,0x88d,0x89a,0x8a7,0x8b3,0x8c0,
0x8cd,0x8da,0x8e7,0x8f3,0x900,0x90d,0x91a,0x926,0x933,0x940,0x94c,0x959,0x966,0x973,0x97f,0x98c,
0x998,0x9a5,0x9b2,0x9be,0x9cb,0x9d7,0x9e4,0x9f0,0x9fd,0xa09,0xa16,0xa22,0xa2e,0xa3b,0xa47,0xa53,
0xa60,0xa6c,0xa78,0xa84,0xa91,0xa9d,0xaa9,0xab5,0xac1,0xacd,0xad9,0xae5,0xaf1,0xafd,0xb09,0xb15,
0xb21,0xb2d,0xb38,0xb44,0xb50,0xb5c,0xb67,0xb73,0xb7e,0xb8a,0xb96,0xba1,0xbac,0xbb8,0xbc3,0xbcf,
0xbda,0xbe5,0xbf0,0xbfc,0xc07,0xc12,0xc1d,0xc28,0xc33,0xc3e,0xc49,0xc53,0xc5e,0xc69,0xc74,0xc7e,
0xc89,0xc94,0xc9e,0xca9,0xcb3,0xcbd,0xcc8,0xcd2,0xcdc,0xce6,0xcf1,0xcfb,0xd05,0xd0f,0xd19,0xd23,
0xd2c,0xd36,0xd40,0xd4a,0xd53,0xd5d,0xd66,0xd70,0xd79,0xd82,0xd8c,0xd95,0xd9e,0xda7,0xdb0,0xdb9,
0xdc2,0xdcb,0xdd4,0xddd,0xde6,0xdee,0xdf7,0xdff,0xe08,0xe10,0xe19,0xe21,0xe29,0xe31,0xe39,0xe41,
0xe49,0xe51,0xe59,0xe61,0xe69,0xe70,0xe78,0xe7f,0xe87,0xe8e,0xe96,0xe9d,0xea4,0xeab,0xeb2,0xeb9,
0xec0,0xec7,0xece,0xed5,0xedb,0xee2,0xee8,0xeef,0xef5,0xefc,0xf02,0xf08,0xf0e,0xf14,0xf1a,0xf20,
0xf26,0xf2b,0xf31,0xf37,0xf3c,0xf42,0xf47,0xf4c,0xf51,0xf57,0xf5c,0xf61,0xf66,0xf6a,0xf6f,0xf74,
0xf79,0xf7d,0xf82,0xf86,0xf8a,0xf8f,0xf93,0xf97,0xf9b,0xf9f,0xfa3,0xfa6,0xfaa,0xfae,0xfb1,0xfb5,
0xfb8,0xfbb,0xfbf,0xfc2,0xfc5,0xfc8,0xfcb,0xfce,0xfd0,0xfd3,0xfd6,0xfd8,0xfdb,0xfdd,0xfdf,0xfe2,
0xfe4,0xfe6,0xfe8,0xfea,0xfeb,0xfed,0xfef,0xff0,0xff2,0xff3,0xff5,0xff6,0xff7,0xff8,0xff9,0xffa,
0xffb,0xffc,0xffc,0xffd,0xffe,0xffe,0xffe,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xffe,0xffe,
0xffe,0xffd,0xffc,0xffc,0xffb,0xffa,0xff9,0xff8,0xff7,0xff6,0xff5,0xff3,0xff2,0xff0,0xfef,0xfed,
0xfeb,0xfea,0xfe8,0xfe6,0xfe4,0xfe2,0xfdf,0xfdd,0xfdb,0xfd8,0xfd6,0xfd3,0xfd0,0xfce,0xfcb,0xfc8,
0xfc5,0xfc2,0xfbf,0xfbb,0xfb8,0xfb5,0xfb1,0xfae,0xfaa,0xfa6,0xfa3,0xf9f,0xf9b,0xf97,0xf93,0xf8f,
0xf8a,0xf86,0xf82,0xf7d,0xf79,0xf74,0xf6f,0xf6a,0xf66,0xf61,0xf5c,0xf57,0xf51,0xf4c,0xf47,0xf42,
0xf3c,0xf37,0xf31,0xf2b,0xf26,0xf20,0xf1a,0xf14,0xf0e,0xf08,0xf02,0xefc,0xef5,0xeef,0xee8,0xee2,
0xedb,0xed5,0xece,0xec7,0xec0,0xeb9,0xeb2,0xeab,0xea4,0xe9d,0xe96,0xe8e,0xe87,0xe7f,0xe78,0xe70,
0xe69,0xe61,0xe59,0xe51,0xe49,0xe41,0xe39,0xe31,0xe29,0xe21,0xe19,0xe10,0xe08,0xdff,0xdf7,0xdee,
0xde6,0xddd,0xdd4,0xdcb,0xdc2,0xdb9,0xdb0,0xda7,0xd9e,0xd95,0xd8c,0xd82,0xd79,0xd70,0xd66,0xd5d,
0xd53,0xd4a,0xd40,0xd36,0xd2c,0xd23,0xd19,0xd0f,0xd05,0xcfb,0xcf1,0xce6,0xcdc,0xcd2,0xcc8,0xcbd,
0xcb3,0xca9,0xc9e,0xc94,0xc89,0xc7e,0xc74,0xc69,0xc5e,0xc53,0xc49,0xc3e,0xc33,0xc28,0xc1d,0xc12,
0xc07,0xbfc,0xbf0,0xbe5,0xbda,0xbcf,0xbc3,0xbb8,0xbac,0xba1,0xb96,0xb8a,0xb7e,0xb73,0xb67,0xb5c,
0xb50,0xb44,0xb38,0xb2d,0xb21,0xb15,0xb09,0xafd,0xaf1,0xae5,0xad9,0xacd,0xac1,0xab5,0xaa9,0xa9d,
0xa91,0xa84,0xa78,0xa6c,0xa60,0xa53,0xa47,0xa3b,0xa2e,0xa22,0xa16,0xa09,0x9fd,0x9f0,0x9e4,0x9d7,
0x9cb,0x9be,0x9b2,0x9a5,0x998,0x98c,0x97f,0x973,0x966,0x959,0x94c,0x940,0x933,0x926,0x91a,0x90d,
0x900,0x8f3,0x8e7,0x8da,0x8cd,0x8c0,0x8b3,0x8a7,0x89a,0x88d,0x880,0x873,0x866,0x85a,0x84d,0x840,
0x833,0x826,0x819,0x80c,0x800,0x7f3,0x7e6,0x7d9,0x7cc,0x7bf,0x7b2,0x7a5,0x799,0x78c,0x77f,0x772,
0x765,0x758,0x74c,0x73f,0x732,0x725,0x718,0x70c,0x6ff,0x6f2,0x6e5,0x6d9,0x6cc,0x6bf,0x6b3,0x6a6,
0x699,0x68c,0x680,0x673,0x667,0x65a,0x64d,0x641,0x634,0x628,0x61b,0x60f,0x602,0x5f6,0x5e9,0x5dd,
0x5d1,0x5c4,0x5b8,0x5ac,0x59f,0x593,0x587,0x57b,0x56e,0x562,0x556,0x54a,0x53e,0x532,0x526,0x51a,
0x50e,0x502,0x4f6,0x4ea,0x4de,0x4d2,0x4c7,0x4bb,0x4af,0x4a3,0x498,0x48c,0x481,0x475,0x469,0x45e,
0x453,0x447,0x43c,0x430,0x425,0x41a,0x40f,0x403,0x3f8,0x3ed,0x3e2,0x3d7,0x3cc,0x3c1,0x3b6,0x3ac,
0x3a1,0x396,0x38b,0x381,0x376,0x36b,0x361,0x356,0x34c,0x342,0x337,0x32d,0x323,0x319,0x30e,0x304,
0x2fa,0x2f0,0x2e6,0x2dc,0x2d3,0x2c9,0x2bf,0x2b5,0x2ac,0x2a2,0x299,0x28f,0x286,0x27d,0x273,0x26a,
0x261,0x258,0x24f,0x246,0x23d,0x234,0x22b,0x222,0x219,0x211,0x208,0x200,0x1f7,0x1ef,0x1e6,0x1de,
0x1d6,0x1ce,0x1c6,0x1be,0x1b6,0x1ae,0x1a6,0x19e,0x196,0x18f,0x187,0x180,0x178,0x171,0x169,0x162,
0x15b,0x154,0x14d,0x146,0x13f,0x138,0x131,0x12a,0x124,0x11d,0x117,0x110,0x10a,0x103,0xfd,0xf7,
0xf1,0xeb,0xe5,0xdf,0xd9,0xd4,0xce,0xc8,0xc3,0xbd,0xb8,0xb3,0xae,0xa8,0xa3,0x9e,
0x99,0x95,0x90,0x8b,0x86,0x82,0x7d,0x79,0x75,0x70,0x6c,0x68,0x64,0x60,0x5c,0x59,
0x55,0x51,0x4e,0x4a,0x47,0x44,0x40,0x3d,0x3a,0x37,0x34,0x31,0x2f,0x2c,0x29,0x27,
0x24,0x22,0x20,0x1d,0x1b,0x19,0x17,0x15,0x14,0x12,0x10,0xf,0xd,0xc,0xa,0x9,
0x8,0x7,0x6,0x5,0x4,0x3,0x3,0x2,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x1,0x1,0x2,0x3,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xc,
0xd,0xf,0x10,0x12,0x14,0x15,0x17,0x19,0x1b,0x1d,0x20,0x22,0x24,0x27,0x29,0x2c,
0x2f,0x31,0x34,0x37,0x3a,0x3d,0x40,0x44,0x47,0x4a,0x4e,0x51,0x55,0x59,0x5c,0x60,
0x64,0x68,0x6c,0x70,0x75,0x79,0x7d,0x82,0x86,0x8b,0x90,0x95,0x99,0x9e,0xa3,0xa8,
0xae,0xb3,0xb8,0xbd,0xc3,0xc8,0xce,0xd4,0xd9,0xdf,0xe5,0xeb,0xf1,0xf7,0xfd,0x103,
0x10a,0x110,0x117,0x11d,0x124,0x12a,0x131,0x138,0x13f,0x146,0x14d,0x154,0x15b,0x162,0x169,0x171,
0x178,0x180,0x187,0x18f,0x196,0x19e,0x1a6,0x1ae,0x1b6,0x1be,0x1c6,0x1ce,0x1d6,0x1de,0x1e6,0x1ef,
0x1f7,0x200,0x208,0x211,0x219,0x222,0x22b,0x234,0x23d,0x246,0x24f,0x258,0x261,0x26a,0x273,0x27d,
0x286,0x28f,0x299,0x2a2,0x2ac,0x2b5,0x2bf,0x2c9,0x2d3,0x2dc,0x2e6,0x2f0,0x2fa,0x304,0x30e,0x319,
0x323,0x32d,0x337,0x342,0x34c,0x356,0x361,0x36b,0x376,0x381,0x38b,0x396,0x3a1,0x3ac,0x3b6,0x3c1,
0x3cc,0x3d7,0x3e2,0x3ed,0x3f8,0x403,0x40f,0x41a,0x425,0x430,0x43c,0x447,0x453,0x45e,0x469,0x475,
0x481,0x48c,0x498,0x4a3,0x4af,0x4bb,0x4c7,0x4d2,0x4de,0x4ea,0x4f6,0x502,0x50e,0x51a,0x526,0x532,
0x53e,0x54a,0x556,0x562,0x56e,0x57b,0x587,0x593,0x59f,0x5ac,0x5b8,0x5c4,0x5d1,0x5dd,0x5e9,0x5f6,
0x602,0x60f,0x61b,0x628,0x634,0x641,0x64d,0x65a,0x667,0x673,0x680,0x68c,0x699,0x6a6,0x6b3,0x6bf,
0x6cc,0x6d9,0x6e5,0x6f2,0x6ff,0x70c,0x718,0x725,0x732,0x73f,0x74c,0x758,0x765,0x772,0x77f,0x78c,
0x799,0x7a5,0x7b2,0x7bf,0x7cc,0x7d9,0x7e6,0x7f3,0x800};
uint32_t tremolo_divider = 0;
uint32_t tremolo_counter = 0;
uint32_t speed_count = 25;


void* lcd_print()
{
  while(1)
  {
   lcd_timer++;
   if(lcd_timer == 100)
   {
     lcd_timer = 0 ;
//     printf("pot2_prev = %d\n",pot2_prev );
//     printf("pot2 = %d\n",pot2 );
     if(PUSH1_val ==1 || PUSH2_val ==1 || pot1_prev != pot1 || pot2_prev != pot2)
     {
           if (effect_selection ==0)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2);
            sprintf(screen_text,"vol %03s         ",pot1_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("bypass          ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }
           else if(effect_selection ==1)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2);
            sprintf(screen_text,"vol %03s drive%03s",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("overdrive       ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }
           else if(effect_selection ==2)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2);
            sprintf(screen_text,"vol %03s time %03s ",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("delay           ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }
           else if(effect_selection ==3)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2);
            sprintf(screen_text,"vol %03s time %03s ",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("echo           ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
            }
            else if(effect_selection ==4)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2);
            sprintf(screen_text,"vol %03s time %03s ",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("reverb           ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }
            else if(effect_selection ==5)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2/10-5);
            sprintf(screen_text,"vol %03s shift %02s ",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("octave           ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }
            else if(effect_selection ==6)
           {
            sprintf(pot1_text,"%d",pot1);
            sprintf(pot2_text,"%d",pot2/10);
            sprintf(screen_text,"vol %03s speed %02s",pot1_text,pot2_text);
            lcd1602SetCursor(0,0);
            lcd1602WriteString("tremolo           ");
            lcd1602SetCursor(0,1);
            lcd1602WriteString(screen_text);
           }

       }
     }
  }
}

int main(int argc, char **argv)
{
//lcd threading
  pthread_t t;
//  pthread_create(&t,NULL,lcd_print,"LCD");

// Start the BCM2835 Library to access GPIO.
    if (!bcm2835_init())
    {printf("bcm2835_init failed. Are you running as root??\n"); return 1;}

    // Start the SPI BUS.
    if (!bcm2835_spi_begin())
    {printf("bcm2835_spi_begin failed. Are you running as root??\n"); return 1;}

    //i2c lcd function
    int rc;
    rc = lcd1602Init(1,0x27);
    if (rc)
	{
	 printf("lcd init failed");
	return 0;
	}
    lcd1602Clear();
    lcd1602WriteString("initializing    ");
    lcd1602Control(1,0,0);
    lcd1602SetCursor(0,1);
    lcd1602WriteString("                ");

    pthread_create(&t,NULL,lcd_print,"LCD");

    
    //define PWM mode
    bcm2835_gpio_fsel(18,BCM2835_GPIO_FSEL_ALT5 ); //PWM0 signal on GPIO18
    bcm2835_gpio_fsel(13,BCM2835_GPIO_FSEL_ALT0 ); //PWM1 signal on GPIO13
    bcm2835_pwm_set_clock(2);                      // Max clk frequency (19.2MHz/2 = 9.6MHz)
    bcm2835_pwm_set_mode(0,1 , 1);                 //channel 0, markspace mode, PWM enabled. 
    bcm2835_pwm_set_range(0,64);                   //channel 0, 64 is max range (6bits): 9.6MHz/64=150KHz PWM freq.
    bcm2835_pwm_set_mode(1, 1, 1);                 //channel 1, markspace mode, PWM enabled.
    bcm2835_pwm_set_range(1,64);                   //channel 0, 64 is max range (6bits): 9.6MHz/64=150KHz PWM freq.

    //define SPI bus configuration
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);   // 4MHz clock with _64
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    uint8_t mosi[2] = { 0x40, 0x00 }; //12 bit ADC read channel 0.
    uint8_t miso[4] = { 0 };

    uint8_t mosi_pot1[3] = { 0x01, 0x00,0x00 };
    uint8_t miso_pot1[4] = { 0 };

    uint8_t mosi_pot2[3] = { 0x01, 0x20,0x00 }; //12 bit ADC read channel 0.
    uint8_t miso_pot2[4] = { 0 };

    //Define GPIO pins configuration
    bcm2835_gpio_fsel(PUSH1, BCM2835_GPIO_FSEL_INPT);           //PUSH1 button as input
    bcm2835_gpio_fsel(PUSH2, BCM2835_GPIO_FSEL_INPT);           //PUSH2 button as input
    bcm2835_gpio_fsel(TOGGLE_SWITCH, BCM2835_GPIO_FSEL_INPT);   //TOGGLE_SWITCH as input
    bcm2835_gpio_fsel(FOOT_SWITCH, BCM2835_GPIO_FSEL_INPT);     //FOOT_SWITCH as input
    bcm2835_gpio_fsel(LED, BCM2835_GPIO_FSEL_OUTP);             //LED as output
  
    bcm2835_gpio_set_pud(PUSH1, BCM2835_GPIO_PUD_UP);           //PUSH1 pull-up enabled   
    bcm2835_gpio_set_pud(PUSH2, BCM2835_GPIO_PUD_UP);           //PUSH2 pull-up enabled 
    bcm2835_gpio_set_pud(TOGGLE_SWITCH, BCM2835_GPIO_PUD_UP);   //TOGGLE_SWITCH pull-up enabled 
    bcm2835_gpio_set_pud(FOOT_SWITCH, BCM2835_GPIO_PUD_UP);     //FOOT_SWITCH pull-up enabled 
  
    while(1) //Main Loop
    {
    //Read the PUSH buttons every 50000 times (0.25s) to save resources.
    read_timer++;
//    printf("read_timer=%d \n",read_timer );
    if (read_timer==50000)
     {
      //spi for control pot
       bcm2835_spi_chipSelect(BCM2835_SPI_CS1);                      // The default
       bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);      // the default
       bcm2835_spi_transfernb(mosi_pot1, miso_pot1 , 3);
       pot1 =( miso_pot1[2] + ((miso_pot1[1] & 0x03 ) << 8));
       pot1 = pot1 << 1;
       if(pot1 >= 1000)
       {
        pot1 = 1000;
       }
       pot1 = pot1 / 10;

       bcm2835_spi_transfernb(mosi_pot2, miso_pot2 , 3);
       pot2 =( miso_pot2[2] + ((miso_pot2[1] & 0x03 ) << 8));
       pot2 = pot2 << 1;
       if(pot2 >= 1000)
       {
        pot2 = 1000;
       }
       pot2 = pot2 / 10;

       // recover spi to input signal ADC
       bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The defau$
       bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the defau$

       read_timer=0;
       PUSH1_val = bcm2835_gpio_lev(PUSH1);
       PUSH2_val = bcm2835_gpio_lev(PUSH2);
       TOGGLE_SWITCH_val = bcm2835_gpio_lev(TOGGLE_SWITCH);
       FOOT_SWITCH_val = bcm2835_gpio_lev(FOOT_SWITCH);
       bcm2835_gpio_write(LED,FOOT_SWITCH_val); //light the effect when the footswitch is activated.
      // toggle control break
       if (TOGGLE_SWITCH_val ==1)
	 {
	 break;
	 }
      // Push button control effect selection 
       if (PUSH1_val == 1)
         {
           if (PUSH1_val ==1)
	   {
           effect_selection ++;
           }
           if(effect_selection >=7)
            {
		effect_selection = 0;
            }
         }
       if (PUSH2_val == 1)
         {
           if (PUSH2_val ==1)
           {
           effect_selection--;
           }
           if(effect_selection <=-1)
            {
                effect_selection = 6;
            }
         }

      //let lcd print to catch change
       pot1_prev = pot1;
       pot2_prev = pot2;
     }

    //read 10 bits ADC
        bcm2835_spi_transfernb(mosi, miso, 2);
        input_signal =( miso[1] + ((miso[0] & 0x0F ) << 8));
//        printf("input_before=%d \n",input_signal );
        input_signal = input_signal << 2 ;
//      printf("input_after=%d \n",input_signal );
      //effect start
      //bypass
      effect_selection ==1;
      if(effect_selection == 0)
      {
	output_signal = input_signal;
      }
      //overdrive
      else if (effect_selection ==1)
      {
        if (input_signal > (1768 + (drive+(100-pot2))))
	{
	 output_signal = 1768 + (3*drive);
        }
        else if (input_signal < (1760 - (drive+(100-pot2))) )
        {
         output_signal = 1768 - (3*drive);
        }
	else
	{
	 output_signal = input_signal;
	}
      }
      // delay effect
      else if (effect_selection ==2 )
     {
	delay_buffer[delay_counter] = input_signal;
	delay_counter++;
        delay_depth = pot2 * 1500;
	if(pot2==0)
          {
	     delay_depth = 500;
          }
	if(delay_counter >= delay_depth)
	{
	  delay_counter = 0;
	}
	output_signal = (delay_buffer[delay_counter] + input_signal) >> 1;
      }
      // echo effect
      else if (effect_selection ==3 )
     {
        echo_buffer[echo_counter] =( input_signal+echo_buffer[echo_counter])>>1;
        echo_counter++;
        echo_depth = pot2 * 1500;
        if(pot2==0)
          {
             echo_depth = 500;
          }
        if(echo_counter >= echo_depth)
        {
          echo_counter = 0;
        }
        output_signal = (echo_buffer[echo_counter] + input_signal) >> 1;
      }
      //reverb effect
      else if(effect_selection ==4)
      {
        reverb_buffer1[reverb_counter1] = (input_signal + reverb_buffer1[reverb_counter1])>>1;
        reverb_buffer2[reverb_counter2] = (input_signal + reverb_buffer2[reverb_counter2])>>1;
        reverb_buffer3[reverb_counter3] = (input_signal + reverb_buffer3[reverb_counter3])>>1;
        reverb_counter1++;
        reverb_depth1 = pot2 * 300;
        reverb_depth2 = pot2 * 150;
        reverb_depth3 = pot2 * 75;
        if(reverb_counter1 >= reverb_depth1)
	{
	  reverb_counter1 = 0;
	}
        if(reverb_counter2 >= reverb_depth2)
        {
          reverb_counter2 = 0;
        }
        if(reverb_counter3 >= reverb_depth3)
        {
          reverb_counter3 = 0;
        }

	output_signal = (input_signal + reverb_buffer1[reverb_counter1] + reverb_buffer2[reverb_counter2] + reverb_buffer3[reverb_counter3]) >>2;
       }
       //octave effect
       else if(effect_selection == 5)
       {
         octave_buffer[octave_store_counter] = input_signal;
         octave_store_counter++;
         if (octave_store_counter >= octave_depth )
           {
            octave_store_counter = 0;
           }
         output_signal = octave_buffer[octave_play_counter];
         octave_value = pot2/10;
         if(octave_value==10)
           {
            octave_play_counter = octave_play_counter + 6;
           }
         if(octave_value==9)
           { 
            octave_play_counter = octave_play_counter + 5;
           }
         if(octave_value==8)
           { 
            octave_play_counter = octave_play_counter + 4;
           }
         if(octave_value==7)
           { 
            octave_play_counter = octave_play_counter + 3;
           }
         if(octave_value==6)
           { 
            octave_play_counter = octave_play_counter + 2;
           }
         if(octave_value==5)
           { 
            octave_play_counter = octave_play_counter + 1;
           }
         if(octave_value==4)
           {
             octave_divider++;
            if(octave_divider >=2)
	    {
              octave_divider = 0;
              octave_play_counter= octave_play_counter + 1;
            }
           }
         if(octave_value==3)
           {
            octave_divider++;
            if(octave_divider >=3)
            {
              octave_divider = 0;
              octave_play_counter= octave_play_counter + 1;
            }
           }
         if(octave_value==2)
           {
            octave_divider++;
            if(octave_divider >=4)
            {
              octave_divider = 0;
              octave_play_counter= octave_play_counter + 1;
            }
           }
         if(octave_value==1)
           {
            octave_divider++;
            if(octave_divider >=5)
            { 
              octave_divider = 0;
              octave_play_counter= octave_play_counter + 1;
            }
           }
         if(octave_value==0)
           {
             octave_divider++;
             if(octave_divider >=6)
             {
               octave_divider = 0;
              octave_play_counter= octave_play_counter + 1;
              }
            }
         if(octave_play_counter >= octave_depth)
           {
            octave_play_counter =0;
           }
       }
       //tremolo effect
       else if (effect_selection ==6)
       {
         tremolo_divider++;
         speed_count = 26 + ((int)pot2/10 -5) ;
//         printf("speed= %d\n",speed_count);
         if ( tremolo_divider >= speed_count)
            {
             tremolo_divider=0; 
             tremolo_counter++;
             }
         if(tremolo_counter==999)
            {
              tremolo_counter=1;
            }
          output_signal= (int)(((float)input_signal) * ((float)tremolo_sine_table[tremolo_counter]/12000.0));
       }

      output_signal = output_signal *(90+(pot1/10)) /100 ;
//	printf("output = %d\n",output_signal);
//	sine_counter++;
//	if(sine_counter >= 40)
//	{
//	sine_counter = 0;
//	}
    //generate output PWM signal 6 bits
        bcm2835_pwm_set_data(1,output_signal & 0x3F);
        bcm2835_pwm_set_data(0,output_signal >> 6);
    }

    //close all and exit
    bcm2835_spi_end();
    bcm2835_close();
    lcd1602Shutdown();
    return 0;
}
