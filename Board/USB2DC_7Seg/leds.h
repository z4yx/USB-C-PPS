#pragma once

void LED_7Seg_Init(void);
void LED_OutEnable(uint8_t enable);
void LED_ToggleOutEnable(void);
void Seg7_Refresh(void);
void Seg7_Update(uint32_t number, uint8_t point_mask);
