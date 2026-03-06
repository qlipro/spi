/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include "aht20.h"
#include "stdio.h"
#include "6050.h"
#include "printf.h"
#include "cursor.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//UART_HandleTypeDef huart2;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t last_process_time = 0;
uint32_t last_display1 = 0;
uint32_t last_display2 = 0;
char display_buf[16];  // 局部缓冲区，不占用全局内存
char uart_buffer[32];
float angle1 =0;
float angle2 =0;


static int16_t last_position = 0;
static uint32_t last_rotate_time = 0;
static uint8_t cursor_moved = 0;

// 字符集
static const char* charset[] = {
    "abcdefghijklmnopqrstuvwxyz",            // 模式0: 小写字母
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",            // 模式1: 大写字母
    "0123456789",                            // 模式2: 数字
	" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",    // 模式3: 符号 (33个)
	"\b \t\r\n\v\f"                          // 模式4: 控制字符 (退格,空格,制表,回车,换行,垂直制表,换页
};
// 控制字符的预览显示（与charset[4]一一对应）
static const char* ctrl_preview[] = {
	    "\\b",  // 退格
	    "[]",  // 空格
	    "\\t",  // 制表符
	    "\\r",  // 回车
	    "\\n",  // 换行
	    "\\v",  // 垂直制表
	    "\\f"   // 换页
};

static uint8_t mode = 0;
static uint8_t char_index = 0;

// 控制字符的实际字符（与charset[3]相同，用于执行）
#define CTRL_CHARS charset[4]


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
LCD_Init();

LCD_Clear(CYAN);


float temperature, humidity; // 温度和湿度变量
 char message_uart[50];       // 要发送的字符串
 char message_temp[30];	// oled显示温度字符串
 char message_hum[30];		// oled显示湿度字符串
 char message[50];
 AHT20_Init();
 HAL_Delay(20);


if(MPU6050_Init(&hi2c1) == HAL_OK) {
	LCD_ShowString(8, 6, "MPU6050 is ready", RED, &afont8x6);
	// 	  OLED_DrawImage((128 - (lklkImg.w)) / 2, 0, &lklkImg, OLED_COLOR_NORMAL);
	// 	  OLED_ShowFrame();
	/* 启动第一次DMA读取 */
	HAL_Delay(100);
	MPU6050_StartDMARead();
	} else {
	while(1);
	}


HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);

LCD_InitContext(55, 12, &afont12x8, 0xFFFF, 0x0000);

LCD_Printf("hello\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  char pos_buf[20];
	      sprintf(pos_buf, "X:%03d Y:%03d", lcd_ctx.cursor_x-8, lcd_ctx.cursor_y-lcd_ctx.start_y);
	      LCD_ShowString(0, 30, pos_buf, GREEN, &afont8x6);


	  // 1. 读取编码器位置
	         int16_t current_position = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);

	         // 2. 检测是否旋转
	         if (current_position != last_position) {
	             int8_t direction = (current_position > last_position) ? 1 : -1;
	             last_position = current_position;

	             // 更新字符索引
	             int len = strlen(charset[mode]);
	             char_index = (char_index + len + direction) % len;

	             if (mode == 4) {
	                 // 模式4：显示控制字符预览
	                 LCD_ShowString(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
	                                ctrl_preview[char_index],
	                                lcd_ctx.text_color, lcd_ctx.font);
	                 // ShowString 不移动光标
	             } else {
	             // 显示字符
	             LCD_Printf("%c", charset[mode][char_index]);

	             lcd_ctx.cursor_x -= lcd_ctx.font->w;
	             }
	             LCD_ClearArea(lcd_ctx.cursor_x, lcd_ctx.cursor_y+lcd_ctx.font->h-1,
	             	                    lcd_ctx.font->w, 1,
	             	                    0x0000);
	             // 记录时间
	             last_rotate_time = HAL_GetTick();
	             cursor_moved = 0;
	         }

	         // 3. 检测按钮
	         if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
	             HAL_Delay(20);
	             if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
	                 while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET);
	                 LCD_ClearArea(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
								   lcd_ctx.font->w * 2, lcd_ctx.font->h, lcd_ctx.bg_color);
	                 mode = (mode + 1) % 5;
	                 char_index = 0;

	                 if (mode == 4) {
						 // 模式3：显示第一个控制字符预览
						 LCD_ShowString(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
										ctrl_preview[0],
										lcd_ctx.text_color, lcd_ctx.font);
					 } else {
						 // 其他模式：显示第一个字符
						 LCD_Printf("%c", charset[mode][char_index]);
						 lcd_ctx.cursor_x -= lcd_ctx.font->w;
					 }

	                 last_rotate_time = HAL_GetTick();
	                 cursor_moved = 0;
	             }
	         }

	         // 4. 800ms 超时推进光标
	         if (!cursor_moved && last_rotate_time != 0) {
	             if (HAL_GetTick() - last_rotate_time > 800) {

	            	 if (mode == 4) {
	            	             // 模式3：执行控制字符功能
	            	             // 先擦除预览的 [XXX]
	            	             LCD_ClearArea(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
	            	                           lcd_ctx.font->w * 2, lcd_ctx.font->h, lcd_ctx.bg_color);

	            	             // 通过printf输出实际控制字符（自动执行功能）
	            	             LCD_Printf("%c", CTRL_CHARS[char_index]);


	            	         } else {
	            	             // 其他模式：正常推进光标
								 lcd_ctx.cursor_x += lcd_ctx.font->w;
								 if (lcd_ctx.cursor_x + lcd_ctx.font->w > lcd_ctx.width + 8) {
									 lcd_ctx.cursor_x = 8;
									 lcd_ctx.cursor_y += lcd_ctx.line_height;
								 }
	            	         }
	                 cursor_moved = 1;
	       	      LCD_ClearArea(lcd_ctx.cursor_x, lcd_ctx.cursor_y+lcd_ctx.font->h-1,
	       	                    lcd_ctx.font->w, 1,
	       	                    0xFFFF);
	                 last_rotate_time = 0;
	             }
	         }




////读取温湿度
//	  	  AHT20_Read(&temperature, &humidity);
//	  	  // 打印温湿度
//	  	  sprintf(message_uart, "t: %.1f , h: %.1f ", temperature, humidity);
//	  	  sprintf(message_temp, "T %.1f ", temperature);
//	  	  sprintf(message_hum, "H %.1f ", humidity);

//for(uint16_t i=0x0000;i<0x001F;i++){
//LCD_Clear_DMA(i);
//}LCD_WaitForDMA();

//LCD_ShowString(8, 20, message_temp, WHITE,BLACK, &afont12x8);
//LCD_ShowString(64, 20, message_hum, WHITE,BLACK, &afont12x8);
//LCD_ShowStringBG_DMA(32,52,"AHT20",BLACK,CYAN,&afont12x8);
//LCD_WaitForDMA();
//
//
//for(uint16_t i=0x0000;i<0x07E0;i+=0x01<<6){
//LCD_Clear_DMA(i);
//}LCD_WaitForDMA();

//LCD_ShowString(8, 20, message_temp, WHITE,BLACK, &afont12x8);
//LCD_ShowString(64, 20, message_hum, WHITE,BLACK, &afont12x8);
//LCD_ShowString(32,52,"AHT20",BLACK,CYAN,&afont12x8);

//for(uint16_t i=0x0000;i<0xF800;i+=0x01<<11){
//LCD_Clear_DMA(i);
//}LCD_WaitForDMA();

//LCD_ShowStringBG_DMA(8, 20, message_temp, WHITE,BLACK, &afont12x8);
//LCD_ShowStringBG_DMA(64, 20, message_hum, WHITE,BLACK, &afont12x8);
//LCD_ShowStringBG_DMA(32,52,"AHT20",BLACK,CYAN,&afont12x8);

//if(MPU6050_IsDataReady()) {
//		  MPU6050_ProcessData();
//		  MPU6050_StartDMARead();  // 启动下一次读取
//		  angle1 = MPU6050_GetAngle1();
//		  angle2 = MPU6050_GetAngle2();
//	  }

//	  // 按需更新显示（比如50ms一次）
//	  if(HAL_GetTick() - last_display1 >= 50) {
////	    	  HAL_UART_Transmit(&huart2, (uint8_t*)display_buf, strlen(display_buf), 100);
////		  MPU6050_FormatAngle(display_buf, sizeof(display_buf));
////		  OLED_PrintString(0, 0, display_buf, &font16x16, 0);
////		  OLED_ShowFrame();
//		  static int debug_counter = 0;
//		      debug_counter++;
//
//		      // 显示计数器，确认程序执行到此处
//		      sprintf(message, "cnt:%d", debug_counter);
////		      LCD_ShowString(32, 65, "          ", GREEN, BLACK, &afont12x8);
//
//		      LCD_ShowString(32, 65, message, GREEN, BLACK, &afont12x8);
//		  sprintf(message,"a, %.3f",angle1);
//		  LCD_ShowString(32,80,message,BLACK,CYAN,&afont12x8);
//		  LCD_ShowString(32,40,"a",BLACK,CYAN,&afont12x8);
////		  MPU6050_SetServoAngleAuto(servo1, angle1);
//
//		  last_display1 = HAL_GetTick();
//	  }






    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
