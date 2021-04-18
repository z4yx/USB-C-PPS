#include "bsp_headers.h"

#define DBG_UART huart1

UART_HandleTypeDef DBG_UART;
void DEBUG_USART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* Peripheral clock enable */
  __HAL_RCC_USART1_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();
  /**USART1 GPIO Configuration
  PB6     ------> USART1_TX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  DBG_UART.Instance = USART1;
  DBG_UART.Init.BaudRate = 115200;
  DBG_UART.Init.WordLength = UART_WORDLENGTH_8B;
  DBG_UART.Init.StopBits = UART_STOPBITS_1;
  DBG_UART.Init.Parity = UART_PARITY_NONE;
  DBG_UART.Init.Mode = UART_MODE_TX;
  DBG_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  DBG_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  DBG_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  DBG_UART.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  DBG_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&DBG_UART) != HAL_OK) {
  }
}

int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void _ttywrch(int ch);

/* Retargeting functions for gcc-arm-embedded */

int _write (int fd, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
  int i;
  for (i = 0; i < len; ++i)
  {
    _ttywrch(ptr[i]);
  }
  return len;
}

int _read (int fd, char *ptr, int len)
{
  /* Read "len" of char to "ptr" from file id "fd"
   * Return number of char read.
   * Need implementing with UART here. */
  return 0;
}

void _ttywrch(int ch) {
  /* Write one char "ch" to the default console
   * Need implementing with UART here. */
  HAL_UART_Transmit(&DBG_UART, (uint8_t*)&ch, 1, 1000);
}