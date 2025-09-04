#
# Makefile
#-L/home/gec/freetype-2.12.1/tmp/lib -I/home/gec/freetype-2.12.1/tmp/include/freetype2
CC = arm-linux-gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0   -I$(LVGL_DIR)/ -Wall  -std=gnu99
LDFLAGS ?= -lm -lpthread  # 添加pthread链接
BIN = demo


#Collect the files to compile
MAINSRC = ./main.c
TESTSRC = $(wildcard ./test/*.c) ##1. 添加新的源文件

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))
TESTOBJ = $(TESTSRC:.c=$(OBJEXT)) ##2. 添加.o文件

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES


all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

#3.将新的文件添加到目标文件中       
default: $(AOBJS) $(COBJS) $(MAINOBJ) $(TESTOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(TESTOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

#4.添加新删除的目标文件
clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) $(TESTOBJ)

