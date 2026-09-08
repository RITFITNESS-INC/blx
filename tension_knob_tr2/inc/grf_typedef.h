#ifndef _GRF_TYPEDEF_H_
#define _GRF_TYPEDEF_H_

/*This code is automatically written by the IDE, do not add user code*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define GRF_OK    (0)
#define GRF_FAIL  (-1)

#ifndef NULL 
#define NULL 0
#endif

typedef enum
{
    GRF_FALSE,
    GRF_TRUE
}grf_bool;

typedef u32  grf_color_t;
#define GRF_COLOR_GET(r,g,b)   (0xff<<24|((r)<<16)|((g)<<8)|(b))
typedef struct  
{
    u32 width;
    u32 height;
} grf_size_t;

typedef s32 grf_coord_t;

typedef struct  
{
    grf_coord_t x;
    grf_coord_t y;
} grf_point_t;
//小数的整数与小数部分
typedef struct {
    u16 i;
    u16 f;
} grf_sqrt_t;
//控件所在区域，（x1,y1）控件左上角坐标，（x2,y2）控件右下角坐标
typedef struct GRF_AREA_T {
    grf_coord_t x1;
    grf_coord_t y1;
    grf_coord_t x2;
    grf_coord_t y2;
} grf_area_t;

typedef struct 
{
    u32 view_id;
    void (*grf_view_init)(void);
    void (*grf_view_entry)(void);
    void (*grf_view_exit)(void);
}grf_view_fun_t;
s32 grf_prj_create(grf_view_fun_t views_fun_t[],u32 view_cont);


typedef struct 
{
    u32 ctrl_id;
    void (*grf_ctrl_event)(void *ctrl, u32 event);
}grf_ctrl_fun_t;
s32 grf_view_create(u32 v_num,grf_ctrl_fun_t ctrls_fun_t[],u32 ctrl_cont);
s32 grf_view_delete(u32 v_num);

















#endif
