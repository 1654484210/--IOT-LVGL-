#ifndef __MY_TEST_H__
#define __MY_TEST_H__

// 标准库头文件
#include <stdio.h>     // 标准输入输出
#include <stdlib.h>    // 标准库函数
#include <string.h>    // 字符串操作
#include <strings.h>   // 字符串操作(BSD)
#include <stdbool.h>   // 布尔类型
#include <stdint.h>    // 标准整数类型
#include <signal.h>    // 信号处理
#include <semaphore.h> // 信号量
#include <errno.h>     // 错误号定义
#include <fcntl.h>     // 文件控制
#include <unistd.h>    // POSIX系统调用
#include <pthread.h>   // 线程支持
#include <time.h>

// 系统调用相关头文件
#include <sys/stat.h>   // 文件状态
#include <sys/time.h>   // 时间相关
#include <sys/types.h>  // 基本系统数据类型
#include <sys/mman.h>   // 内存映射
#include <sys/socket.h> // 套接字
#include <sys/ioctl.h>

// Linux特定头文件
#include <linux/fb.h> // 帧缓冲
#include <linux/un.h> // UNIX域套接字

// 网络相关头文件
#include <arpa/inet.h>  // 网络地址转换
#include <netinet/in.h> // 网际协议
#include <netdb.h>      // 网络数据库操作

// LVGL相关头文件
#include "lvgl/lvgl.h"                    // LVGL核心库
#include "lv_drivers/display/fbdev.h"     // 帧缓冲显示驱动
#include "lv_drivers/indev/evdev.h"       // 输入设备驱动
#include "lvgl/demos/lv_demos.h"          // LVGL示例
#include "lv_font_source_han_sans_bold.h" // 字体

// 函数声明
extern void init_cont();                                                   // 主界面
extern void init_timebox();                                                // 时间界面
extern void init_weatherbox(char city[20], char txt[1024], char temp[10]); // 天气界面
extern void init_btnbox();                                                 // 按钮界面
extern void update_weather_display(char * city, char * txt, char * temp);
extern void update_time(lv_timer_t * timer); // 更新时间

extern void btn_Buzzer_handler(lv_event_t * e); // 蜂鸣器按钮事件
extern void btn_LED_handler(lv_event_t * e);    // led按钮事件
#endif