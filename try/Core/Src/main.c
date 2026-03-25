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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_tim2_up;
DMA_HandleTypeDef hdma_tim4_up_kick;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM6_Init(void);

/* USER CODE BEGIN PFP */
static void VGA_FillSolid(uint8_t color);
static void VGA_FillVerticalStripes(uint8_t c0, uint8_t c1, uint16_t stripe_w);
static void VGA_FillCheckerboard(uint8_t c0, uint8_t c1, uint16_t cell);
static void VGA_FillMovingBar(uint8_t bg, uint8_t fg, uint16_t x0, uint16_t w);
static void VGA_FillGradient(void);
static void VGA_InitPatternBuffer(void);
static void VGA_InitPixelPath(void);
static void VGA_InitDMAAndTimers(void);
static void VGA_StartPixelPath(void);
static void VGA_StartDMAAndTimers(void);
void delay_cycles(uint32_t cycles);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define VGA_R_PORT GPIOA
#define VGA_R_PIN  GPIO_PIN_5

#define VGA_G_PORT GPIOA
#define VGA_G_PIN  GPIO_PIN_7

#define VGA_B_PORT GPIOD
#define VGA_B_PIN  GPIO_PIN_15

#define VGA_HS_PORT GPIOB
#define VGA_HS_PIN  GPIO_PIN_9

#define VGA_VS_PORT GPIOB
#define VGA_VS_PIN  GPIO_PIN_8

#define HS_LOW()    (GPIOB->BSRR = (uint32_t)GPIO_PIN_9 << 16)
#define HS_HIGH()   (GPIOB->BSRR = GPIO_PIN_9)
#define VS_LOW()    (GPIOB->BSRR = (uint32_t)GPIO_PIN_8 << 16)
#define VS_HIGH()   (GPIOB->BSRR = GPIO_PIN_8)

#define R_LOW()     (GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16)
#define R_HIGH()    (GPIOA->BSRR = GPIO_PIN_5)
#define G_LOW()     (GPIOA->BSRR = (uint32_t)GPIO_PIN_7 << 16)
#define G_HIGH()    (GPIOA->BSRR = GPIO_PIN_7)
#define B_LOW()     (GPIOD->BSRR = (uint32_t)GPIO_PIN_15 << 16)
#define B_HIGH()    (GPIOD->BSRR = GPIO_PIN_15)

#define H_SYNC       (128U / 8U)
#define H_BACKPORCH  (88U / 8U)
#define H_VISIBLE    (800U / 8U)
#define H_FRONTPORCH (40U / 8U)
#define H_PIXEL      (H_SYNC + H_BACKPORCH + H_VISIBLE + H_FRONTPORCH)
#define H_ACTUAL     H_VISIBLE

#define V_SYNC_LINES      4U
#define V_BACKPORCH_LINES 23U
#define V_VISIBLE_LINES   600U
#define V_FRONTPORCH_LINES 1U
#define V_TOTAL_LINES (V_SYNC_LINES + V_BACKPORCH_LINES + V_VISIBLE_LINES + V_FRONTPORCH_LINES)

#define V_VISIBLE    (V_VISIBLE_LINES / 8U)
#define V_VISIBLE_LINE_START (V_SYNC_LINES + V_BACKPORCH_LINES)
#define V_VISIBLE_LINE_END   (V_VISIBLE_LINE_START + V_VISIBLE_LINES)
#define V_VISIBLE_START (V_VISIBLE_LINE_START / 8U)
#define V_VISIBLE_END   (V_VISIBLE_START + V_VISIBLE)
#define V_PIXEL      ((V_TOTAL_LINES + 7U) / 8U)

#define H_VISIBLE_START (H_SYNC + H_BACKPORCH)
#define H_VISIBLE_END   (H_VISIBLE_START + H_VISIBLE)

#define RESOLUTION   (H_PIXEL * V_PIXEL)

#define TIM1_PSC     0U
#define TIM1_ARR     3167U   /* 120MHz / 3168 = 37.8788kHz (1056 * 60Hz family) */
#define TIM2_PSC     0U
#define TIM2_ARR     23U     /* 120MHz / 24 = 5.0MHz (= 40MHz / 8) */
#define TIM3_PSC     31U
#define TIM3_ARR     62171U  /* 120MHz / 32 / 62172 = 60.3169Hz = 37.8788kHz / 628 */

#define PIXEL_CYCLES 24U
#define HSYNC_CYCLES (H_SYNC * PIXEL_CYCLES)
#define VSYNC_CYCLES (V_SYNC_LINES * (TIM1_ARR + 1U))
#define H_PHASE_PIX_OFFSET 0
#define V_PHASE_LINE_OFFSET 0
#define DMA_PHASE_DELAY_CYCLES (((H_SYNC + H_BACKPORCH + H_PHASE_PIX_OFFSET) * PIXEL_CYCLES))

#define PIXEL_PATH_GPIO_ODR    0U
#define PIXEL_PATH_SPI3_SERIAL 1U
#define PIXEL_PATH_OCTOSPI1    2U
/*
 * Safe default for monitor lock:
 * keep Stage 4 code present, but run legacy GPIO bus until OCTOSPI pin mux
 * (clock + data lanes) is fully validated on hardware.
 */
#define PIXEL_PATH_MODE PIXEL_PATH_GPIO_ODR

#if (PIXEL_PATH_MODE == PIXEL_PATH_SPI3_SERIAL) || (PIXEL_PATH_MODE == PIXEL_PATH_OCTOSPI1)
/* Serialized monochrome-like byte patterns on serial pixel lane */
#define COLOR_BLACK  0x00U
#define COLOR_RED    0xF0U
#define COLOR_GREEN  0x0FU
#else
#define COLOR_BLACK  0x00U
#define COLOR_RED    ((uint8_t)GPIO_PIN_5)
#define COLOR_GREEN  ((uint8_t)GPIO_PIN_7)
#endif

#define SQUARE_SIZE  36U

#define TEST_SOLID      0U
#define TEST_STRIPES    1U
#define TEST_CHECKER    2U
#define TEST_MOVING_BAR 3U
#define TEST_GRADIENT   4U
#define TEST_MODE       TEST_GRADIENT
#define CENTER_SQUARE_ENABLE 0U

volatile uint16_t current_line = 0;
static uint8_t frame_buffer[RESOLUTION];
/* CH1: hardware line alignment by resetting TIM2->CNT at each TIM4 update */
static uint32_t tim2_cnt_reset_word = 0U;
/* CH2 base pointer currently active in circular stream */
static const uint8_t *active_line_src = 0;
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
  /* USER CODE BEGIN 2 */

#if PIXEL_PATH_MODE == PIXEL_PATH_GPIO_ODR
  R_LOW();
  G_LOW();
#endif
  B_LOW();
  VS_HIGH();

  /* 打开 DWT cycle counter */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;

  VGA_InitPixelPath();
  VGA_InitPatternBuffer();
  VGA_InitDMAAndTimers();
  VGA_StartDMAAndTimers();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {


  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 15;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = TIM1_PSC;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = TIM1_ARR;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 119;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 31;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE8 PE9 PE10
                           PE11 PE12 PE13 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE14 PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM1_COMP1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PG7 PG8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 PC10 PC11
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD3 PD4 PD5 PD6 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 (TIM4_CH4 for hardware HSYNC) */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
#if PIXEL_PATH_MODE == PIXEL_PATH_OCTOSPI1
  /*
   * Stage 4 experimental mapping:
   * reuse PB5 as OCTOSPIM Port1 lane.
   * Exact board-level OCTOSPI clock/data pinout still needs final tune in CubeMX.
   */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void VGA_FillSolid(uint8_t color)
{
  for (uint16_t v = 0; v < V_PIXEL; ++v)
  {
    for (uint16_t h = 0; h < H_PIXEL; ++h)
    {
      if ((v >= V_VISIBLE_START) && (v < V_VISIBLE_END) &&
          (h >= H_VISIBLE_START) && (h < H_VISIBLE_END))
      {
        frame_buffer[(v * H_PIXEL) + h] = color;
      }
      else
      {
        frame_buffer[(v * H_PIXEL) + h] = COLOR_BLACK;
      }
    }
  }
}

static void VGA_FillVerticalStripes(uint8_t c0, uint8_t c1, uint16_t stripe_w)
{
  if (stripe_w == 0U)
  {
    stripe_w = 1U;
  }

  for (uint16_t v = 0; v < V_PIXEL; ++v)
  {
    for (uint16_t h = 0; h < H_PIXEL; ++h)
    {
      if ((v >= V_VISIBLE_START) && (v < V_VISIBLE_END) &&
          (h >= H_VISIBLE_START) && (h < H_VISIBLE_END))
      {
        uint16_t x = (uint16_t)(h - H_VISIBLE_START);
        uint8_t color = (((x / stripe_w) & 1U) == 0U) ? c0 : c1;
        frame_buffer[(v * H_PIXEL) + h] = color;
      }
      else
      {
        frame_buffer[(v * H_PIXEL) + h] = COLOR_BLACK;
      }
    }
  }
}

static void VGA_FillCheckerboard(uint8_t c0, uint8_t c1, uint16_t cell)
{
  if (cell == 0U)
  {
    cell = 1U;
  }

  for (uint16_t v = 0; v < V_PIXEL; ++v)
  {
    for (uint16_t h = 0; h < H_PIXEL; ++h)
    {
      if ((v >= V_VISIBLE_START) && (v < V_VISIBLE_END) &&
          (h >= H_VISIBLE_START) && (h < H_VISIBLE_END))
      {
        uint16_t x = (uint16_t)(h - H_VISIBLE_START);
        uint16_t y = (uint16_t)(v - V_VISIBLE_START);
        uint8_t color = ((((x / cell) + (y / cell)) & 1U) == 0U) ? c0 : c1;
        frame_buffer[(v * H_PIXEL) + h] = color;
      }
      else
      {
        frame_buffer[(v * H_PIXEL) + h] = COLOR_BLACK;
      }
    }
  }
}

static void VGA_FillMovingBar(uint8_t bg, uint8_t fg, uint16_t x0, uint16_t w)
{
  uint16_t x1 = (uint16_t)(x0 + w);
  if (x1 > H_VISIBLE)
  {
    x1 = H_VISIBLE;
  }

  VGA_FillSolid(bg);

  for (uint16_t v = V_VISIBLE_START; v < V_VISIBLE_END; ++v)
  {
    for (uint16_t h = (uint16_t)(H_VISIBLE_START + x0);
         h < (uint16_t)(H_VISIBLE_START + x1); ++h)
    {
      frame_buffer[(v * H_PIXEL) + h] = fg;
    }
  }
}

static void VGA_FillGradient(void)
{
  static const uint8_t bayer4x4[4][4] = {
    { 0U,  8U,  2U, 10U},
    {12U,  4U, 14U,  6U},
    { 3U, 11U,  1U,  9U},
    {15U,  7U, 13U,  5U}
  };

  for (uint16_t v = 0; v < V_PIXEL; ++v)
  {
    for (uint16_t h = 0; h < H_PIXEL; ++h)
    {
      if ((v >= V_VISIBLE_START) && (v < V_VISIBLE_END) &&
          (h >= H_VISIBLE_START) && (h < H_VISIBLE_END))
      {
        uint16_t x = (uint16_t)(h - H_VISIBLE_START);
        uint16_t y = (uint16_t)(v - V_VISIBLE_START);
        uint32_t r_level = (H_VISIBLE > 1U) ? ((uint32_t)x * 255U) / (H_VISIBLE - 1U) : 0U;
        uint32_t g_level = (V_VISIBLE > 1U) ? ((uint32_t)y * 255U) / (V_VISIBLE - 1U) : 0U;
        uint32_t threshold = ((uint32_t)bayer4x4[y & 0x3U][x & 0x3U] * 16U) + 8U;
        uint8_t color = COLOR_BLACK;

        if (r_level > threshold)
        {
          color |= COLOR_RED;
        }
        if (g_level > threshold)
        {
          color |= COLOR_GREEN;
        }

        frame_buffer[(v * H_PIXEL) + h] = color;
      }
      else
      {
        frame_buffer[(v * H_PIXEL) + h] = COLOR_BLACK;
      }
    }
  }
}

static void VGA_InitPatternBuffer(void)
{
  uint16_t sq_x0 = (uint16_t)(H_VISIBLE_START + ((H_VISIBLE - SQUARE_SIZE) / 2U));
  uint16_t sq_y0 = (uint16_t)(V_VISIBLE_START + ((V_VISIBLE - SQUARE_SIZE) / 2U));
  uint16_t sq_x1 = (uint16_t)(sq_x0 + SQUARE_SIZE);
  uint16_t sq_y1 = (uint16_t)(sq_y0 + SQUARE_SIZE);

#if TEST_MODE == TEST_SOLID
  VGA_FillSolid(COLOR_RED);
#elif TEST_MODE == TEST_STRIPES
  VGA_FillVerticalStripes(COLOR_RED, COLOR_GREEN, 2U);
#elif TEST_MODE == TEST_CHECKER
  VGA_FillCheckerboard(COLOR_RED, COLOR_GREEN, 4U);
#elif TEST_MODE == TEST_MOVING_BAR
  VGA_FillMovingBar(COLOR_RED, COLOR_GREEN, (uint16_t)(H_VISIBLE / 3U), SQUARE_SIZE);
#elif TEST_MODE == TEST_GRADIENT
  VGA_FillGradient();
#else
  VGA_FillSolid(COLOR_RED);
#endif

#if CENTER_SQUARE_ENABLE
  for (uint16_t v = sq_y0; v < sq_y1 && v < V_VISIBLE_END; ++v)
  {
    for (uint16_t h = sq_x0; h < sq_x1 && h < H_VISIBLE_END; ++h)
    {
      frame_buffer[(v * H_PIXEL) + h] = COLOR_GREEN;
    }
  }
#endif
}

static void VGA_InitPixelPath(void)
{
#if PIXEL_PATH_MODE == PIXEL_PATH_SPI3_SERIAL
  __HAL_RCC_SPI3_CLK_ENABLE();

  SPI3->CR1 = 0U;
  SPI3->CR2 = 0U;
  SPI3->CRCPR = 7U;
  SPI3->I2SCFGR = 0U;

  /* 8-bit data size, TX FIFO threshold at 8-bit */
  SPI3->CR2 = (7U << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH;

  /*
   * Master, software NSS.
   * BR=000 -> fPCLK/2 (can be tuned later with monitor lock measurements).
   */
  SPI3->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
#elif PIXEL_PATH_MODE == PIXEL_PATH_OCTOSPI1
  __HAL_RCC_OSPIM_CLK_ENABLE();
  __HAL_RCC_OSPI1_CLK_ENABLE();

  /* Route Port1 resources to OCTOSPI1 (CLK + IO low group enabled) */
  OCTOSPIM->PCR[0] = OCTOSPIM_PCR_CLKEN | OCTOSPIM_PCR_IOLEN;
  OCTOSPIM->PCR[1] = 0U;

  OCTOSPI1->CR = 0U;
  OCTOSPI1->FCR = OCTOSPI_FCR_CTEF | OCTOSPI_FCR_CTCF | OCTOSPI_FCR_CSMF | OCTOSPI_FCR_TOF;

  /*
   * Conservative bring-up timing:
   * - prescaler 3 => f_ospi = f_src / 4
   * - minimal chip-select high time
   */
  OCTOSPI1->DCR1 = (1U << OCTOSPI_DCR1_CSHT_Pos);
  OCTOSPI1->DCR2 = (3U << OCTOSPI_DCR2_PRESCALER_Pos);
  OCTOSPI1->DCR3 = 0U;
  OCTOSPI1->TCR = 0U;

  /* 1-line data mode, no instruction/address/alternate phase */
  OCTOSPI1->CCR = OCTOSPI_CCR_DMODE_0;
  OCTOSPI1->IR = 0U;
  OCTOSPI1->ABR = 0U;
  OCTOSPI1->AR = 0U;
  /* Keep stream effectively continuous */
  OCTOSPI1->DLR = 0xFFFFFFFFU;
#endif
}

static void VGA_InitDMAAndTimers(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_TIM4_CLK_ENABLE();
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = TIM1_PSC;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = TIM1_ARR;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = HSYNC_CYCLES;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = TIM2_PSC;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = TIM2_ARR;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = TIM3_PSC;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = TIM3_ARR;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* CH1: one-word line alignment, hardware-triggered by TIM4 update */
  hdma_tim4_up_kick.Instance = DMA1_Channel1;
  hdma_tim4_up_kick.Init.Request = DMA_REQUEST_TIM4_UP;
  hdma_tim4_up_kick.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_tim4_up_kick.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_tim4_up_kick.Init.MemInc = DMA_MINC_DISABLE;
  hdma_tim4_up_kick.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_tim4_up_kick.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_tim4_up_kick.Init.Mode = DMA_CIRCULAR;
  hdma_tim4_up_kick.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  if (HAL_DMA_Init(&hdma_tim4_up_kick) != HAL_OK)
  {
    Error_Handler();
  }

  hdma_tim2_up.Instance = DMA1_Channel2;
  hdma_tim2_up.Init.Request = DMA_REQUEST_TIM2_UP;
  hdma_tim2_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_tim2_up.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_tim2_up.Init.MemInc = DMA_MINC_ENABLE;
  hdma_tim2_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tim2_up.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_tim2_up.Init.Mode = DMA_CIRCULAR;
  hdma_tim2_up.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  if (HAL_DMA_Init(&hdma_tim2_up) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(&htim4, hdma[TIM_DMA_ID_UPDATE], hdma_tim4_up_kick);
  __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_UPDATE], hdma_tim2_up);

  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

static void VGA_StartDMAAndTimers(void)
{
  current_line = (uint16_t)(V_TOTAL_LINES - 1U);
  hdma_tim4_up_kick.Instance->CPAR = (uint32_t)&TIM2->CNT;
  hdma_tim4_up_kick.Instance->CMAR = (uint32_t)&tim2_cnt_reset_word;
  hdma_tim4_up_kick.Instance->CNDTR = 1U;

#if PIXEL_PATH_MODE == PIXEL_PATH_SPI3_SERIAL
  hdma_tim2_up.Instance->CPAR = (uint32_t)&SPI3->DR;
#elif PIXEL_PATH_MODE == PIXEL_PATH_OCTOSPI1
  hdma_tim2_up.Instance->CPAR = (uint32_t)&OCTOSPI1->DR;
#else
  hdma_tim2_up.Instance->CPAR = (uint32_t)&GPIOA->ODR;
#endif
  active_line_src = frame_buffer;
  hdma_tim2_up.Instance->CMAR = (uint32_t)active_line_src;
  hdma_tim2_up.Instance->CNDTR = H_PIXEL;

  VGA_StartPixelPath();

  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);

  hdma_tim4_up_kick.Instance->CCR &= ~DMA_CCR_EN;
  hdma_tim4_up_kick.Instance->CCR |= DMA_CCR_EN;
  hdma_tim2_up.Instance->CCR &= ~DMA_CCR_EN;
  hdma_tim2_up.Instance->CCR |= DMA_CCR_EN;

  if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  TIM2->CNT = 0U;
  TIM2->CR1 |= TIM_CR1_CEN;

  __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
}

static void VGA_StartPixelPath(void)
{
#if PIXEL_PATH_MODE == PIXEL_PATH_SPI3_SERIAL
  SPI3->CR2 |= SPI_CR2_TXDMAEN;
  SPI3->CR1 |= SPI_CR1_SPE;
#elif PIXEL_PATH_MODE == PIXEL_PATH_OCTOSPI1
  OCTOSPI1->FCR = OCTOSPI_FCR_CTEF | OCTOSPI_FCR_CTCF | OCTOSPI_FCR_CSMF | OCTOSPI_FCR_TOF;
  /* Indirect write mode + DMA + enable */
  OCTOSPI1->CR = (1U << OCTOSPI_CR_FTHRES_Pos) |
                 OCTOSPI_CR_DMAEN |
                 OCTOSPI_CR_FMODE_0 |
                 OCTOSPI_CR_EN;
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    uint16_t row;
    int32_t line;
    const uint8_t *src;

    /* Blue is on GPIOD and not fed by the GPIOA DMA stream */
    B_LOW();

    current_line++;
    if (current_line >= V_TOTAL_LINES)
    {
      current_line = 0U;
    }

    if (current_line == 0U)
    {
      VS_LOW();
    }
    else if (current_line == V_SYNC_LINES)
    {
      VS_HIGH();
    }

    line = (int32_t)current_line + V_PHASE_LINE_OFFSET;
    while (line < 0)
    {
      line += (int32_t)V_TOTAL_LINES;
    }
    while (line >= (int32_t)V_TOTAL_LINES)
    {
      line -= (int32_t)V_TOTAL_LINES;
    }

    if ((line >= (int32_t)V_VISIBLE_LINE_START) &&
        (line < (int32_t)V_VISIBLE_LINE_END))
    {
      uint16_t visible_line = (uint16_t)(line - (int32_t)V_VISIBLE_LINE_START);
      row = (uint16_t)(V_VISIBLE_START + (visible_line >> 3));
      if (row >= V_PIXEL)
      {
        row = 0U;
      }
      src = frame_buffer + ((uint32_t)row * H_PIXEL);
    }
    else
    {
      src = frame_buffer;
    }

    /* CH2: only update base pointer when it changes */
    if (src != active_line_src)
    {
      active_line_src = src;
      hdma_tim2_up.Instance->CMAR = (uint32_t)src;
    }
  }
}



void delay_cycles(uint32_t cycles)
{
  uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles)
  {
  }
}


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
