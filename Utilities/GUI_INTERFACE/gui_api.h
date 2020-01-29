/**
  ******************************************************************************
  * @file    gui_api.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for GUI
  *          interface.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _GUI_API_H
#define _GUI_API_H

/* Includes ------------------------------------------------------------------*/
#include "usbpd_pdo_defs.h"
#include "bsp_gui.h"

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_GUI_API
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup USBPD_GUI_API_Exported_Defines USBPD GUI API exported Defines
  * @{
  */
#define GUI_PE_PORT_NUM_Pos             (24u)
#define GUI_PE_PORT_NUM_Msk             (0x3u << GUI_PE_PORT_NUM_Pos)
#define GUI_PE_PORT_NUM_0               (0x0u << GUI_PE_PORT_NUM_Pos)
#define GUI_PE_PORT_NUM_1               (0x1u << GUI_PE_PORT_NUM_Pos)
#define GUI_PE_NOTIF_Pos                (4u)
#define GUI_PE_NOTIF_Msk                (0xFFu << GUI_PE_NOTIF_Pos)

#define GUI_NOTIF_MEASUREMENT           (USBPD_NOTIFY_ALL + 1)
#define GUI_NOTIF_MEASUREMENT_STEP      40u

/**
  * @brief  USB PD GUI STATE MACHINE STATE
  */
typedef enum
{
  GUI_STATE_INIT,     /*!< GUI init state */
  GUI_STATE_RUNNING,  /*!< GUI running state */
  GUI_STATE_RESET,    /*!< GUI reset state */
} USBPD_GUI_State;

/**
  * @brief  USB PD GUI TAG TYPE
  */
typedef enum
{
  DPM_RESET_REQ            = 0x01,
  DPM_INIT_REQ             = 0x02,
  DPM_INIT_CNF             = 0x03,
  DPM_CONFIG_GET_REQ       = 0x04,
  DPM_CONFIG_GET_CNF       = 0x05,
  DPM_CONFIG_SET_REQ       = 0x06,
  DPM_CONFIG_SET_CNF       = 0x07,
  DPM_CONFIG_REJ           = 0x08,
  DPM_MESSAGE_REQ          = 0x09,
  DPM_MESSAGE_CNF          = 0x0A,
  DPM_MESSAGE_REJ          = 0x0B,
  DPM_MESSAGE_IND          = 0x0C,
  DPM_MESSAGE_RSP          = 0x0D,
  DPM_REGISTER_READ_REQ    = 0x0E,
  DPM_REGISTER_READ_CNF    = 0x0F,
  DPM_REGISTER_WRITE_REQ   = 0x10,
  DPM_REGISTER_WRITE_CNF   = 0x11,
  DEBUG_STACK_MESSAGE      = 0x12,
} USBPD_GUI_Tag_TypeDef;

/** @defgroup USBPD_CORE_GUI_TYPE_NOTIFICATION USBPD CORE TRACE  TYPE NOTIFICATION
  * @{
  */
#define  GUI_NOTIF_NUMBEROFRCVSNKPDO                    (1 << 0)
#define  GUI_NOTIF_RDOPOSITION                          (1 << 1)
#define  GUI_NOTIF_LISTOFRCVSRCPDO                      (1 << 2)
#define  GUI_NOTIF_NUMBEROFRCVSRCPDO                    (1 << 3)
#define  GUI_NOTIF_LISTOFRCVSNKPDO                      (1 << 4)
#define  GUI_NOTIF_ISCONNECTED                          (1 << 5)
#define  GUI_NOTIF_DATAROLE                             (1 << 6)
#define  GUI_NOTIF_POWERROLE                            (1 << 7)
#define  GUI_NOTIF_CCDEFAULTCURRENTADVERTISED           (1 << 8)
#define  GUI_NOTIF_VCONNON                              (1 << 9)
#define  GUI_NOTIF_VCONNSWAPED                          (1 << 10)
#define  GUI_NOTIF_MEASUREREPORTING                     (1 << 11)
#define  GUI_NOTIF_CC                                   (1 << 12)
#define  GUI_NOTIF_PE_EVENT                             (1 << 13)
#define  GUI_NOTIF_TIMESTAMP                            (1 << 14)
#define  GUI_NOTIF_POWER_EVENT                          (1 << 15)

#if !defined(PORT0_NB_SINKAPDO)
#define PORT0_NB_SINKAPDO               0U
#endif /* !PORT0_NB_SINKAPDO */
#if !defined(PORT1_NB_SINKAPDO)
#define PORT1_NB_SINKAPDO               0U
#endif /* !PORT1_NB_SINKAPDO */
#if !defined(PORT0_NB_SOURCEAPDO)
#define PORT0_NB_SOURCEAPDO             0U
#endif /* !PORT0_NB_SOURCEAPDO */
#if !defined(PORT1_NB_SOURCEAPDO)
#define PORT1_NB_SOURCEAPDO             0U
#endif /* !PORT1_NB_SOURCEAPDO */

/**
  * @}
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup USBPD_GUI_API_Exported_TypeDef USBPD GUI API exported TypeDef
  * @{
  */

typedef struct
{
  uint16_t VBUS_Level               :16;  /*!< VBUS Level */
  uint16_t IBUS_Level               :16;  /*!< IBUS Level */
  /* Measurement Reporting */
  union {
    uint8_t MeasurementReporting;
    struct {
      uint8_t MeasReportValue       :7;   /*!< Enable Measure reporting every tr x 40 ms  */
      uint8_t MeasReportActivation  :1;   /*!< Enable or Disable Measure reporting        */
    }d;
  }u;
  uint8_t Reserved                  :8;   /*!< Reserved bits */
} GUI_USER_ParamsTypeDef;

typedef struct
{
#if defined(_SNK) || defined(_DRP)
  USBPD_SNKPowerRequest_TypeDef DPM_SNKRequestedPower;          /*!< Requested Power by the sink board                     */
#else
  uint32_t            Reserved_ReqPower[6];                       /*!< Reserved bits to match with Resquested power information            */
#endif /* _SNK || _DRP */
#if defined(USBPD_REV30_SUPPORT)
#if _SRC_CAPA_EXT && (defined(_SRC)||defined(_DRP))
  USBPD_SCEDB_TypeDef DPM_SRCExtendedCapa;                      /*!< SRC Extended Capability                               */
#else
  uint32_t            ReservedSrcCapa[6];                       /*!< Reserved bits to match with SrcCapa information            */
#endif /* _SRC_CAPA_EXT && (_SRC || _DRP) */
#if _SNK_CAPA_EXT && (defined(_SNK)||defined(_DRP))
  USBPD_SKEDB_TypeDef DPM_SNKExtendedCapa;                      /*!< SNK Extended Capability                                */
  uint8_t             ReservedSnkCapa[3];                       /*!< Reserved bits to match with SnkCapaExt information     */
#else
  uint32_t            ReservedSnkCapa[6];                       /*!< Reserved bits to match with SnkCapaExt information     */
#endif /* _SNK_CAPA_EXT && (_SNK || _DRP) */
#if _MANU_INFO
  USBPD_MIDB_TypeDef  DPM_ManuInfoPort;                         /*!< Manufacturer information used for the port            */
  uint16_t            ReservedManu;                             /*!< Reserved bits to match with Manufacturer information            */
#else
  uint32_t            ReservedManu[7];                          /*!< Reserved bits to match with Manufacturer information            */
#endif /* _MANU_INFO */
#else
  uint32_t            ReservedRev3[13];                         /*!< Reserved bits to match with PD3.0 information            */
#endif /* USBPD_REV30_SUPPORT */
  uint32_t PE_DataSwap                                    : 1;  /*!< support data swap                                     */
  uint32_t PE_VconnSwap                                   : 1;  /*!< support VCONN swap                                    */
  uint32_t Reserved1                                      :30;  /*!< Reserved bits */
  uint32_t PWR_AccessoryDetection                         : 1; /*!< It enables or disables powered accessory detection */
  uint32_t PWR_AccessoryTransition                        : 1; /*!< It enables or disables transition from Powered.accessory to Try.SNK */
  USBPD_CORE_PDO_ExtPowered_TypeDef PWR_UnconstrainedPower: 1; /*!< UUT has an external power source available that is sufficient to adequately power the system while charging external devices or the UUT primary function is to charge external devices. */
  CAD_SNK_Source_Current_Adv_Typedef PWR_RpResistorValue  : 2; /*!< RP resitor value based on @ref CAD_SNK_Source_Current_Adv_Typedef */
  USBPD_CORE_PDO_USBCommCapable_TypeDef USB_Support       : 1; /*!< USB_Comms_Capable, is the UUT capable of enumerating as a USB host or device? */
  uint32_t USB_Device                                     : 1; /*!< Type_C_Can_Act_As_Device, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a device or as the Upstream Facing Port of a hub. */
  uint32_t USB_Host                                       : 1; /*!<  Type_C_Can_Act_As_Host, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a host or as the Downstream Facing Port of a hub */
  USBPD_CORE_PDO_USBSuspendSupport_TypeDef USB_SuspendSupport: 1; /*!<  USB Suspend support values in PDO definition (Source) */
  uint32_t CAD_tDRP                                       :7;  /*!< The period that DRP shall complete a Source to Sink and back advertisement */
  uint32_t CAD_dcSRC_DRP                                  :7;  /*!< The percent of time that a DRP shall advertise Source during tDRP (in %) */
  uint32_t Reserved2                                      :31;  /*!< reserved bits */
} USBPD_USER_SettingsTypeDef;
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported variable ---------------------------------------------------------*/
/** @defgroup USBPD_GUI_API_Exported_Variable USBPD GUI API exported Variable
  * @{
  */
extern GUI_USER_ParamsTypeDef GUI_USER_Params[USBPD_PORT_COUNT];
#if !defined(_RTOS)
extern volatile uint32_t      GUI_Flag;
#endif /* !_RTOS */

#if defined(GUI_API_C)
USBPD_USER_SettingsTypeDef       DPM_USER_Settings[USBPD_PORT_COUNT] =
{
  {
    .PWR_AccessoryDetection     = USBPD_FALSE,  /*!< It enables or disables powered accessory detection */
    .PWR_AccessoryTransition    = USBPD_FALSE,  /*!< It enables or disables transition from Powered.accessory to Try.SNK */
    .PWR_UnconstrainedPower     = USBPD_CORE_PDO_NOT_EXT_POWERED, /*!< UUT has an external power source available that is sufficient to adequately power the system while charging external devices or the UUT�s primary function is to charge external devices. */
    .PWR_RpResistorValue        = vRd_3_0A,     /*!< RP resitor value based on @ref CAD_SNK_Source_Current_Adv_Typedef */
    .USB_Support                = USBPD_CORE_PDO_USBCOMM_NOT_CAPABLE, /*!< USB_Comms_Capable, is the UUT capable of enumerating as a USB host or device? */
    .USB_Device                 = USBPD_FALSE,  /*!< Type_C_Can_Act_As_Device, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a device or as the Upstream Facing Port of a hub. */
    .USB_Host                   = USBPD_FALSE,  /*!<  Type_C_Can_Act_As_Host, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a host or as the Downstream Facing Port of a hub */
    .USB_SuspendSupport         = USBPD_CORE_PDO_USBSUSP_NOT_SUPPORTED, /*!<  USB Suspend support values in PDO definition (Source) */
    .CAD_tDRP                   = 80,           /*!<  Type_C_Can_Act_As_Host, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a host or as the Downstream Facing Port of a hub */
    .CAD_dcSRC_DRP              = 50,           /*!<  USB Suspend support values in PDO definition (Source) */
  },
#if USBPD_PORT_COUNT >= 2
  {
  .PWR_AccessoryDetection     = USBPD_FALSE,  /*!< It enables or disables powered accessory detection */
  .PWR_AccessoryTransition    = USBPD_FALSE,  /*!< It enables or disables transition from Powered.accessory to Try.SNK */
  .PWR_UnconstrainedPower     = USBPD_CORE_PDO_NOT_EXT_POWERED, /*!< UUT has an external power source available that is sufficient to adequately power the system while charging external devices or the UUT�s primary function is to charge external devices. */
  .PWR_RpResistorValue        = vRd_3_0A,     /*!< RP resitor value based on @ref CAD_SNK_Source_Current_Adv_Typedef */
  .USB_Support                = USBPD_CORE_PDO_USBCOMM_NOT_CAPABLE, /*!< USB_Comms_Capable, is the UUT capable of enumerating as a USB host or device? */
  .USB_Device                 = USBPD_FALSE,  /*!< Type_C_Can_Act_As_Device, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a device or as the Upstream Facing Port of a hub. */
  .USB_Host                   = USBPD_FALSE,  /*!<  Type_C_Can_Act_As_Host, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a host or as the Downstream Facing Port of a hub */
  .USB_SuspendSupport         = USBPD_CORE_PDO_USBSUSP_NOT_SUPPORTED, /*!<  USB Suspend support values in PDO definition (Source) */
  .CAD_tDRP                   = 80,           /*!<  Type_C_Can_Act_As_Host, Indicates whether the UUT can communicate with USB 2.0 or USB 3.1 as a host or as the Downstream Facing Port of a hub */
  .CAD_dcSRC_DRP              = 50,           /*!<  USB Suspend support values in PDO definition (Source) */
  }
#endif /* USBPD_PORT_COUNT >= 2 */
};
uint8_t GUI_NbPDO[4] = {(PORT0_NB_SINKPDO + PORT0_NB_SINKAPDO), 
                          ((PORT0_NB_SOURCEPDO + PORT0_NB_SOURCEAPDO)),
                          ((PORT1_NB_SINKPDO + PORT1_NB_SINKAPDO)),
                          ((PORT1_NB_SOURCEPDO + PORT1_NB_SOURCEAPDO))};
#else
extern USBPD_USER_SettingsTypeDef  DPM_USER_Settings[USBPD_PORT_COUNT];
extern uint8_t GUI_NbPDO[4];
#endif /* GUI_API_C */
/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup USBPD_GUI_API_Exported_Functions USBPD GUI API exported functions
  * @{
  */

USBPD_FunctionalState GUI_Init(const uint8_t* (*CB_HWBoardVersion)(void), const uint8_t* (*CB_HWPDType)(void), uint16_t (*CB_GetVoltage)(uint8_t), int16_t (*CB_GetCurrent)(uint8_t));
void                  GUI_Start(void);
void                  GUI_TimerCounter(void);
uint32_t              GUI_RXProcess(uint32_t Event);
uint32_t              GUI_FormatAndSendNotification(uint32_t PortNum, uint32_t TypeNotification, uint32_t Value);
uint32_t              GUI_GetMessage(uint8_t Character, uint8_t Error);
USBPD_GUI_State       GUI_SendAnswer(uint8_t **pMsgToSend, uint8_t *pSizeMsg);
USBPD_GUI_State       GUI_SendNotification(uint8_t PortNum, uint8_t **pMsgToSend, uint8_t *pSizeMsg, uint32_t TypeNotifcation, uint32_t Value);
void                  GUI_PostNotificationMessage(uint8_t PortNum, uint16_t EventVal);
void                  GUI_SaveInfo(uint8_t PortNum, uint8_t DataId, uint8_t *Ptr, uint32_t Size);
USBPD_FunctionalState GUI_IsRunning(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* _GUI_API_H */

