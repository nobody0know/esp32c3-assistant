/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define HOST_IP_ADDR "192.168.4.54"

#define PORT 12345

static const char *TAG = "UDP";
extern QueueHandle_t imu_data_queue;

// 填充ADC数据报文，为减少发送频率，将5次测量结果放一帧发送，报文总长为 帧头9byte + 5 * 8byte = 49
int udp_data_prepare(uint8_t *databuffer)
{

    databuffer[0] = 0xAA;

    //printf("data waiting to be read : %d available spaces: %d \n",uxQueueMessagesWaiting(imu_data_queue),uxQueueSpacesAvailable(imu_data_queue));//队列剩余空间//队列中待读取的消息uxQueueSpacesAvailable(ADC_queue));//队列剩余空间

    if (uxQueueMessagesWaiting(imu_data_queue) == 0)
    {
        ESP_LOGE(TAG, "imu queue may have something error");
        return 0;
    }
    uint8_t rx_buffer[100];
    memset(rx_buffer, 0, sizeof(rx_buffer));
    if (xQueueReceive(imu_data_queue, rx_buffer, 10))
    {
        memcpy(&databuffer[1], rx_buffer, 20);
    }
    return 1;
}

static void udp_client_task(void *pvParameters)
{
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        while (1)
        {

            extern bool usermsg_send_start;
            if (usermsg_send_start == 1)
            {
                uint8_t udpdatabuffer[20];
                memset(udpdatabuffer,0,sizeof(udpdatabuffer));
                if (udp_data_prepare(udpdatabuffer))
                {
                    int err = sendto(sock, udpdatabuffer, sizeof(udpdatabuffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }
                    ESP_LOGI(TAG, "SEND DATA");
                }
                else
                {
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
            }
            else
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void UDP_Init(void)
{
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 4, NULL);
}
