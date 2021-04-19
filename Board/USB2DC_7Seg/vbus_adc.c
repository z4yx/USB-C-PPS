#include "bsp_headers.h"
#include "common.h"
#include "leds.h"
#include "stm32g0xx_hal.h"

#define VBUS_ADC hadc1
#define ADC_DMA hdma_adc1
#define ADC_CONVERTED_DATA_BUFFER_SIZE 3

ADC_HandleTypeDef VBUS_ADC;
DMA_HandleTypeDef ADC_DMA;
uint8_t ubADCInitSuccess;
/* Variables for ADC conversion data */
__IO uint16_t
    aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]; /* ADC group regular
                                                           conversion data
                                                           (array of data) */
/* Variables for ADC conversion data computation to physical values */
__IO uint16_t uhADCxConvertedData_VoltageVbus_mVolt =
    0U; /* Value of voltage on GPIO pin (on which is mapped ADC channel)
           calculated from ADC conversion data (unit: mV) */
__IO uint16_t uhADCxConvertedData_VrefInt_mVolt =
    0U; /* Value of internal voltage reference VrefInt calculated from ADC
           conversion data (unit: mV) */
__IO int16_t hADCxConvertedData_Temperature_DegreeCelsius =
    0U; /* Value of temperature calculated from ADC conversion data (unit:
           degree Celsius) */
/*  0: DMA transfer is not completed                                          */
/*  1: DMA transfer is completed                                              */
/*  2: DMA transfer has not yet been started yet (initial state)              */
__IO uint8_t ubDmaTransferStatus =
    2; /* Variable set into DMA interruption callback */

static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);

// overide weak symbols in stm32g0xx_hal_adc.c
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc);

static void Error_Handler(void) { ERR_MSG("ADC error\n"); }

void VBUS_ADC_ResultCalculation(void) {
  // DBG_MSG("aADCxConvertedData: %hu %hu %hu\n", aADCxConvertedData[0],
  //         aADCxConvertedData[1], aADCxConvertedData[2]);
  /* Computation of ADC conversions raw data to physical values           */
  /* using LL ADC driver helper macro.                                    */
  /* Note: ADC results are transferred into array "aADCxConvertedData"  */
  /*       in the order of their rank in ADC sequencer.                   */
  uint32_t vdda = __LL_ADC_CALC_VREFANALOG_VOLTAGE(aADCxConvertedData[1],
                                                   LL_ADC_RESOLUTION_12B);
  // DBG_MSG("vdda=%lu cal=%lu\n", vdda, (uint32_t)(*VREFINT_CAL_ADDR));
  uint32_t vbus_smp = __LL_ADC_CALC_DATA_TO_VOLTAGE(vdda, aADCxConvertedData[2],
                                                    LL_ADC_RESOLUTION_12B);
  uhADCxConvertedData_VoltageVbus_mVolt =
      (uint16_t)(vbus_smp * 118 / 18); // voltage divider on PCB
  hADCxConvertedData_Temperature_DegreeCelsius = __LL_ADC_CALC_TEMPERATURE(
      vdda, aADCxConvertedData[0], LL_ADC_RESOLUTION_12B);
  uhADCxConvertedData_VrefInt_mVolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(
      vdda, aADCxConvertedData[1], LL_ADC_RESOLUTION_12B);
  // DBG_MSG("Vbus/Temp/Vref %hu %hd %hu\n", uhADCxConvertedData_VoltageVbus_mVolt,
  //         hADCxConvertedData_Temperature_DegreeCelsius,
  //         uhADCxConvertedData_VrefInt_mVolt);
}

void BSP_PWR_VBUSDeInit(uint8_t PortNum) {
  HAL_ADC_DeInit(&VBUS_ADC);
  ubADCInitSuccess = 0;
}
void BSP_PWR_VBUSInit(uint8_t PortNum) {
  if (ubADCInitSuccess)
    return;

  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }

  DBG_MSG("Init Vbus ADC\n");
  MX_DMA_Init();
  MX_ADC1_Init();

  DBG_MSG("ADC calibration\n");
  /* Run the ADC calibration */
  if (HAL_ADCEx_Calibration_Start(&VBUS_ADC) != HAL_OK) {
    /* Calibration Error */
    Error_Handler();
    HAL_ADC_DeInit(&VBUS_ADC);
    return;
  }
  ubADCInitSuccess = 1;

  DBG_MSG("Testing ADC\n");
  for (int i = 0; i < 3; i++) {
    BSP_PWR_VBUSGetVoltage(0);
    // DBG_MSG("ADCx->CFGR1=%x CHSELR=%x\n", hadc1.Instance->CFGR1, hadc1.Instance->CHSELR);
    DBG_MSG("Vbus/Temp/Vref %hu %hd %hu\n",
            uhADCxConvertedData_VoltageVbus_mVolt,
            hADCxConvertedData_Temperature_DegreeCelsius,
            uhADCxConvertedData_VrefInt_mVolt);
  }
}
uint32_t BSP_PWR_VBUSGetVoltage(uint8_t PortNum) {

  //   DBG_MSG("ADC Start DMA\n");
  /* Start ADC group regular conversion with DMA */
  if (HAL_ADC_Start_DMA(&VBUS_ADC, (uint32_t *)aADCxConvertedData,
                        ADC_CONVERTED_DATA_BUFFER_SIZE) != HAL_OK) {
    /* ADC conversion start error */
    Error_Handler();
    HAL_ADC_DeInit(&VBUS_ADC);
    return 0;
  }
  //   DBG_MSG("ADCx->CR=%x ISR=%x\n", hadc1.Instance->CR, hadc1.Instance->ISR);

  /* Wait for ADC conversion and DMA transfer completion (update of variable
   * ubDmaTransferStatus) */
  //   HAL_Delay(1);

  /* Check whether ADC has converted all ranks of the sequence */
  for (int timeout = 0; timeout < 0xfffff && ubDmaTransferStatus != 1;
       timeout++)
    ;
  //   DBG_MSG("ADCx->CR=%x ISR=%x\n", hadc1.Instance->CR, hadc1.Instance->ISR);
  //   DBG_MSG("ubDmaTransferStatus=%u\n", ubDmaTransferStatus);
  if (ubDmaTransferStatus != 1) {
    Error_Handler();
    return 0;
  }
  VBUS_ADC_ResultCalculation();
  /* Update status variable of DMA transfer */
  ubDmaTransferStatus = 0;
  return uhADCxConvertedData_VoltageVbus_mVolt;
}

/**
 * @brief  Conversion complete callback in non blocking mode
 * @param  hadc: ADC handle
 * @note   This example shows a simple way to report end of conversion
 *         and get conversion result. You can add your own implementation.
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  /* Update status variable of DMA transfer */
  ubDmaTransferStatus = 1;

  /* Set LED depending on DMA transfer status */
  /* - Turn-on if DMA transfer is completed */
  /* - Turn-off if DMA transfer is not completed */
  //   BSP_LED_On(LED1);
}

/**
 * @brief  Conversion DMA half-transfer callback in non blocking mode
 * @note   This example shows a simple way to report end of conversion
 *         and get conversion result. You can add your own implementation.
 * @retval None
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  /* Set LED depending on DMA transfer status */
  /* - Turn-on if DMA transfer is completed */
  /* - Turn-off if DMA transfer is not completed */
  //   BSP_LED_Off(LED1);
}

/**
 * @brief  ADC error callback in non blocking mode
 *        (ADC conversion with interruption or transfer by DMA)
 * @param  hadc: ADC handle
 * @retval None
 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
  /* In case of ADC error, call main error handler */
  Error_Handler();
}

/**
 * @brief This function handles DMA1 channel 1 interrupt.
 */
void DMA1_Channel1_IRQHandler(void) {
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&ADC_DMA);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
 * @brief This function handles ADC1, COMP1 and COMP2 interrupts (COMP
 * interrupts through EXTI lines 17 and 18).
 */
void ADC1_COMP_IRQHandler(void) {
  /* USER CODE BEGIN ADC1_COMP_IRQn 0 */

  /* USER CODE END ADC1_COMP_IRQn 0 */
  HAL_ADC_IRQHandler(&VBUS_ADC);
  /* USER CODE BEGIN ADC1_COMP_IRQn 1 */

  /* USER CODE END ADC1_COMP_IRQn 1 */
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (hadc->Instance == ADC1) {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB12     ------> ADC1_IN16
    */
    GPIO_InitStruct.Pin = VBUS_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VBUS_ADC_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
    /* USER CODE BEGIN ADC1_MspInit 1 */
    DBG_MSG("HAL_ADC_MspInit done\n");
    /* USER CODE END ADC1_MspInit 1 */
  }
}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == ADC1) {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB12     ------> ADC1_IN16
    */
    HAL_GPIO_DeInit(VBUS_ADC_GPIO_Port, VBUS_ADC_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);

    /* ADC1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_SEQ_FIXED;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = ADC_CONVERTED_DATA_BUFFER_SIZE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_19CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
    return;
  }
  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
    return;
  }
  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
    return;
  }
  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_16;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
    return;
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}
