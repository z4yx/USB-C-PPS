#pragma once

#include "bsp_headers.h"

typedef enum {
//   MODE_SPY,
  MODE_STANDALONE,
} DEMO_MODE;

#define USBPD_USER_THREAD_COUNT    0

void            DEMO_InitBSP(void);
void            DEMO_InitTask(DEMO_MODE mode);
void            DEMO_SPY_Handler(void);
uint8_t         DEMO_IsSpyMode(void);
void            DEMO_PostCADMessage(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc);
void            DEMO_PostNotificationMessage(uint8_t PortNum, USBPD_NotifyEventValue_TypeDef EventVal);
void            DEMO_PostMMIMessage(uint32_t EventVal);
