#获取当前工程绝对路径
LOCAL_PATH = $(dir  $(abspath $(lastword $(MAKEFILE_LIST))))
PROJ_INC_INCLUDE := ../../inc
PROJ_LIBS_PLATFORM := ./

#工程源码顶层目录
PROJ_PATH := ../../apps
#递归遍历目录下所有文件
recursioncard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call recursioncard,$d/,$2))
#获取工程下所有文件
PROJ_ALL_FILES := $(foreach p,$(PROJ_PATH),$(call recursioncard,$(p),*.*))
#过滤文件，只留.c文件
PROJ_C_FILES := $(filter %.c,$(PROJ_ALL_FILES))

#获取相应的目录
LOCAL_C_FILES := $(PROJ_C_FILES)
PROJ_C_DIR := $(dir $(foreach p,$(LOCAL_C_FILES),$(p)))
PROJ_C_DIR := $(sort $(PROJ_C_DIR))

#c文件匹配o文件
OBJS_PATH = tobj/a/
OBJS_O = $(patsubst %.c,%.o,$(LOCAL_C_FILES))
OBJS = $(addprefix $(OBJS_PATH),$(OBJS_O))
OBJS_DIR = $(addprefix $(OBJS_PATH),$(PROJ_C_DIR))

CFLAGS = -Wincompatible-pointer-types
#包含头文件
INCLUDE_DIR := -I$(PROJ_INC_INCLUDE)
LIB_DIR := -L$(PROJ_LIBS_PLATFORM) -L./libs_a/ -L./dll/
#连接库
LIBFLAGS = -lgrfui -lmingw32 -lSDL2 -lfreetype-6 -lavformat-60 -lavformat-60 -lavutil-58 -lswscale-7 -lavcodec-60 -lswresample-4
TIMESTAMP=$(shell date +%Y%m%d%H%M%S)

TOBJ = tobj
TAPP = apps

#命令
MKDIR = mkdir
RM = rm
ECHO = echo
TARGET = app.exe
CP = cp
all : objsdir $(TARGET)

$(OBJS_PATH)%.o:%.c
	@$(CC) -g -c $< -o $@ $(INCLUDE_DIR) $(CFLAGS)$(LIBS) 

$(TARGET):$(OBJS)
	@$(CC) $^ -o $@ $(LIBFLAGS) $(LIB_DIR) $(CFLAGS)
	@$(RM) $(TOBJ) -rf
	./app.exe t $(TIMESTAMP)

objsdir:
	@$(MKDIR) $(OBJS_DIR) -p
	
clean:
	@$(RM) $(TAPP) -rf
	@$(RM) $(TARGET) -f
	
.PHONY:all clean objsdir $(TARGET)
	
	