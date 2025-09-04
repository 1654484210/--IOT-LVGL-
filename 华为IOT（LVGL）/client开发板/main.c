#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "test/my_test.h"

#define DISP_BUF_SIZE (480 * 800)
#define MY_IP "192.168.2.24"
#define MY_PORT 60001

// 定义LED控制命令
#define TEST_MAGIC 'x'
#define LED1 _IO(TEST_MAGIC, 0)
#define LED2 _IO(TEST_MAGIC, 1)
#define LED3 _IO(TEST_MAGIC, 2)
#define LED4 _IO(TEST_MAGIC, 3)
#define LED_ON 0
#define LED_OFF 1

struct wea_date
{
    char city[20];
    char txt[1024];
    char temp[10];
};

int temp = 0;

// 中转英文函数
void trans_fun(char weath[20])
{
    if(strcmp(weath, "晴") == 0) {
        strcpy(weath, "sunny");
    }
    if(strcmp(weath, "中雨") == 0) {
        strcpy(weath, "moderate rain");
    }
    if(strcmp(weath, "小雨") == 0) {
        strcpy(weath, "light rain");
    }
    if(strcmp(weath, "多云") == 0) {
        strcpy(weath, "cloudy");
    }
    if(strcmp(weath, "阴") == 0) {
        strcpy(weath, "overcast");
    }
}

// 根据温度控制线程函数
void * led_func(void * arg)
{
    // 打开蜂鸣器
    int fd = open("/dev/beep", O_RDWR);
    if(fd == -1) {
        perror("open:/dev/beep");
        exit(0);
    }

    // 打开LED设备
    int fd2 = open("/dev/Led", O_RDWR);
    if(fd2 < 0) {
        perror("open led device error");
        exit(0);
    }

    int cnt = 5;

    if(temp > 0 && temp < 10) {
        printf("温度小于10度,每2秒闪烁一次\n");
        while(cnt) {
            ioctl(fd2, LED1, LED_OFF);
            ioctl(fd2, LED2, LED_OFF);
            ioctl(fd2, LED3, LED_OFF);
            ioctl(fd2, LED4, LED_OFF);
            sleep(2);
            ioctl(fd2, LED1, LED_ON);
            ioctl(fd2, LED2, LED_ON);
            ioctl(fd2, LED3, LED_ON);
            ioctl(fd2, LED4, LED_ON);
            cnt--;
        }
        printf("停止闪烁");
    }

    if(temp > 10 && temp < 30) {
        printf("温度小于30度,每1秒闪烁一次\n");
        while(cnt) {
            ioctl(fd2, LED1, LED_OFF);
            ioctl(fd2, LED2, LED_OFF);
            ioctl(fd2, LED3, LED_OFF);
            ioctl(fd2, LED4, LED_OFF);
            usleep(500000);
            ioctl(fd2, LED1, LED_ON);
            ioctl(fd2, LED2, LED_ON);
            ioctl(fd2, LED3, LED_ON);
            ioctl(fd2, LED4, LED_ON);
            usleep(500000); // 0.5秒
            cnt--;
        }
        printf("停止闪烁\n");
    }
    if(temp > 30) {
        printf("温度大于30度,每1秒闪烁一次并且发出警报\n");
        while(cnt) {
            ioctl(fd2, LED1, LED_OFF);
            ioctl(fd2, LED2, LED_OFF);
            ioctl(fd2, LED3, LED_OFF);
            ioctl(fd2, LED4, LED_OFF);

            // 打开蜂鸣器
            ioctl(fd, 0, 1);
            usleep(500000); // 0.5秒
            // 关闭蜂鸣器
            ioctl(fd, 1, 1);
            usleep(500000); // 0.5秒
            ioctl(fd2, LED1, LED_ON);
            ioctl(fd2, LED2, LED_ON);
            ioctl(fd2, LED3, LED_ON);
            ioctl(fd2, LED4, LED_ON);
            cnt--;
        }
        printf("停止闪烁");
    }

    close(fd);
    close(fd2);
    pthread_exit(NULL); // 结束线程
}

struct wea_date current_weather;
pthread_mutex_t weather_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;
    return now_ms - start_ms;
}

void * network_receiver_thread(void * arg)
{
    int ret;
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        perror("socket fail");
        return NULL;
    }

    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in my_addr;
    my_addr.sin_family      = AF_INET;
    my_addr.sin_port        = htons(MY_PORT);
    my_addr.sin_addr.s_addr = inet_addr(MY_IP);
    ret                     = bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if(ret < 0) {
        perror("bind fail");
        close(socket_fd);
        return NULL;
    }
    printf("接收端已启动，等待数据...\n");

    while(1) {
        struct wea_date weather_data;
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        ret = recvfrom(socket_fd, &weather_data, sizeof(weather_data), 0, (struct sockaddr *)&from_addr, &from_len);
        if(ret < 0) {
            perror("recvfrom error");
            continue;
        }

        printf("\n=== 收到天气数据 ===\n");
        printf("城市: %s\n", weather_data.city);
        printf("天气: %s\n", weather_data.txt);
        printf("温度: %s°C\n", weather_data.temp);
        printf("===================\n\n");

        pthread_mutex_lock(&weather_mutex);
        memcpy(&current_weather, &weather_data, sizeof(struct wea_date));
        pthread_mutex_unlock(&weather_mutex);

        // 获取温度
        temp = atoi(weather_data.temp);

        trans_fun(weather_data.txt);
        char city_1[50] = {"city:"};
        strcat(city_1, weather_data.city);
        char txt_1[50] = {"txt:"};
        strcat(txt_1, weather_data.txt);
        char temp_1[50] = {"temp:"};
        strcat(temp_1, weather_data.temp);
        char temp_2[50] = {"°C"};
        strcat(temp_1, temp_2);
        update_weather_display(city_1, txt_1, temp_1);

        pthread_t LED_thread;
        if(pthread_create(&LED_thread, NULL, led_func, NULL) != 0) {
            perror("无法创建网络线程");
        }
    }

    close(socket_fd);
    return NULL;
}

int main()
{
    memset(&current_weather, 0, sizeof(struct wea_date));
    strcpy(current_weather.city, "city:Waiting");
    strcpy(current_weather.txt, "txt:Waiting");
    strcpy(current_weather.temp, "temp:Waiting");

    pthread_t network_thread;
    if(pthread_create(&network_thread, NULL, network_receiver_thread, NULL) != 0) {
        perror("无法创建网络线程");
        return -1;
    }

    lv_init();
    fbdev_init();

    lv_color_t * buf1 = malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    if(!buf1) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, DISP_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf  = &disp_buf;
    disp_drv.flush_cb  = fbdev_flush;
    disp_drv.hor_res   = 800;
    disp_drv.ver_res   = 480;
    disp_drv.sw_rotate = 1;
    disp_drv.rotated   = LV_DISP_ROT_NONE;
    lv_disp_drv_register(&disp_drv);

    evdev_init();

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

    init_cont();
    init_timebox();
    init_weatherbox(current_weather.city, current_weather.txt, current_weather.temp);
    init_btnbox();

    uint32_t last_tick = custom_tick_get();
    while(1) {
        lv_timer_handler();

        uint32_t current_tick = custom_tick_get();
        uint32_t elapsed      = current_tick - last_tick;

        if(elapsed >= 5) {
            lv_tick_inc(elapsed);
            last_tick = current_tick;
        }

        usleep(2000);
    }

    pthread_join(network_thread, NULL);
    free(buf1);
    return 0;
}