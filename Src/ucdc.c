
#include "ucdc.h"
#include "common.h"
#include "stm32g0xx_ll_ucpd.h"
#include "usbpd_core.h"
#include "usbpd_dpm_conf.h"
#include "usbpd_dpm_core.h"
#include "usbpd_hw.h"
#include "usbpd_pwr_if.h"

uint16_t UCDC_Search_Next_Voltage(uint16_t target_centivolt, uint8_t up,
                                  uint8_t *pdoindex_o) {
  uint16_t result = up ? 0xffff : 0;
  uint16_t upper_bound = 0, lower_bound = 0xffff;
  DBG_MSG("target=%hucV up=%hu\n", target_centivolt, up);
  DBG_MSG("Searching in %lu PDOs\n",
          DPM_Ports[USBPD_PORT_0].DPM_NumberOfRcvSRCPDO);
  for (int8_t index = 0; index < DPM_Ports[USBPD_PORT_0].DPM_NumberOfRcvSRCPDO;
       index++) {

    if (USBPD_PDO_TYPE_FIXED ==
        (DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
         USBPD_PDO_TYPE_Msk)) {
      uint32_t centiamp = ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                            USBPD_PDO_SRC_FIXED_MAX_CURRENT_Msk) >>
                           USBPD_PDO_SRC_FIXED_MAX_CURRENT_Pos);
      uint32_t centivolt =
          5 * ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                USBPD_PDO_SRC_FIXED_VOLTAGE_Msk) >>
               USBPD_PDO_SRC_FIXED_VOLTAGE_Pos);
      DBG_MSG("FIXED: %lucV %lucA\n", centivolt, centiamp);
      if (centivolt > upper_bound)
        upper_bound = centivolt;
      if (centivolt < lower_bound)
        lower_bound = centivolt;

      if (up && centivolt >= target_centivolt) {
        if (centivolt < result) {
          *pdoindex_o = index;
          result = centivolt;
        }
      } else if (!up && centivolt <= target_centivolt) {
        if (centivolt > result) {
          *pdoindex_o = index;
          result = centivolt;
        }
      }
      DBG_MSG("result=%hucV\n", result);
    } else if (USBPD_PDO_TYPE_APDO ==
               (DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                USBPD_PDO_TYPE_Msk)) {
      uint32_t centiamp =
          5 * ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                USBPD_PDO_SRC_APDO_MAX_CURRENT_Msk) >>
               USBPD_PDO_SRC_APDO_MAX_CURRENT_Pos);
      uint32_t centivolt_min =
          10 * ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                 USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Msk) >>
                USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Pos);
      uint32_t centivolt_max =
          10 * ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[index] &
                 USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Msk) >>
                USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Pos);
      DBG_MSG("PPS: %lucV~%lucV %lucA\n", centivolt_min, centivolt_max,
              centiamp);
      if (centivolt_max > upper_bound)
        upper_bound = centivolt_max;
      if (centivolt_min < lower_bound)
        lower_bound = centivolt_min;

      if (up && centivolt_max >= target_centivolt) {
        if (centivolt_min <= target_centivolt) {
          *pdoindex_o = index;
          result = target_centivolt;
        } else if (centivolt_min < result) {
          *pdoindex_o = index;
          result = centivolt_min;
        }
      } else if (!up && centivolt_min <= target_centivolt) {
        if (centivolt_max >= target_centivolt) {
          *pdoindex_o = index;
          result = target_centivolt;
        } else if (centivolt_max > result) {
          *pdoindex_o = index;
          result = centivolt_max;
        }
      }
      DBG_MSG("result=%hucV\n", result);
    }
  }
  if (result == 0xffff)
    result = upper_bound;
  else if (result == 0)
    result = lower_bound;

  if (result == 0xffff)
    result = 0; // failure
  return result;
}

USBPD_StatusTypeDef UCDC_Request_Output(uint8_t pdoindex, uint16_t pps_centivolt) {
  USBPD_SNKRDO_TypeDef rdo;
  USBPD_PDO_TypeDef pdo;
  USBPD_StatusTypeDef ret;

  pdo.d32 = DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[pdoindex];

  if (USBPD_CORE_PDO_TYPE_APDO == pdo.GenericPDO.PowerObject) {
    rdo.d32 = 0;
    rdo.ProgRDO.OperatingCurrentIn50mAunits =
        ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[pdoindex] &
          USBPD_PDO_SRC_APDO_MAX_CURRENT_Msk) >>
         USBPD_PDO_SRC_APDO_MAX_CURRENT_Pos);
    rdo.ProgRDO.OutputVoltageIn20mV = pps_centivolt / 2;
    rdo.ProgRDO.UnchunkedExtendedMessage = 0;
    rdo.ProgRDO.NoUSBSuspend = 0;
    rdo.ProgRDO.USBCommunicationsCapable = 0;
    rdo.ProgRDO.CapabilityMismatch = 0;
    rdo.FixedVariableRDO.ObjectPosition = pdoindex + 1;
    ret = USBPD_PE_Send_Request(0, rdo.d32, USBPD_CORE_PDO_TYPE_APDO);

  } else if (USBPD_CORE_PDO_TYPE_FIXED == pdo.GenericPDO.PowerObject) {
    rdo.d32 = 0;
    rdo.FixedVariableRDO.MaxOperatingCurrent10mAunits =
        ((DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[pdoindex] &
          USBPD_PDO_SRC_FIXED_MAX_CURRENT_Msk) >>
         USBPD_PDO_SRC_FIXED_MAX_CURRENT_Pos);
    rdo.FixedVariableRDO.OperatingCurrentIn10mAunits =
        rdo.FixedVariableRDO.MaxOperatingCurrent10mAunits;
    rdo.FixedVariableRDO.NoUSBSuspend = 0;
    rdo.FixedVariableRDO.USBCommunicationsCapable = 0;
    rdo.FixedVariableRDO.CapabilityMismatch = 0;
    rdo.FixedVariableRDO.GiveBackFlag = 0;
    rdo.FixedVariableRDO.ObjectPosition = pdoindex + 1;
    ret = USBPD_PE_Send_Request(0, rdo.d32, USBPD_CORE_PDO_TYPE_FIXED);
    // USBPD_DPM_RequestMessageRequest(0, rdo.GenericRDO.ObjectPosition,
    // voltage);
  }
  return ret;
}
