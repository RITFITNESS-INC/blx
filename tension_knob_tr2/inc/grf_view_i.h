#ifndef __GRF_VIEW_I_H_
#define __GRF_VIEW_I_H_

/*This code is automatically written by the IDE, do not add user code*/

#include "grf_typedef.h"
#include "grf_ctrl_i.h"


typedef void grf_view_t;
typedef enum 
{
	GRF_LAYER_BG = 3,			//背景图层
    GRF_LAYER_UI = 0,           //UI图层
    GRF_LAYER_STATE,        //状态图层
    GRF_LAYER_TOP,          //顶层图层
}grf_layer_se;
   
typedef enum {
    GRF_SCR_LOAD_ANIM_NONE,
    GRF_SCR_LOAD_ANIM_OVER_LEFT,
    GRF_SCR_LOAD_ANIM_OVER_RIGHT,
    GRF_SCR_LOAD_ANIM_OVER_TOP,
    GRF_SCR_LOAD_ANIM_OVER_BOTTOM,
    GRF_SCR_LOAD_ANIM_MOVE_LEFT,
    GRF_SCR_LOAD_ANIM_MOVE_RIGHT,
    GRF_SCR_LOAD_ANIM_MOVE_TOP,
    GRF_SCR_LOAD_ANIM_MOVE_BOTTOM,
    GRF_SCR_LOAD_ANIM_FADE_IN,
    GRF_SCR_LOAD_ANIM_FADE_ON,
    GRF_SCR_LOAD_ANIM_FADE_OUT,
    GRF_SCR_LOAD_ANIM_OUT_LEFT,
    GRF_SCR_LOAD_ANIM_OUT_RIGHT,
    GRF_SCR_LOAD_ANIM_OUT_TOP,
    GRF_SCR_LOAD_ANIM_OUT_BOTTOM,
	GRF_SCR_LOAD_ANIM_IN_ZOOM,
	GRF_SCR_LOAD_ANIM_OUT_ZOOM,
} grf_scr_load_anim_t;

//设置显示图层位置  
s32 grf_view_set_layer(u32 v_id,grf_layer_se layer);        
//设置显示视图             
s32 grf_view_set_dis_view(u32 v_id);
//设置动画显示视图
s32 grf_view_set_dis_view_anim(u32 v_id,grf_scr_load_anim_t anim_type, u32 time, u32 delay,grf_anim_path_e path_e);
//获取当前视图ID
s32 grf_view_get_cur_id(grf_layer_se layer);                      
//隐藏图层
s32 grf_view_hide_layer(grf_layer_se layer);
//加载视图资源,但是不显示
s32 grf_view_load(u32 v_num);
//设置视图背景图片
s32 grf_view_set_bg(u32 v_id,u8* img_name);
//设置视图背景色
s32 grf_view_set_bgcolor(u32 v_id,grf_color_t color_t);
//设置视图背景透明度
s32 grf_view_set_bg_opa(u32 v_id,u8 opa);
//得到视图宽高
s32 grf_view_get_dis_wh(grf_size_t* disp_size);


#endif
