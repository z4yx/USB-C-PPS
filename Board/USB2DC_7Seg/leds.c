#include "bsp_headers.h"
#include "common.h"

void LED_7Seg_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SEG_DP_Pin|SEG_C_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SEG_B_Pin|SEG_F_Pin
                          |SEG_A_Pin|SEG_E_Pin|SEG_G_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIG1_Pin|DIG2_Pin|DIG3_Pin|OUT_EN_Pin
                          |SEG_D_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : SEG_B_Pin SEG_F_Pin SEG_A_Pin SEG_E_Pin
                           SEG_G_Pin DIG4_Pin */
  GPIO_InitStruct.Pin = SEG_B_Pin|SEG_F_Pin|SEG_A_Pin|SEG_E_Pin
                          |SEG_G_Pin|DIG4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DIG1_Pin DIG2_Pin DIG3_Pin OUT_EN_Pin
                           SEG_D_Pin */
  GPIO_InitStruct.Pin = DIG1_Pin|DIG2_Pin|DIG3_Pin|OUT_EN_Pin
                          |SEG_D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_DP_Pin SEG_C_Pin */
  GPIO_InitStruct.Pin = SEG_DP_Pin|SEG_C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void LED_OutEnable(uint8_t enable)
{
    HAL_GPIO_WritePin(OUT_EN_GPIO_Port, OUT_EN_Pin, enable);
}
