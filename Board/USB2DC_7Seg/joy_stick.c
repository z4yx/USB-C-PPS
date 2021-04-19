#include "bsp_headers.h"

void JoyStick_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : SW_R_Pin */
  GPIO_InitStruct.Pin = SW_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW_R_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SW_D_Pin SW_U_Pin SW_P_Pin SW_L_Pin */
  GPIO_InitStruct.Pin = SW_D_Pin | SW_U_Pin | SW_P_Pin | SW_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

JOYState_TypeDef JoyStick_GetState(void) {
  const uint32_t pins[4] = {SW_D_Pin, SW_U_Pin, SW_P_Pin, SW_L_Pin};
  const JOYState_TypeDef res[4] = {JOY_DOWN, JOY_UP, JOY_SEL, JOY_LEFT};
  for (int i = 0; i < 4; i++)
    if (HAL_GPIO_ReadPin(SW_D_GPIO_Port, pins[i]) == 0)
      return res[i];
  if (HAL_GPIO_ReadPin(SW_R_GPIO_Port, SW_R_Pin) == 0)
    return JOY_RIGHT;

  return JOY_NONE;
}
