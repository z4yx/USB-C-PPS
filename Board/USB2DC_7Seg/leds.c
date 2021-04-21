#include "bsp_headers.h"
#include "common.h"

const GPIO_TypeDef *SEG_Neg_Port[] = {
    DIG1_GPIO_Port,
    DIG2_GPIO_Port,
    DIG3_GPIO_Port,
    DIG4_GPIO_Port,
};
const uint32_t SEG_Neg_Pin[] = {
    DIG1_Pin,
    DIG2_Pin,
    DIG3_Pin,
    DIG4_Pin,
};
const GPIO_TypeDef *SEG_Pos_Port[] = {
    SEG_A_GPIO_Port, SEG_B_GPIO_Port, SEG_C_GPIO_Port, SEG_D_GPIO_Port,
    SEG_E_GPIO_Port, SEG_F_GPIO_Port, SEG_G_GPIO_Port,
};
const uint32_t SEG_Pos_Pin[] = {
    SEG_A_Pin, SEG_B_Pin, SEG_C_Pin, SEG_D_Pin, SEG_E_Pin, SEG_F_Pin, SEG_G_Pin,
};
const uint8_t decode_tab[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,
};
const GPIO_TypeDef *port_num2ptr[] = {GPIOA, GPIOB, GPIOC};

uint16_t fast_refresh_decode[3][16];
uint16_t fast_refresh_mask[3];

static uint8_t disp_digits[4]={5,6,7,8};

static void disp_digit(uint8_t digit) {
#if 0
  uint8_t decoded = decode_tab[digit];
  for (int i = 0; i < 7; i++) {
    HAL_GPIO_WritePin(SEG_Pos_Port[i], SEG_Pos_Pin[i], decoded & 1);
    decoded >>= 1;
  }
#else
  for (int port = 0; port < 3; port++) {
    HAL_GPIO_WritePin(port_num2ptr[port], fast_refresh_mask[port], 0);
    HAL_GPIO_WritePin(port_num2ptr[port], fast_refresh_decode[port][digit], 1);
  }
#endif
}

static int port_ptr2num(GPIO_TypeDef *port) {
  switch ((uintptr_t)port) {
  case (uintptr_t)GPIOA:
    return 0;
  case (uintptr_t)GPIOB:
    return 1;
  case (uintptr_t)GPIOC:
    return 2;
  }
  return 0;
}

static void init_fast_refresh() {
  for (int i = 0; i < 7; i++) {
    int port = port_ptr2num(SEG_Pos_Port[i]);
    fast_refresh_mask[port] |= SEG_Pos_Pin[i];

    uint8_t seg_bit = 1u << i;
    for (int digit = 0; digit < 16; digit++) {
      if ((decode_tab[digit] & seg_bit) != 0)
        fast_refresh_decode[port][digit] |= SEG_Pos_Pin[i];
    }
  }
}

void LED_7Seg_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SEG_DP_Pin | SEG_C_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA,
                    SEG_B_Pin | SEG_F_Pin | SEG_A_Pin | SEG_E_Pin | SEG_G_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB,
                    DIG1_Pin | DIG2_Pin | DIG3_Pin | OUT_EN_Pin | SEG_D_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : SEG_B_Pin SEG_F_Pin SEG_A_Pin SEG_E_Pin
                           SEG_G_Pin DIG4_Pin */
  GPIO_InitStruct.Pin =
      SEG_B_Pin | SEG_F_Pin | SEG_A_Pin | SEG_E_Pin | SEG_G_Pin | DIG4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DIG1_Pin DIG2_Pin DIG3_Pin OUT_EN_Pin
                           SEG_D_Pin */
  GPIO_InitStruct.Pin = DIG1_Pin | DIG2_Pin | DIG3_Pin | OUT_EN_Pin | SEG_D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_DP_Pin SEG_C_Pin */
  GPIO_InitStruct.Pin = SEG_DP_Pin | SEG_C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  init_fast_refresh();
}

void LED_OutEnable(uint8_t enable) {
  HAL_GPIO_WritePin(OUT_EN_GPIO_Port, OUT_EN_Pin, enable);
}

void LED_ToggleOutEnable() { HAL_GPIO_TogglePin(OUT_EN_GPIO_Port, OUT_EN_Pin); }

void Seg7_Refresh() {
  static uint32_t cnt = 0;

  uint8_t col = cnt & 3;
  HAL_GPIO_WritePin(SEG_Neg_Port[col], SEG_Neg_Pin[col], 1);

  cnt++;

  col = cnt & 3;
  disp_digit(disp_digits[col]);
  HAL_GPIO_WritePin(SEG_Neg_Port[col], SEG_Neg_Pin[col], 0);
}
