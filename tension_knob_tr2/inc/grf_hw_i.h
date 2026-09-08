#ifndef __GRF_HW_I_H_
#define __GRF_HW_I_H_

/*This code is automatically written by the IDE, do not add user code*/

#include "grf_typedef.h"

typedef void grf_drv_t;

/***************************grf lcd****************************/
s32 grf_disp_set_bright(u8 bright);
s32 grf_disp_get_scn_size(grf_size_t* disp_size);
/***************************grf touch****************************/
typedef enum
{
    GRF_TOUCH_UP,
    GRF_TOUCH_DOWN,
    GRF_TOUCH_LONG
}grf_touch_state;
typedef struct 
{
    grf_touch_state t_state;
    u16 touch_x;
    u16 touch_y;
}grf_touch_t;
s32 grf_touch_get(grf_touch_t *_touch_t);
/***************************grf uart****************************/
typedef enum{
    UART_SPEED_2400,
    UART_SPEED_4800,
    UART_SPEED_9600,
    UART_SPEED_19200,
    UART_SPEED_38400,
    UART_SPEED_57600,
    UART_SPEED_115200,
    UART_SPEED_230400,
    UART_SPEED_460800,
    UART_SPEED_500000,
    UART_SPEED_576000,
    UART_SPEED_921600
}grf_uart_speed_e;
typedef enum{
    UART_BIT_7,
    UART_BIT_8
}grf_uart_bit_e;
typedef enum{
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN
}grf_uart_parity_e;
typedef enum{
    UART_STOP_1,
    UART_STOP_2
}grf_uart_stop_e;
typedef struct 
{
    s32 win_port;          //电脑端口号
    s32 port;              //IC 端口号,-1为默认端口
    grf_uart_speed_e speed_e;
    grf_uart_bit_e bit_e;
    grf_uart_parity_e parity_e;
    grf_uart_stop_e stop_e;
}grf_uart_cfg_t;
//串口打开
grf_drv_t* grf_drv_uart_open(grf_uart_cfg_t cfg_t);
//串口关闭
s32 grf_drv_uart_close(grf_drv_t *drv_t);
//串口发送
s32 grf_drv_uart_send(grf_drv_t *drv_t,char* databuf, int datalen);
//设置串口接收中断处理函数,max_len-一次接收数据的最大值
typedef void (*uart_rev_bfun)(u8* databuf,u32 datalen);
s32 grf_drv_uart_rev_set_bfun(grf_drv_t *drv_t,uart_rev_bfun rev_bfun,u32 max_len);
//设置接收中断超时时间,到达超时时间后,才会接收到数据,默认5ms,最大1000ms
void grf_drv_uart_rev_set_timeout(grf_drv_t *drv_t,u16 timeout);
/*****************************grf can***************************/
typedef enum{
    CAN_SPEED_1M     = 1000UL * 1000,    /* 1 MBit/sec   */
    CAN_SPEED_800K   = 1000UL * 800,     /* 800 kBit/sec */
    CAN_SPEED_500K   = 1000UL * 500,     /* 500 kBit/sec */
    CAN_SPEED_250K   = 1000UL * 250,     /* 250 kBit/sec */
    CAN_SPEED_125K   = 1000UL * 125,     /* 125 kBit/sec */
    CAN_SPEED_100K   = 1000UL * 100,     /* 100 kBit/sec */
    CAN_SPEED_50K    = 1000UL * 50,      /* 50 kBit/sec  */
    CAN_SPEED_20K    = 1000UL * 20,      /* 20 kBit/sec  */
    CAN_SPEED_10K    = 1000UL * 10       /* 10 kBit/sec  */
}grf_can_speed_e;
typedef enum {
    CAN_FRAME_FORMAT_STD = 0,
    CAN_FRAME_FORMAT_EXT = 1,
}grf_can_frame_format_e;
typedef enum {
    CAN_FRAME_TYPE_DATA      = 0,
    CAN_FRAME_TYPE_REMOTE    = 1,
}grf_can_frame_type_e;
typedef struct 
{
    u32 id;                     /* 报文 ID */
    u32 idmask;                 /* ID 掩码，0 表示对应的位不关心，1 表示对应的位必须匹配 */
    grf_can_frame_format_e ide; /* 只接收 标准帧/扩展帧*/
}grf_can_filter;
typedef struct 
{
    s32 port;                   // IC 端口号,-1为默认端口
    grf_can_speed_e speed_e;    // 波特率
    grf_can_filter *filter;     // 过滤器
}grf_can_cfg_t;
typedef struct {
    u32 id;                      // ID号
    grf_can_frame_type_e ide;    // 标准帧 / 扩展帧
    grf_can_frame_format_e rtr;  // 数据帧 / 远程帧
    u8 dlc;                      // 数据长度
    u8 data[8];
}grf_can_msg_t;

//CAN打开
grf_drv_t* grf_can_open(grf_can_cfg_t cfg_t);
//CAN关闭
s32 grf_can_close(grf_drv_t *drv_t);
//CAN发送
s32 grf_can_send(grf_drv_t *drv_t,grf_can_msg_t *msg);
//设置CAN接收中断处理函数
typedef void (*can_rev_bfun)(grf_can_msg_t* msg_t);
s32 grf_can_rev_set_bfun(grf_drv_t *drv_t,can_rev_bfun rev_bfun);

/****************************grf wdt*****************************/
//打开看门狗
s32 grf_wdt_open(void);
//关闭看门狗
s32 grf_wdt_close(void);
//设置看门狗超时时间,单位秒
s32 grf_wdt_set_timeout(u32 time);
//喂狗,刷新看门狗时间
s32 grf_wdt_keepalive(void);
/***************************grf gpio****************************/
typedef enum {
    GRF_GPIO_GROUP_A=0,
    GRF_GPIO_GROUP_B,
    GRF_GPIO_GROUP_C,
    GRF_GPIO_GROUP_D,
    GRF_GPIO_GROUP_E,
    GRF_GPIO_GROUP_F,
}grf_gpio_pin_e;
typedef enum {
    GRF_GPIO_INPUT=0,
    GRF_GPIO_OUTPUT,
    GRF_GPIO_INPUT_PULLUP,
    GRF_GPIO_INPUT_PULLDOWN,
    GRF_GPIO_OUTPUT_OD,
}grf_gpio_mode_e;
typedef enum {
    GRF_GPIO_IRQ_RISING,
    GRF_GPIO_IRQ_FALLING,
    GRF_GPIO_IRQ_RISING_FALLING
}grf_gpio_irq_mode_e;
typedef void grf_gpio_irq_t;
typedef void(*gpio_irq_task)(grf_gpio_irq_t *irq_t);
typedef struct
{
    u8 gpio_group;
    u8 gpio_pin;
    grf_gpio_mode_e gpio_mode;
    grf_gpio_irq_mode_e irq_mode;
}grf_gpio_para_t;
s32 grf_gpio_init(grf_gpio_para_t* para_t);
s32 grf_gpio_setPin(u8 gpio_group,u8 gpio_pin,u8 value);
s32 grf_gpio_readInputPin(u8 gpio_group,u8 gpio_pin);
s32 grf_gpio_readOutputPin(u8 gpio_group,u8 gpio_pin);
//TR660 linux not supported
s32 grf_gpio_set_irq(grf_gpio_para_t* para_t,gpio_irq_task irqfun);

//only TR660 linux support
typedef struct{
    u8 irq_group;
    u8* irq_pins;       
    u8 gpio_pin_num;
    grf_gpio_irq_mode_e mode_e;
    gpio_irq_task irq_task;
}grf_gpio_irq_para_t;
s32 grf_gpio_group_set_irq(grf_gpio_irq_para_t irq_para_t);
s32 grf_gpio_group_get_irq_pin(grf_gpio_irq_t *irq_t);
s32 grf_gpio_group_get_irq_mode(grf_gpio_irq_t *irq_t);
/***************************grf rtc****************************/
typedef  struct 
{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
}grf_rtc_time_t;
s32 grf_rtc_get_time(grf_rtc_time_t* rtc_t);
s32 grf_rtc_set_time(grf_rtc_time_t* rtc_t);
/****************************SPI*****************************/
//需固件支持
#define SPI_CPHA		     0x01
#define SPI_CPOL		     0x02
#define SPI_WORK_MODE0		(0|0)
#define SPI_WORK_MODE1		(0|SPI_CPHA)
#define SPI_WORK_MODE2		(SPI_CPOL|0)
#define SPI_WORK_MODE3		(SPI_CPOL|SPI_CPHA)

typedef struct
{
    u32     port;         //端口号
	u32		work_clk;     //单位：HZ
    u32     work_mode;    //SPI工作模式 
	u8		nbits;        //位数
    u16		delay;        //发送时间间隔     
}grf_spi_para_t;

grf_drv_t* grf_spi_open(grf_spi_para_t *para_t);
s32 grf_spi_transfer(grf_drv_t* drv_t,u8 *tx,u32 tx_len,u8 *rx,u32 rx_len);
s32 grf_spi_close(grf_drv_t *drv_t);
/****************************IIC*****************************/
//需固件支持
typedef struct
{
    u32     port; //端口号
    u8      slave_addr;  //设备地址
    u8      reg_addr;    //寄存器地址
    u8      reg_addr_width;  //寄存器字节个数
}grf_iic_para_t;

grf_drv_t* grf_iic_open(grf_iic_para_t* para_t);
s32 grf_iic_read(grf_drv_t* drv_t,u16 addr,u8 *buff,u16 len);
s32 grf_iic_write(grf_drv_t* drv_t,u16 addr,u8 *buff,u16 len);
s32 grf_iic_close(grf_drv_t *drv_t);
/****************************PWM*****************************/
typedef struct
{
    u32     channel;        //pwm通道
    u32     period_time;    //周期 单位:ns
    u32     pulse_time;      //占宽百分比 单位:ns
}grf_pwm_para_t;

grf_drv_t* grf_pwm_open(grf_pwm_para_t* para_t);
s32 grf_pwm_start(grf_drv_t *drv_t);
s32 grf_pwm_set(grf_drv_t *drv_t,u32 period_time,u32 pulse_time);
s32 grf_pwm_stop(grf_drv_t *drv_t);
s32 grf_pwm_close(grf_drv_t *drv_t);
/****************************ADC*****************************/
grf_drv_t* grf_adc_open(u32 chan);
s32 grf_adc_read(grf_drv_t *drv_t);
s32 grf_adc_to_voltage(u32 adc_value);
s32 grf_adc_close(grf_drv_t *drv_t);

/***************************Tone****************************/
void grf_hmi_tone_on(u16 time);

/***************************flash******************************/\
grf_drv_t* grf_flash_part_open(u8* fs_str);
s32 grf_flash_part_get_size(grf_drv_t* part_drv);
s32 grf_flash_part_earse(grf_drv_t* part_drv,u32 addr,u32 len);
s32 grf_flash_part_write(grf_drv_t* part_drv,u32 addr, u8 *data, u32 size);
s32 grf_flash_part_read(grf_drv_t* part_drv,u32 addr, u8 *data, u32 size);
s32 grf_flash_part_close(grf_drv_t* part_drv);

#endif
