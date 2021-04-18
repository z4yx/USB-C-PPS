#pragma once

#include "stm32g071b_discovery_pwr.h"
#include "leds.h"
#include "joy_stick.h"
#include "usbpd_core.h"

#define LED_A1_Pin GPIO_PIN_13
#define LED_A1_GPIO_Port GPIOC
#define LED_K10_Pin GPIO_PIN_14
#define LED_K10_GPIO_Port GPIOC
#define LED_A4_Pin GPIO_PIN_0
#define LED_A4_GPIO_Port GPIOA
#define LED_A8_Pin GPIO_PIN_1
#define LED_A8_GPIO_Port GPIOA
#define OUT_EN_Pin GPIO_PIN_3
#define OUT_EN_GPIO_Port GPIOA
#define SW_R_Pin GPIO_PIN_4
#define SW_R_GPIO_Port GPIOA
#define SW_U_Pin GPIO_PIN_2
#define SW_U_GPIO_Port GPIOB
#define SW_D_Pin GPIO_PIN_10
#define SW_D_GPIO_Port GPIOB
#define SW_P_Pin GPIO_PIN_11
#define SW_P_GPIO_Port GPIOB
#define VBUS_ADC_Pin GPIO_PIN_12
#define VBUS_ADC_GPIO_Port GPIOB
#define SW_L_Pin GPIO_PIN_13
#define SW_L_GPIO_Port GPIOB
#define LED_K01_Pin GPIO_PIN_12
#define LED_K01_GPIO_Port GPIOA
#define LED_A2_Pin GPIO_PIN_15
#define LED_A2_GPIO_Port GPIOA
#define LED_K1_Pin GPIO_PIN_3
#define LED_K1_GPIO_Port GPIOB

/* Definitions of environment analog values */
  /* Value of analog reference voltage (Vref+), connected to analog voltage   */
  /* supply Vdda (unit: mV).                                                  */
#define VDDA_APPLI                       (3300UL)
