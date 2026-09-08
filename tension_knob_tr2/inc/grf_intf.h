#ifndef _GRF_INTF_H__
#define _GRF_INTF_H__

/*This code is automatically written by the IDE, do not add user code*/

#include "grf_typedef.h"

/***************************base api****************************/
u8* grf_ver_get_str(void);  //SDK版本(旧版)
//获取SDK版本号
u8* grf_sdk_get_version(void);
//获取固件(firmware)版本号
u8 *grf_fw_get_version(void);
//调试打印
int grf_printf(const char *format,...);
//延时函数,使用可能会减缓刷新,请谨慎使用
void grf_delay(int time_ms);
//获取系统时间片
u32 grf_sys_time_ms(void);
//剩余内存大小
int grf_mem_get_free(void);
//已使用内存（只有windows支持）
u32 grf_mem_get_used(void);
//系统重启
int grf_reboot(void);
/***************************memory****************************/
void *grf_malloc(unsigned int size);
void *grf_realloc(void * data_p, unsigned int new_size);
void grf_free(void* p);
/***************************grf color****************************/
#define GRF_COLOR_WHITE 	GRF_COLOR_GET(0xFF, 0xFF, 0xFF)
#define GRF_COLOR_SILVER 	GRF_COLOR_GET(0xC0, 0xC0, 0xC0)
#define GRF_COLOR_GRAY 		GRF_COLOR_GET(0x80, 0x80, 0x80)
#define GRF_COLOR_BLACK 	GRF_COLOR_GET(0x00, 0x00, 0x00)
#define GRF_COLOR_RED 		GRF_COLOR_GET(0xFF, 0x00, 0x00)
#define GRF_COLOR_MAROON 	GRF_COLOR_GET(0x80, 0x00, 0x00)
#define GRF_COLOR_YELLOW 	GRF_COLOR_GET(0xFF, 0xFF, 0x00)
#define GRF_COLOR_OLIVE 	GRF_COLOR_GET(0x80, 0x80, 0x00)
#define GRF_COLOR_LIME 		GRF_COLOR_GET(0x00, 0xFF, 0x00)
#define GRF_COLOR_GREEN 	GRF_COLOR_GET(0x00, 0x80, 0x00)
#define GRF_COLOR_CYAN 		GRF_COLOR_GET(0x00, 0xFF, 0xFF)
#define GRF_COLOR_TEAL 		GRF_COLOR_GET(0x00, 0x80, 0x80)
#define GRF_COLOR_BLUE 		GRF_COLOR_GET(0x00, 0x00, 0xFF)
#define GRF_COLOR_NAVY 		GRF_COLOR_GET(0x00, 0x00, 0x80)
#define GRF_COLOR_MAGENTA 	GRF_COLOR_GET(0xFF, 0x00, 0xFF)
#define GRF_COLOR_PURPLE 	GRF_COLOR_GET(0x80, 0x00, 0x80)
#define GRF_COLOR_ORANGE 	GRF_COLOR_GET(0xFF, 0xA5, 0x00)
/***************************grf task****************************/
typedef void *grf_task_t;
//任务回调函数类型
typedef void (*grf_task_cb_t)(grf_task_t *task_t);
//创建任务
grf_task_t *grf_task_create(grf_task_cb_t task_xcb, u32 period, void* user_data);
//删除任务
void grf_task_del(grf_task_t *task_t);
//获取该任务的用户参数
void* grf_task_get_user_data(grf_task_t *task_t);
//更改任务回调函数
void grf_task_set_cb(grf_task_t *task_t, grf_task_cb_t task_cb);
//设置任务回调周期
void grf_task_set_period(grf_task_t *task_t, u32 period);
//任务准备就绪,不等待时间,立即执行
void grf_task_ready(grf_task_t * task_t);
//使任务回调函数只运行一次
void grf_task_once(grf_task_t *task_t);
//复位任务
void grf_task_reset(grf_task_t * task_t);
//设置任务暂停
void grf_task_set_cb_pause(grf_task_t *task_t);
//任务恢复
void grf_task_set_cb_resume(grf_task_t *task_t);
/***************************grf resource****************************/
//根据ID获取图片名
u8* grf_j2s_res_img_get(u32 dirID,u32 fileID);

/***************************grf language****************************/
//设置当前语言
s32 grf_lang_set(u8* language);
//根据变量名获取当前语言字符串
char *grf_lang_get_cur_str(u8 *var);
//根据变量名设置当前语言字符串
s32 grf_lang_set_cur_str(char *var,char *str);
//将当前变量保存到flash中(如果发生改变)
//注意:flash有写入寿命,尽可能减小写入次数
s32 grf_lang_save_cur_to_flash(void);

/***************************file system*****************************/
typedef enum {
    GRF_FS_MODE_WR = 0x01,
    GRF_FS_MODE_RD = 0x02,
} grf_fs_mode_e;
typedef enum {
    GRF_FS_SEEK_SET = 0x00,
    GRF_FS_SEEK_CUR = 0x01,
    GRF_FS_SEEK_END = 0x02,
} grf_fs_whence_e;
typedef void grf_fs_file_t;
//drive letter board - D:/ | TF-CARD - T:/ | U-DISK - U:/ |
//             EXTRA - E:/ |
//path - eg "D:/data.bin" "T:/grf_app/data.bin"
grf_fs_file_t *grf_fs_open(char *path,grf_fs_mode_e mode);
s32 grf_fs_close(grf_fs_file_t *file_t);
s32 grf_fs_read(grf_fs_file_t *file_t,void *buf, u32 count);
s32 grf_fs_write(grf_fs_file_t *file_t,void *buf, u32 count);
s32 grf_fs_seek(grf_fs_file_t *file_t, u32 pos, grf_fs_whence_e whence);
s32 grf_fs_tell(grf_fs_file_t *file_t);

typedef void grf_fs_dir_t;
grf_fs_dir_t *grf_fs_dir_open(char *path);
s32 grf_fs_dir_read(grf_fs_dir_t *dir_t, char *fn);
s32 grf_fs_dir_close(grf_fs_dir_t *dir_t);

// Standard file system access interface eg: fopen/fwrite/fread/fclose
u8 *grf_dir_get_ddisk();
u8 *grf_dir_get_tfdisk();
u8 *grf_dir_get_udisk();

/********************************func stdby************************************** */
//设置待机
typedef struct 
{
    u16 stdby_view;         //屏幕保护页面,0-无屏保页面
    u16 stdby_time;         //待机时间,0-不切换
    u8 stdby_bright;        //待机亮度,swpage_time=0时,该参数无效
    u16 off_time;           //关闭屏幕时间,0-不关闭屏幕
    u8 off_bright;          //关闭屏幕亮度,当offlight_time=0时,该参数无效
    u8 on_bright;           //激活亮度,点击亮屏,或者串口命令亮屏,0xff-待机前亮度
    u16 on_view;            //激活页面,0-使用进入待机前界面
}grf_func_standby_t;//待机保护

typedef enum {
    FUNC_STDBY_STATUE_ACTIVE,    //激活状态
    FUNC_STDBY_STATUE_STDBY,     //待机状态
    FUNC_STDBY_STATUE_SLEEP      //关闭屏幕状态
}func_stdby_statue_e;

s32 grf_func_stdby_set_param(grf_func_standby_t *standby_t);
s32 grf_func_stdby_get_statue(void);
s32 grf_func_stdby_set_statue(func_stdby_statue_e statue_e);

/********************************Modbus RTU***************************************/
typedef enum {
    MODBUS_CMD_READ_COIL = 0x01,            //读线圈
    MODBUS_CMD_READ_DISCRETE_INPUT = 0x02,  //读离散输入,只读
    MODBUS_CMD_READ_HOLD_REG = 0x03,        //读保持寄存器
    MODBUS_CMD_READ_INPUT_REG = 0x04,       //读输入寄存器,只读
    MODBUS_CMD_WRITE_COIL = 0x05,           //写线圈
    MODBUS_CMD_WRITE_HOLD_REG = 0x06,       //写保持寄存器
    MODBUS_CMD_WRITE_MULT_COIL = 0x0F,      //写多个线圈
    MODBUS_CMD_WRITE_MULT_HOLD_REG = 0x10,  //写多个保持寄存器
}grf_modbus_cmd_e;

typedef enum {
    GRF_MODBUS_NO_ERR,
    GRF_MODBUS_ILL_FUN,
    GRF_MODBUS_ILL_ADDR,
    GRF_MODBUS_ILL_DATA,
    GRF_MODBUS_SLAVE_OR_SERVER_FAIL,
    GRF_MODBUS_ACKNOWLEDGE,
    GRF_MODBUS_SLAVE_OR_SERVER_BUSY,
    GRF_MODBUS_NEGATIVE_ACKNOWLEDGE,
    GRF_MODBUS_MEMORY_PARITY,
    GRF_MODBUS_NOT_DEFINED,
    GRF_MODBUS_GATEWAY_PATH,
    GRF_MODBUS_GATEWAY_TARGET,
    GRF_MODBUS_OTHER_ERR = 0xFF,
    GRF_MODBUS_TIMEOUT = 0xFF + 138,
}grf_modbus_err_e;

typedef void (*grf_modbus_callback)(u8 slave,grf_modbus_cmd_e cmd,u16 addr,u16 num,u8 *data,s32 errid);
typedef void *grf_modbus_t;

grf_modbus_t *grf_modbus_rtu_create(grf_uart_cfg_t *uart_cfg,u32 rev_buf_size);
void grf_modbus_rtu_delete(grf_modbus_t *modbus);

//主机接口
typedef struct {
    u16 cmd_fifo_size;
    u32 response_timeout;       //响应超时时间,单位ms
    u8 retrans_freq;            //重传次数
    u8 retrans_delay;           //重传间隔
    grf_modbus_callback callback;
}grf_modbus_master_info_t;
s32 grf_modbus_master_init(grf_modbus_t *modbus,grf_modbus_master_info_t *master_info);
void grf_modbus_master_deinit(grf_modbus_t *modbus);
s32 grf_modbus_read_coil(grf_modbus_t *modbus,u8 slave, u16 addr);
s32 grf_modbus_read_discrete_input(grf_modbus_t *modbus,u8 slave, u16 addr, u16 num);
s32 grf_modbus_read_hold_reg(grf_modbus_t *modbus,u8 slave, u16 addr);
s32 grf_modbus_read_input_reg(grf_modbus_t *modbus,u8 slave, u16 addr, u16 num);
s32 grf_modbus_write_coil(grf_modbus_t *modbus,u8 slave, u16 addr, grf_bool val);
s32 grf_modbus_write_mult_coil(grf_modbus_t *modbus,u8 slave, u16 addr, u8 *valbuf, u16 num);
s32 grf_modbus_write_hold_reg(grf_modbus_t *modbus,u8 slave, u16 addr, u16 val);
s32 grf_modbus_write_mult_hold_reg(grf_modbus_t *modbus,u8 slave, u16 addr, u16 *valbuf, u16 num);

//从机接口
typedef struct {
    u8 slave_id;
    u16 cmd_fifo_size;

    u32 coil_nb;
    u32 coil_addr;
    u32 discrete_nb;
    u32 discrete_addr;
    u32 input_reg_addr;
    u32 input_reg_nb;
    u32 hold_reg_nb;
    u32 hold_reg_addr;

    grf_modbus_callback callback;
}grf_modbus_slave_info_t;
s32 grf_modbus_slave_init(grf_modbus_t *modbus,grf_modbus_slave_info_t *slave_info);
void grf_modbus_slave_deinit(grf_modbus_t *modbus);
s32 grf_modbus_set_coils(grf_modbus_t *modbus, u16 addr,u16 num, u8 *valbuf);
s32 grf_modbus_get_coils(grf_modbus_t *modbus, u16 addr,u16 num,u8 *valbuf);
s32 grf_modbus_set_discrete_input(grf_modbus_t *modbus, u16 addr,u16 num, u8 *valbuf);
s32 grf_modbus_get_discrete_input(grf_modbus_t *modbus, u16 addr,u16 num,u8 *valbuf);
s32 grf_modbus_set_hold_reg(grf_modbus_t *modbus, u16 addr,u16 num, u16 *valbuf);
s32 grf_modbus_get_hold_reg(grf_modbus_t *modbus, u16 addr,u16 num,u16 *valbuf);
s32 grf_modbus_set_input_reg(grf_modbus_t *modbus, u16 addr,u16 num, u16 *valbuf);
s32 grf_modbus_get_input_reg(grf_modbus_t *modbus, u16 addr,u16 num,u16 *valbuf);


#endif
