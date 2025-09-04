#include "my_test.h"

// 定义LED控制命令
#define TEST_MAGIC 'x'
#define LED1 _IO(TEST_MAGIC, 0)
#define LED2 _IO(TEST_MAGIC, 1)
#define LED3 _IO(TEST_MAGIC, 2)
#define LED4 _IO(TEST_MAGIC, 3)
#define LED_ON 0
#define LED_OFF 1

// 全局UI对象
lv_obj_t * time_label = NULL;
lv_obj_t * cont       = NULL;
lv_timer_t * timer    = NULL;
lv_obj_t * city_label = NULL;
lv_obj_t * txt_label  = NULL;
lv_obj_t * tmp_label  = NULL;

// 蜂鸣器控制函数
void btn_Buzzer_handler(lv_event_t * Buz)
{
    static int fd1         = -1;
    static int beep_status = 0;

    if(lv_event_get_code(Buz) == LV_EVENT_CLICKED) {
        printf("Buzzer Button clicked\n");
        beep_status = !beep_status;

        if(beep_status) {
            fd1 = open("/dev/beep", O_RDWR);
            ioctl(fd1, 0, beep_status);
        } else {
            ioctl(fd1, 1, 1);
            close(fd1);
        }
    }
}

// LED控制线程函数
void * led_thread_func(void * arg)
{
    int * fd2 = (int *)arg;
    while(1) {
        ioctl(*fd2, LED1, LED_ON);
        ioctl(*fd2, LED2, LED_ON);
        ioctl(*fd2, LED3, LED_ON);
        ioctl(*fd2, LED4, LED_ON);
        usleep(1000000);

        ioctl(*fd2, LED1, LED_OFF);
        usleep(1000000);

        ioctl(*fd2, LED2, LED_OFF);
        usleep(1000000);

        ioctl(*fd2, LED3, LED_OFF);
        usleep(1000000);

        ioctl(*fd2, LED4, LED_OFF);
        usleep(1000000);
    }
    return NULL;
}

// LED按钮处理函数
void btn_LED_handler(lv_event_t * LED)
{
    static int fd2        = -1;
    static int led_status = 0;
    static pthread_t led_thread;

    if(lv_event_get_code(LED) == LV_EVENT_CLICKED) {
        printf("LED Button clicked\n");
        led_status = !led_status;

        fd2 = open("/dev/Led", O_RDWR);
        if(fd2 < 0) {
            perror("open led device error");
            return;
        }

        if(led_status) {
            pthread_create(&led_thread, NULL, led_thread_func, &fd2);
        } else {
            pthread_cancel(led_thread);
            close(fd2);
        }
    }
}

// 更新时间显示
void update_time(lv_timer_t * timer)
{
    time_t now;
    struct tm * timeinfo;
    char buffer[80];
    time(&now);
    timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    lv_label_set_text(time_label, buffer);
}

// 初始化主容器
void init_cont()
{
    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xf0f0f0), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
}

// 初始化时间显示
void init_timebox()
{
    lv_obj_t * time_box = lv_obj_create(cont);
    lv_obj_set_size(time_box, LV_PCT(100), 100);
    lv_obj_set_pos(time_box, 0, 0);
    lv_obj_set_style_bg_color(time_box, lv_color_hex(0x1890ff), 0);
    lv_obj_set_layout(time_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(time_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    time_label = lv_label_create(time_box);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);

    timer = lv_timer_create(update_time, 1000, NULL);
    lv_timer_ready(timer);
}

// 初始化天气显示
void init_weatherbox(char city[20], char txt[1024], char temp[10])
{
    lv_obj_t * weather_box = lv_obj_create(cont);
    lv_obj_set_size(weather_box, 400, 300);
    lv_obj_set_pos(weather_box, 10, 120);
    lv_obj_set_style_bg_color(weather_box, lv_color_hex(0xff9900), 0);
    lv_obj_set_layout(weather_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(weather_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(weather_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    city_label = lv_label_create(weather_box);
    lv_label_set_text(city_label, city);
    lv_obj_set_style_text_font(city_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(city_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(city_label, LV_TEXT_ALIGN_CENTER, 0);

    txt_label = lv_label_create(weather_box);
    lv_label_set_text(txt_label, txt);
    lv_obj_set_style_text_font(txt_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(txt_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(txt_label, LV_TEXT_ALIGN_CENTER, 0);

    tmp_label = lv_label_create(weather_box);
    lv_label_set_text(tmp_label, temp);
    lv_obj_set_style_text_font(tmp_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(tmp_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(tmp_label, LV_TEXT_ALIGN_CENTER, 0);
}

// 更新天气显示
void update_weather_display(char * city, char * txt, char * temp)
{
    if(city_label) {
        lv_label_set_text(city_label, city);
        lv_obj_set_style_text_font(city_label, &lv_font_montserrat_24, 0);
    }
    if(txt_label) {
        lv_label_set_text(txt_label, txt);
        lv_obj_set_style_text_font(txt_label, &lv_font_montserrat_24, 0);
    }
    if(tmp_label) {
        lv_label_set_text(tmp_label, temp);
        lv_obj_set_style_text_font(tmp_label, &lv_font_montserrat_24, 0);
    }
}

// 初始化按钮区域
void init_btnbox()
{
    lv_obj_t * btn_box = lv_obj_create(cont);
    lv_obj_set_size(btn_box, 240, 200);
    lv_obj_set_pos(btn_box, 450, 120);
    lv_obj_set_style_bg_color(btn_box, lv_color_hex(0xFFFF00), 0);
    lv_obj_clear_flag(btn_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(btn_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(btn_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 蜂鸣器按钮
    lv_obj_t * btn1 = lv_btn_create(btn_box);
    lv_obj_set_size(btn1, 120, 50);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x1890ff), 0);
    lv_obj_add_event_cb(btn1, btn_Buzzer_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_Buzzer = lv_label_create(btn1);
    lv_label_set_text(btn_Buzzer, "蜂鸣器");
    lv_obj_set_style_text_font(btn_Buzzer, &chinese_ziku, 0);
    lv_obj_set_style_text_color(btn_Buzzer, lv_color_white(), 0);
    lv_obj_center(btn_Buzzer);

    // LED按钮
    lv_obj_t * btn2 = lv_btn_create(btn_box);
    lv_obj_set_size(btn2, 120, 50);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x1890ff), 0);
    lv_obj_add_event_cb(btn2, btn_LED_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_LED = lv_label_create(btn2);
    lv_label_set_text(btn_LED, "LED");
    lv_obj_set_style_text_font(btn_LED, &chinese_ziku, 0);
    lv_obj_set_style_text_color(btn_LED, lv_color_white(), 0);
    lv_obj_center(btn_LED);
}