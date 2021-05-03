#pragma once

#include <stdint.h>
#include "usbpd_def.h"

uint16_t UCDC_Search_Next_Voltage(uint16_t target_centivolt, uint8_t up,
                                         uint8_t *pdoindex_o);
USBPD_StatusTypeDef UCDC_Request_Output(uint8_t pdoindex, uint16_t pps_centivolt);
