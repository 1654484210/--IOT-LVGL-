#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/fb.h>
#include <linux/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#define CITY "guangzhou" // 城市

// 初始化LVGL界面
void init_lvgl_ui()
{
    // 创建主容器
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xf0f0f0), 0);
    lv_obj_set_style_border_width(cont, 0, 0);

    // 时间标签
    lv_obj_t * time_label = lv_label_create(status_bar);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);

    // 天气标签
    lv_obj_t * weather_label = lv_label_create(status_bar);
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(weather_label, lv_color_white(), 0);
    lv_obj_align(weather_label, LV_ALIGN_RIGHT_MID, -10, 0);
}