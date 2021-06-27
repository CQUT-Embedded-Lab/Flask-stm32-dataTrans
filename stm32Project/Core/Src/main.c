/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user/oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t ipaddr[] = ">xxx.xxx.x.xxx";
uint8_t line_num = 2;
uint8_t flag = 0;
int8_t str_pos = -1;
char temp_str[100];    // 临时子串
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void OLED_preFlash(){
  OLED_Clear();
  for(uint8_t i=1; i<8; i++){
    OLED_ShowString(1,i,"***************",8);
  }
  OLED_Clear();
}
void OLED_flash(uint8_t temp[], uint8_t temp_len){
  if(temp_len>14){
    temp_len=14;
  }
  OLED_ShowString(1, line_num, (uint8_t *)"               ", 8);
  OLED_ShowString_cnt(1,line_num,temp,8, temp_len);
  if(line_num++==7){
    line_num=2;
  }
}
void OLED_formatFlash(){
  OLED_ShowString(1,3,"clit:",8);
  OLED_ShowString(1,5,"size:",8);
  OLED_ShowString(1,7,"info:",8);
}
void OLED_infoFlash(uint8_t p1[], uint8_t p2[], uint8_t p3[], uint8_t p3_len){
  OLED_ShowString_cnt(50,3,p1,8, 1);
  OLED_ShowString_cnt(50,5,p2,8, 1);
  OLED_ShowString_cnt(50,7,p3,8, p3_len);
}
void ReadStrUnit(char *str,char *temp_str,int idx,int len)  // 从母串中获取与子串长度相等的临时子串
{
  int index = 0;
  for(index; index < len; index++)
  {
    temp_str[index] = str[idx+index];
  }
  temp_str[index] = '\0';
}

int GetSubStrPos(char *str1,char *str2){
  int idx = 0;
  int len1 = strlen(str1);
  int len2 = strlen(str2);

  if( len1 < len2)
  {
    printf("error 1 \n"); // 子串比母串长
    return -1;
  }

  while(1)
  {
    ReadStrUnit(str1,temp_str,idx,len2);    // 不断获取的从 母串�?? idx 位置处更新临时子�??
    if(strcmp(str2,temp_str)==0)break;      // 若临时子串和子串�??致，结束循环
    idx++;                                  // 改变从母串中取临时子串的位置
    if(idx>=len1)return -1;                 // �?? idx 已经超出母串长度，说明母串不包含该子�??
  }

  return idx;    // 返回子串第一个字符在母串中的位置
}
void user_API(uint8_t temp[], uint8_t temp_len){
  if(flag<6){
    OLED_flash(temp, temp_len);
  }
  if(flag == 0){
    str_pos = GetSubStrPos(temp, "WIFI GOT IP");
    if(str_pos != -1){
      DMA_Usart1_Send("AT+CIFSR\r\n", 10);
      flag=1;
    }
  } else if(flag == 1){
    str_pos = GetSubStrPos(temp, "+CIFSR:STAIP,");
    if(str_pos != -1){
      for(uint8_t i=0; i<13; i++){
        ipaddr[i+1] = temp[str_pos+14+i];
      }
      OLED_ShowString_cnt(1,1,ipaddr,8, 14);

      DMA_Usart1_Send("AT+CIPMUX=1\r\n", 13);
      flag=2;
    }
  } else if(flag == 2){
    str_pos = GetSubStrPos(temp, "OK");
    if(str_pos!=-1){
      DMA_Usart1_Send("AT+CIPSERVER=1\r\n", 16);
      flag=3;
    }
  } else if(flag == 3){
    str_pos = GetSubStrPos(temp, "OK");
    if(str_pos!=-1){
      DMA_Usart1_Send("AT+CIPSTO=0\r\n", 13);
      flag=4;
    }
  } else if(flag == 4){
    str_pos = GetSubStrPos(temp, "OK");
    if(str_pos!=-1){
      DMA_Usart1_Send("AT\r\n", 4);
      flag=5;
    }
  } else if(flag == 5){
    str_pos = GetSubStrPos(temp, "OK");
    if(str_pos!=-1){
      OLED_Clear();
      OLED_ShowString_cnt(1,1,ipaddr,8, 14);
      OLED_formatFlash();
      flag = 6;
    }
  } else if(flag == 6){
    str_pos = GetSubStrPos(temp, "+IPD");
    if(str_pos!=-1){
      uint8_t p1[] = "0";
      uint8_t p2[] = "3";
      uint8_t p3[10];
      p1[0] = temp[str_pos+5];
      p2[0] = temp[str_pos+7];
      uint8_t p3_len = (uint8_t)(p2[0]-'0');
      for(uint8_t i=0; i<p3_len; i++){
        p3[i] = temp[str_pos+9+i];
      }

      OLED_infoFlash(p1, p2, p3, p3_len);
    }
  }
  //  DMA_Usart1_Send(rx1_buffer, rx1_len);
}

void Usart1_Handle()     //USART2对接收的�?????帧数据进行处�?????
{
  user_API(rx1_buffer, rx1_len);
  rx1_len = 0;//清除计数
  rec1_end_flag = 0;//清除接收结束标志�?????
  memset(rx1_buffer,0,rx1_len);
  HAL_UART_Receive_DMA(&huart1,rx1_buffer,BUFFER_SIZE);//重新打开DMA接收
}

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
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  OLED_Init();
  OLED_preFlash();
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart1,rx1_buffer,BUFFER_SIZE);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(rec1_end_flag)  //判断是否接收�??????1帧数�??????
    {
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      Usart1_Handle();
    }
    if(!HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) { //DEBUG
      while(!HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin));
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); //翻转LED
      OLED_flash("AT+CIFSR\r\n", 8);
      DMA_Usart1_Send("AT+CIFSR\r\n", 10);
      flag = 1;
    }

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
