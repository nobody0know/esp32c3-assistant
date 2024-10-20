#include "EPD.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "MAIN INIT";

static spi_device_handle_t spi;

void EPD_SPIInit(void)
{
	esp_err_t ret;
	spi_bus_config_t buscfg = {
		.miso_io_num = PIN_NUM_MISO, // MISO信号线
		.mosi_io_num = PIN_NUM_MOSI, // MOSI信号线
		.sclk_io_num = PIN_NUM_CLK,	 // SCLK信号线
		.quadwp_io_num = -1,		 // WP信号线，专用于QSPI的D2
		.quadhd_io_num = -1,		 // HD信号线，专用于QSPI的D3
		.max_transfer_sz = 64 * 8,	 // 最大传输数据大小

	};
	// Initialize the SPI bus
	ret = spi_bus_initialize(SPI2_HOST, &buscfg, 0);
	ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz，墨水屏通常使用较低的 SPI 时钟频率
        .mode = 0,                           // SPI 模式 0
        .spics_io_num = GPIO_NUM_4,          // CS 引脚
        .queue_size = 7,                     // 事务队列大小
        .pre_cb = NULL,                      // 事务前的回调函数，可以用于控制 DC 引脚
    };
	// Attach the epaper to the SPI bus
	ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
	    // 复位墨水屏
		#define PIN_NUM_RST   7  // 复位引脚
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "E-Ink display initialized.");
}

void eink_send_command(spi_device_handle_t spi, uint8_t command)
{
    gpio_set_level(PIN_NUM_DC, 0); // 设置 DC 为 0 表示发送命令

    spi_transaction_t t = {
        .length = 8,             // 传输 8 位
        .tx_buffer = &command,   // 命令数据
    };

    esp_err_t ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
	gpio_set_level(PIN_NUM_DC, 1); // 设置 DC 为 0 表示发送命令
}

void eink_send_data(spi_device_handle_t spi, uint8_t data)
{
    gpio_set_level(PIN_NUM_DC, 1); // 设置 DC 为 1 表示发送数据

    spi_transaction_t t = {
        .length = 8,           // 传输 8 位
        .tx_buffer = &data,    // 数据
    };

    esp_err_t ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
	gpio_set_level(PIN_NUM_DC, 1); // 设置 DC 为 0 表示发送命令
}

void EPD_WR_REG(uint8_t reg)
{
	eink_send_command(spi,reg);
}
void EPD_WR_DATA8(uint8_t data)
{
	eink_send_data(spi,data);
}

/*******************************************************************
		函数说明:判忙函数
		入口参数:无
		说明:忙状态为1
*******************************************************************/
void EPD_READBUSY(void)
{
	delay_ms(100);
	// while (1)
	// {
	// 	#define PIN_NUM_BUSY  2   // 忙碌信号引脚
	// 	if (gpio_get_level(PIN_NUM_BUSY) == 0)
	// 	{
	// 		break;
	// 	}
	// }
}

/*******************************************************************
		函数说明:硬件复位函数
		入口参数:无
		说明:在E-Paper进入Deepsleep状态后需要硬件复位
*******************************************************************/
void EPD_HW_RESET(void)
{
	delay_ms(100);
	gpio_set_level(PIN_NUM_RST, 0);
	delay_ms(10);
	gpio_set_level(PIN_NUM_RST, 1);
	delay_ms(10);
	EPD_READBUSY();
}

/*******************************************************************
		函数说明:更新函数
		入口参数:无
		说明:更新显示内容到E-Paper
*******************************************************************/
void EPD_Update(void)
{
	EPD_WR_REG(0x22);
	EPD_WR_DATA8(0xF4);
	EPD_WR_REG(0x20);
	EPD_READBUSY();
}
/*******************************************************************
		函数说明:局刷更新函数
		入口参数:无
		说明:E-Paper工作在局刷模式
*******************************************************************/
void EPD_PartUpdate(void)
{

	EPD_WR_REG(0x22);
	EPD_WR_DATA8(0x1C);
	EPD_WR_REG(0x20);
	EPD_READBUSY();
}
/*******************************************************************
		函数说明:快刷更新函数
		入口参数:无
		说明:E-Paper工作在快刷模式
*******************************************************************/
void EPD_FastUpdate(void)
{
	EPD_WR_REG(0x22);
	EPD_WR_DATA8(0xC7);
	EPD_WR_REG(0x20);
	EPD_READBUSY();
}

/*******************************************************************
		函数说明:休眠函数
		入口参数:无
		说明:屏幕进入低功耗模式
*******************************************************************/
void EPD_DeepSleep(void)
{
	EPD_WR_REG(0x10);
	EPD_WR_DATA8(0x01);
	delay_ms(200);
}

/*******************************************************************
		函数说明:初始化函数
		入口参数:无
		说明:调整E-Paper默认显示方向
*******************************************************************/
void EPD_Init(void)
{
	EPD_SPIInit();
	EPD_HW_RESET();
	EPD_READBUSY();
	EPD_WR_REG(0x12); // SWRESET
	EPD_READBUSY();

	EPD_WR_REG(0x3C); // BorderWavefrom
	EPD_WR_DATA8(0x05);

	EPD_WR_REG(0x01); // Driver output control
	EPD_WR_DATA8((EPD_H - 1) % 256);
	EPD_WR_DATA8((EPD_H - 1) / 256);
	EPD_WR_DATA8(0x00);

	EPD_WR_REG(0x11); // data entry mode
	EPD_WR_DATA8(0x02);
	EPD_WR_REG(0x44); // set Ram-X address start/end position
	EPD_WR_DATA8(EPD_W / 8 - 1);
	EPD_WR_DATA8(0x00);
	EPD_WR_REG(0x45); // set Ram-Y address start/end position
	EPD_WR_DATA8(0x00);
	EPD_WR_DATA8(0x00);
	EPD_WR_DATA8((EPD_H - 1) % 256);
	EPD_WR_DATA8((EPD_H - 1) / 256);
	EPD_WR_REG(0x21); //  Display update control
	EPD_WR_DATA8(0x00);
	EPD_WR_DATA8(0x80);
	EPD_WR_REG(0x18); // Read built-in temperature sensor
	EPD_WR_DATA8(0x80);
	EPD_WR_REG(0x4E); // set RAM x address count to 0;
	EPD_WR_DATA8(EPD_W / 8 - 1);
	EPD_WR_REG(0x4F); // set RAM y address count to 0X199;
	EPD_WR_DATA8(0x00);
	EPD_WR_DATA8(0x00);
	EPD_READBUSY();
}

/*******************************************************************
		函数说明:快刷初始化函数
		入口参数:无
		说明:E-Paper工作在快刷模式
*******************************************************************/
void EPD_FastInit(void)
{
	EPD_HW_RESET();
	EPD_READBUSY();
	//  EPD_WR_REG(0x12);  //旋转180度显示 不能重新进行软复位IC
	//	EPD_READBUSY();
	EPD_WR_REG(0x18);
	EPD_WR_DATA8(0x80);
	EPD_WR_REG(0x22);
	EPD_WR_DATA8(0xB1);
	EPD_WR_REG(0x20);
	EPD_READBUSY();
	EPD_WR_REG(0x1A);
	EPD_WR_DATA8(0x64);
	EPD_WR_DATA8(0x00);
	EPD_WR_REG(0x22);
	EPD_WR_DATA8(0x91);
	EPD_WR_REG(0x20);
	EPD_READBUSY();
}

/*******************************************************************
		函数说明:清屏函数
		入口参数:无
		说明:E-Paper刷白屏
*******************************************************************/
void EPD_Display_Clear(void)
{
	uint16_t i;
	EPD_WR_REG(0x3C);
	EPD_WR_DATA8(0x01);
	EPD_WR_REG(0x24);
	for (i = 0; i < 5624; i++)
	{
		EPD_WR_DATA8(0xFF);
	}
	EPD_WR_REG(0x26);
	for (i = 0; i < 5624; i++)
	{
		EPD_WR_DATA8(0xFF);
	}
}

/*******************************************************************
		函数说明:局刷擦除旧数据
		入口参数:无
		说明:E-Paper工作在局刷模式下调用
*******************************************************************/
void EPD_Clear_R26H(void)
{
	uint16_t i;
	EPD_READBUSY();
	EPD_WR_REG(0x26);
	for (i = 0; i < 5624; i++)
	{
		EPD_WR_DATA8(0xFF);
	}
}

/*******************************************************************
		函数说明:数组数据更新到E-Paper
		入口参数:无
		说明:
*******************************************************************/
void EPD_Display(const uint8_t *image)
{
	uint16_t i, j, Width, Height;
	Width = (EPD_W % 8 == 0) ? (EPD_W / 8) : (EPD_W / 8 + 1);
	Height = EPD_H;
	EPD_WR_REG(0x24);
	for (j = 0; j < Height; j++)
	{
		for (i = 0; i < Width; i++)
		{
			EPD_WR_DATA8(image[i + j * Width]);
		}
	}
}
