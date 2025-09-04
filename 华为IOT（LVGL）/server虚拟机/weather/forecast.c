#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <strings.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <netdb.h>
#include "cJSON.h"

#define MY_IP "192.168.2.22"
#define MY_PORT 60000
#define OTHER_IP "192.168.2.24"
#define OTHER_PORT 60001

// 天气数据结构体
struct wea_date
{
	char city[20];	// 城市名称
	char txt[1024]; // 天气情况
	char temp[10];	// 温度
};

int Socket(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd < 0)
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	return fd;
}

// 获取用户输入的城市名称
void get_city_input(struct wea_date *data)
{
	printf("请输入要查询的城市名称（中文或拼音）：");
	fgets(data->city, sizeof(data->city), stdin);
	// 去除换行符
	data->city[strcspn(data->city, "\n")] = '\0';
}

// 生成HTTP请求
char *generate_http_request(const char *city)
{
	static char request[1024];
	snprintf(request, sizeof(request),
			 "GET /v3/weather/now.json?key=SAewqnjWlC7dvMLfL&location=%s&language=zh-Hans&unit=c HTTP/1.1\r\n"
			 "Host:api.seniverse.com\r\n\r\n",
			 city);
	return request;
}

void parse_response(char *res, int *ok, int *len)
{
	char *retcode = res + strlen("HTTP/1.x ");
	switch (atoi(retcode))
	{
	case 200 ... 299:
		*ok = 1;
		printf("查询成功\n");
		break;
	case 400 ... 499:
		*ok = 0;
		printf("客户端错误\n");
		exit(0);
	case 500 ... 599:
		*ok = 0;
		printf("服务端错误\n");
		exit(0);
	}

	char *p;
	if ((p = strstr(res, "Content-Length: ")))
	{
		*len = atoi(p + strlen("Content-Length: "));
	}
}

// 查询天气并填充结构体
void query_weather(struct wea_date *data)
{
	// 解析域名获取IP
	struct hostent *he = gethostbyname("api.seniverse.com");
	if (he == NULL)
	{
		perror("DNS查询失败");
		exit(EXIT_FAILURE);
	}

	// 设置服务器地址
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);
	addr.sin_family = AF_INET;
	addr.sin_addr = *(struct in_addr *)((he->h_addr_list)[0]);
	addr.sin_port = htons(80);

	// 创建TCP连接
	int fd = Socket(AF_INET, SOCK_STREAM, 0);
	if (connect(fd, (struct sockaddr *)&addr, len) != 0)
	{
		perror("连接服务器失败");
		exit(EXIT_FAILURE);
	}

	// 发送HTTP请求
	char *request = generate_http_request(data->city);
	write(fd, request, strlen(request));

	// 接收HTTP响应头
	char res[1024];
	int total = 0;
	while (1)
	{
		int n = read(fd, res + total, 1);
		if (n <= 0)
		{
			perror("读取HTTP头部失败");
			exit(EXIT_FAILURE);
		}
		total += n;
		if (strstr(res, "\r\n\r\n"))
			break;
	}

	// 解析响应头
	int ok, jsonlen = 0;
	parse_response(res, &ok, &jsonlen);
	if (!ok || jsonlen == 0)
	{
		exit(EXIT_FAILURE);
	}

	// 接收JSON响应体
	char *json = calloc(1, jsonlen);
	total = 0;
	while (jsonlen > 0)
	{
		int n = read(fd, json + total, jsonlen);
		total += n;
		jsonlen -= n;
	}

	// 解析JSON数据
	cJSON *root = cJSON_Parse(json);
	if (root == NULL)
	{
		printf("JSON解析失败: %s\n", cJSON_GetErrorPtr());
		free(json);
		exit(EXIT_FAILURE);
	}

	cJSON *results = cJSON_GetObjectItem(root, "results");
	if (results)
	{
		cJSON *first_result = cJSON_GetArrayItem(results, 0);
		if (first_result)
		{
			cJSON *now = cJSON_GetObjectItem(first_result, "now");
			if (now)
			{
				cJSON *text = cJSON_GetObjectItem(now, "text");
				cJSON *temp = cJSON_GetObjectItem(now, "temperature");

				if (text && temp)
				{
					strncpy(data->txt, text->valuestring, sizeof(data->txt) - 1);
					strncpy(data->temp, temp->valuestring, sizeof(data->temp) - 1);

					printf("\n城市: %s\n", data->city);
					printf("天气情况: %s\n", data->txt);
					printf("当前气温: %s°C\n\n", data->temp);
				}
				else
				{
					printf("天气数据解析失败\n");
				}
			}
		}
	}

	free(json);
	cJSON_Delete(root);
	close(fd);
}

int main(void)
{
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		perror("socket fail");
		return EXIT_FAILURE;
	}

	// 设置端口复用
	int optval = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	// 绑定本地地址
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MY_PORT);
	my_addr.sin_addr.s_addr = inet_addr(MY_IP);
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
	{
		perror("bind fail");
		close(socket_fd);
		return EXIT_FAILURE;
	}
	printf("绑定本机成功[%s][%d]\n", MY_IP, MY_PORT);

	// 设置目标地址
	struct sockaddr_in other_addr;
	other_addr.sin_family = AF_INET;
	other_addr.sin_port = htons(OTHER_PORT);
	other_addr.sin_addr.s_addr = inet_addr(OTHER_IP);

	while (1)
	{
		struct wea_date weather_data;

		// 1. 获取用户输入的城市名称
		get_city_input(&weather_data);

		// 2. 查询天气数据
		query_weather(&weather_data);

		// 3. 通过UDP发送天气数据
		if (sendto(socket_fd, &weather_data, sizeof(weather_data), 0,
				   (struct sockaddr *)&other_addr, sizeof(other_addr)) < 0)
		{
			perror("发送天气数据失败");
		}
		else
		{
			printf("天气数据已发送至 %s:%d\n\n", OTHER_IP, OTHER_PORT);
		}
		break;
	}

	close(socket_fd);
	return EXIT_SUCCESS;
}