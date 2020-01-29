/**
  ******************************************************************************
  * @file    usbpd_core.h
  * @author  MCD Application Team
  * @brief   This file contains the core stack API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __USBPD_CORE_H_
#define __USBPD_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbpd_def.h"
#if defined(USBPDCORE_TCPM_SUPPORT)
#include "tcpc.h"
#endif /* USBPDCORE_TCPM_SUPPORT */

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_CORE
  * @{
  */

/** @addtogroup USBPD_CORE_CAD
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup USBPD_CORE_CAD_Exported_Types USBPD CORE CAD Exported Types
  * @{
  */

/**
  * @brief funtion return value @ref USBPD_CORE_CAD
  * @{
  */
typedef enum
{
  USBPD_CAD_OK,                   /*!< USBPD CAD Status OK                */
  USBPD_CAD_INVALID_PORT,         /*!< USBPD CAD Status INVALID PORT      */
  USBPD_CAD_ERROR,                /*!< USBPD CAD Status ERROR             */
  USBPD_CAD_MALLOCERROR,          /*!< USBPD CAD Status ERROR MALLOC      */
  USBPD_CAD_INVALIDRESISTOR       /*!< USBPD CAD Status INVALID RESISTOR  */
}
USBPD_CAD_StatusTypeDef;
/**
  * @}
  */

/**
  * @brief activation value @ref USBPD_CORE_CAD
  * @{
  */
typedef enum
{
  USBPD_CAD_DISABLE,         /*!< USBPD CAD activation status Disable   */
  USBPD_CAD_ENABLE           /*!< USBPD CAD activation status Enable   */
} USBPD_CAD_activation;
/**
  * @}
  */

/**
  * @brief CallBacks exposed by the @ref USBPD_CORE_CAD
  */
typedef struct
{
  /**
  * @brief  CallBack used to report events to DPM.
  * @param  PortNum The handle of the port
  * @param  State   CAD state @ref USBPD_CAD_EVENT
  * @param  Cc      The Communication Channel for the USBPD communication @ref CCxPin_TypeDef
  * @retval None
  */
  void (*USBPD_CAD_CallbackEvent)(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc);
  /**
  * @brief  CallBack to wakeup the CAD.
  * @retval None
  */
  void (*USBPD_CAD_WakeUp)(void);
} USBPD_CAD_Callbacks;

/**
  * @}
  */

/** @defgroup USBPD_CORE_CAD_Exported_Functions_Grp1 USBPD CORE CAD Exported Functions
  * @{
  */
/**
  * @brief  Initialize the CAD module for a specified port.
  * @param  PortNum           Index of current used port
  * @param  CallbackFunctions CAD port callback function
  * @param  Settings          Pointer on @ref USBPD_SettingsTypeDef structure
  * @param  Params            Pointer on @ref USBPD_ParamsTypeDef structure
  * @retval USBPD_CAD status
  */
USBPD_CAD_StatusTypeDef USBPD_CAD_Init(uint8_t PortNum, const USBPD_CAD_Callbacks *CallbackFunctions, USBPD_SettingsTypeDef *Settings, USBPD_ParamsTypeDef *Params);

/**
  * @brief  function used to process type C state machine detection.
  * @retval Timing in ms
  */
uint32_t                USBPD_CAD_Process(void);

/**
  * @brief  Enable or Disable CAD port.
  * @param  PortNum Index of current used port
  * @param  State   The new state of the port @ref USBPD_CAD_activation
  * @retval None
  */
void                    USBPD_CAD_PortEnable(uint8_t PortNum, USBPD_CAD_activation State);

/**
  * @brief  Set the resitor to present a SNK.
  * @param  PortNum Index of current used port
  * @retval None
  */
void                    USBPD_CAD_AssertRd(uint8_t PortNum);

/**
  * @brief  Set the resitor to present a SRC.
  * @param  PortNum Index of current used port
  * @retval None
  */
void                    USBPD_CAD_AssertRp(uint8_t PortNum);

/**
  * @brief  Force type C state machine to enter recovery state
  * @param  PortNum Index of current used port
  * @retval None
  */
void                    USBPD_CAD_EnterErrorRecovery(uint8_t PortNum);

/**
  * @brief  Set the default Rd resistor
  * @param  PortNum Index of current used port
  * @param  RdValue
  * @retval USBPD_CAD_OK or USBPD_CAD_INVALIDRESISTOR
  */
USBPD_CAD_StatusTypeDef USBPD_CAD_SetRpResistor(uint8_t PortNum, CAD_RP_Source_Current_Adv_Typedef RdValue);
/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup USBPD_CORE_TRACE
  * @{
  */

/** @defgroup USBPD_CORE_TRACE_Exported_Types USBPD CORE TRACE Exported Types
  * @{
  */
typedef enum
{
  USBPD_TRACE_FORMAT_TLV  = 0,
  USBPD_TRACE_MESSAGE_IN  = 1,
  USBPD_TRACE_MESSAGE_OUT = 2,
  USBPD_TRACE_CADEVENT    = 3,
  USBPD_TRACE_PE_STATE    = 4,
  USBPD_TRACE_CAD_LOW     = 5,
  USBPD_TRACE_DEBUG       = 6,
  USBPD_TRACE_SRC         = 7,
  USBPD_TRACE_SNK         = 8,
  USBPD_TRACE_NOTIF       = 9,
  USBPD_TRACE_POWER       = 10
}
TRACE_EVENT;

typedef void (*TRACE_ENTRY_POINT)(TRACE_EVENT type, uint8_t port, uint8_t sop, uint8_t *ptr, uint32_t size);

extern uint32_t Crc;
extern TRACE_ENTRY_POINT USBPD_Trace;

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup USBPD_CORE_PE
  * @{
  */

#if defined(USBPDCORE_SVDM) || defined(USBPDCORE_UVDM) || defined(USBPDCORE_VCONN_SUPPORT)
/** @defgroup USBPD_CORE_VDM_Exported_Callback USBPD CORE VDM Exported Callback
  * @{
  */

/**
  * @brief CallBacks exposed by the @ref USBPD_CORE_PE
  * */
typedef struct
{
  /**
    * @brief  VDM Discovery identity callback
    * @note   Function is called to get Discovery identity information linked to the device and answer
    *         to SVDM Discovery identity init message sent by port partner
    * @param  PortNum   current port number
    * @param  pIdentity Pointer on @ref USBPD_DiscoveryIdentity_TypeDef structure
    * @retval USBPD status: @ref USBPD_ACK or @ref USBPD_BUSY
    */
  USBPD_StatusTypeDef(*USBPD_VDM_DiscoverIdentity)(uint8_t PortNum, USBPD_DiscoveryIdentity_TypeDef *pIdentity);

  /**
    * @brief  VDM Discover SVID callback
    * @note   Function is called to retrieve SVID supported by device and answer
    *         to SVDM Discovery SVID init message sent by port partner
    * @param  PortNum     current port number
    * @param  p_SVID_Info Pointer on @ref USBPD_SVIDInfo_TypeDef structure
    * @param  pNbSVID     Pointer on number of SVID
    * @retval USBPD status  @ref USBPD_BUSY or @ref USBPD_ACK or @ref USBPD_NAK
    */
  USBPD_StatusTypeDef(*USBPD_VDM_DiscoverSVIDs)(uint8_t PortNum, uint16_t **p_SVID_Info, uint8_t *pNbSVID);

  /**
    * @brief  VDM Discover Mode callback (report all the modes supported by SVID)
    * @note   Function is called to report all the modes supported by selected SVID and answer
    *         to SVDM Discovery Mode init message sent by port partner
    * @param  PortNum      current port number
    * @param  SVID         SVID value
    * @param  p_ModeTab    Pointer on the mode value
    * @param  NumberOfMode Number of mode available
    * @retval USBPD status
    */
  USBPD_StatusTypeDef(*USBPD_VDM_DiscoverModes)(uint8_t PortNum, uint16_t SVID, uint32_t **p_ModeTab, uint8_t *NumberOfMode);

  /**
    * @brief  VDM Mode enter callback
    * @note   Function is called to check if device can enter in the mode received for the selected SVID in the
    *         SVDM enter mode init message sent by port partner
    * @param  PortNum   current port number
    * @param  SVID      SVID value
    * @param  ModeIndex Index of the mode to be entered
    * @retval USBPD status @ref USBPD_ACK/@ref USBPD_NAK
    */
  USBPD_StatusTypeDef(*USBPD_VDM_ModeEnter)(uint8_t PortNum, uint16_t SVID, uint32_t ModeIndex);

  /**
    * @brief  VDM Mode exit callback
    * @note   Function is called to check if device can exit from the mode received for the selected SVID in the
    *         SVDM exit mode init message sent by port partner
    * @param  PortNum   current port number
    * @param  SVID      SVID value
    * @param  ModeIndex Index of the mode to be exited
    * @retval USBPD status @ref USBPD_ACK/@ref USBPD_NAK
    */
  USBPD_StatusTypeDef(*USBPD_VDM_ModeExit)(uint8_t PortNum, uint16_t SVID, uint32_t ModeIndex);

  /**
    * @brief  Inform identity callback
    * @note   Function is called to save Identity information received in Discovery identity from port partner
              (answer to SVDM discovery identity sent by device)
    * @param  PortNum       current port number
    * @param  SOPType       SOP type
    * @param  CommandStatus Command status based on @ref USBPD_VDM_CommandType_Typedef
    * @param  pIdentity     Pointer on the discovery identity information based on @ref USBPD_DiscoveryIdentity_TypeDef
    * @retval None
    */
  void (*USBPD_VDM_InformIdentity)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_CommandType_Typedef CommandStatus, USBPD_DiscoveryIdentity_TypeDef *pIdentity);

  /**
    * @brief  Inform SVID callback
    * @note   Function is called to save list of SVID received in Discovery SVID from port partner
              (answer to SVDM discovery SVID sent by device)
    * @param  PortNum       current port number
    * @param  SOPType       SOP type
    * @param  CommandStatus Command status based on @ref USBPD_VDM_CommandType_Typedef
    * @param  pListSVID     Pointer of list of SVID based on @ref USBPD_SVIDInfo_TypeDef
    * @retval None
    */
  void (*USBPD_VDM_InformSVID)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_CommandType_Typedef CommandStatus, USBPD_SVIDInfo_TypeDef *pListSVID);

  /**
    * @brief  Inform Mode callback ( received in Discovery Modes ACK)
    * @note   Function is called to save list of modes linked to SVID received in Discovery mode from port partner
              (answer to SVDM discovery mode sent by device)
    * @param  PortNum         current port number
    * @param  SOPType         SOP type
    * @param  CommandStatus   Command status based on @ref USBPD_VDM_CommandType_Typedef
    * @param  pModesInfo      Pointer of Modes info based on @ref USBPD_ModeInfo_TypeDef
    * @retval None
    */
  void (*USBPD_VDM_InformMode)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_CommandType_Typedef CommandStatus, USBPD_ModeInfo_TypeDef *pModesInfo);

  /**
    * @brief  Inform Mode enter callback
    * @note   Function is called to inform if port partner accepted or not to enter in the mode
    *         specified in the SVDM enter mode sent by the device
    * @param  PortNum       current port number
    * @param  SOPType       SOP type
    * @param  CommandStatus Command status based on @ref USBPD_VDM_CommandType_Typedef
    * @param  SVID          SVID ID
    * @param  ModeIndex     Index of the mode to be entered
    * @retval None
    */
  void (*USBPD_VDM_InformModeEnter)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_CommandType_Typedef CommandStatus, uint16_t SVID, uint32_t ModeIndex);

  /**
    * @brief  Inform Mode exit callback
    * @note   Function is called to inform if port partner accepted or not to exit from the mode
    *         specified in the SVDM exit mode sent by the device
    * @param  PortNum       current port number
    * @param  SOPType       SOP type
    * @param  CommandStatus Command status based on @ref USBPD_VDM_CommandType_Typedef
    * @param  SVID          SVID ID
    * @param  ModeIndex     Index of the mode to be exited
    * @retval None
    */
  void (*USBPD_VDM_InformModeExit)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_CommandType_Typedef CommandStatus, uint16_t SVID, uint32_t ModeIndex);

  /**
    * @brief  Send VDM Attention message callback
    * @note   Function is called when device wants to send a SVDM attention message to port partner
    *         (for instance DP status can be filled through this function)
    * @param  PortNum    current port number
    * @param  pNbData    Pointer of number of VDO to send
    * @param  pVDO       Pointer of VDO to send
    * @retval None
    */
  void (*USBPD_VDM_SendAttention)(uint8_t PortNum, uint8_t *pNbData, uint32_t *pVDO);

  /**
    * @brief  Receive VDM Attention callback
    * @note   Function is called when a SVDM attention init message has been received from port partner
    *         (for instance, save DP status data through this function)
    * @param  PortNum   current port number
    * @param  NbData    Number of received VDO
    * @param  VDO       Received VDO
    * @retval None
    */
  void (*USBPD_VDM_ReceiveAttention)(uint8_t PortNum, uint8_t NbData, uint32_t VDO);

  /**
    * @brief  VDM Send Specific message callback
    * @note   Function is called when device wants to send a SVDM specific init message to port partner
    *         (for instance DP status or DP configure can be filled through this function)
    * @param  PortNum    current port number
    * @param  SOPType    SOP type
    * @param  VDMCommand VDM command based on @ref USBPD_VDM_Command_Typedef
    * @param  pNbData    Pointer of number of VDO to send
    * @param  pVDO       Pointer of VDO to send
    * @retval None
    */
  void (*USBPD_VDM_SendSpecific)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_Command_Typedef VDMCommand, uint8_t *pNbData, uint32_t *pVDO);

  /**
    * @brief  VDM Receive Specific message callback
    * @note   Function is called to answer to a SVDM specific init message received by port partner.
    *         (for instance, retrieve DP status or DP configure data through this function)
    * @param  PortNum         Current port number
    * @param  VDMCommand      VDM command based on @ref USBPD_VDM_Command_Typedef
    * @param  pNbData     Pointer of number of received VDO and used for the answer
    * @param  pVDO        Pointer of received VDO and use for the answer
    * @retval USBPD Status
    */
  USBPD_StatusTypeDef(*USBPD_VDM_ReceiveSpecific)(uint8_t PortNum, USBPD_VDM_Command_Typedef VDMCommand, uint8_t *pNbData, uint32_t *pVDO);

  /**
    * @brief  VDM Specific message callback to inform user of reception of VDM specific message
    * @note   Function is called when answer from SVDM specific init message has been received by the device
    *         (for instance, save DP status and DP configure data through this function)
    * @param  PortNum    current port number
    * @param  SOPType    SOP type
    * @param  VDMCommand VDM command based on @ref USBPD_VDM_Command_Typedef
    * @param  pNbData    Pointer of number of received VDO
    * @param  pVDO       Pointer of received VDO
    * @retval None
    */
  void (*USBPD_VDM_InformSpecific)(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_Command_Typedef VDMCommand, uint8_t *pNbData, uint32_t *pVDO);

#if defined(USBPDCORE_UVDM)
  /**
    * @brief  VDM Send Unstructured message callback
    * @param  PortNum       current port number
    * @param  pUVDM_Header  Pointer on UVDM header based on @ref USBPD_UVDMHeader_TypeDef
    * @param  pNbData       Pointer of number of VDO to send
    * @param  pVDO          Pointer of VDO to send
    * @retval None
    */
  void (*USBPD_VDM_SendUVDM)(uint8_t PortNum, USBPD_UVDMHeader_TypeDef *pUVDM_Header, uint8_t *pNbData, uint32_t *pVDO);

  /**
    * @brief  Unstructured VDM  message callback to inform user of reception of UVDM message
    * @param  PortNum    current port number
    * @param  UVDM_Header UVDM header based on @ref USBPD_UVDMHeader_TypeDef
    * @param  pNbData    Pointer of number of received VDO
    * @param  pVDO       Pointer of received VDO
    * @retval USBPD Status
    */
  USBPD_StatusTypeDef(*USBPD_VDM_ReceiveUVDM)(uint8_t PortNum, USBPD_UVDMHeader_TypeDef UVDM_Header, uint8_t *pNbData, uint32_t *pVDO);
#endif /* USBPDCORE_UVDM */
} USBPD_VDM_Callbacks;

/**
  * @}
  */

#endif /* USBPDCORE_SVDM || USBPDCORE_UVDM || USBPDCORE_VCONN_SUPPORT */

/** @defgroup USBPD_CORE_PE_Exported_TypesDefinitions USBPD CORE PE Exported Types Definitions
  * @{
  */

/** @defgroup PE_CallBacks_structure_definition PE CallBacks structure definition
  * @brief  PE CallBacks exposed by the PE to the  DMP
  * @{
  */
typedef struct
{
  /**
    * @brief  Callback used to request DPM to setup the new power level.
    * @note   this callback is mandatory for a SRC and DRP.
    * @param  PortNum Port number
    * @retval Returned values are: @ref USBPD_ACCEPT, @ref USBPD_REJECT, @ref USBPD_WAIT
  */
  USBPD_StatusTypeDef(*USBPD_PE_RequestSetupNewPower)(uint8_t PortNum);

  /**
    * @brief  Callback used to inform the DPM about the different HardReset step.
    * @param  PortNum Port number
    * @param  Role of the board @ref USBPD_PortPowerRole_TypeDef
    * @param  Status HR Status @ref USBPD_HR_Status_TypeDef
    * @retval None
  */
  void (*USBPD_PE_HardReset)(uint8_t PortNum, USBPD_PortPowerRole_TypeDef CurrentRole, USBPD_HR_Status_TypeDef Status);

  /**
    * @brief  Callback used to ask application the reply status for a power role swap request.
    * @note   if the callback is NULL, the request power role swap are automatically rejected.
    * @param  PortNum Port number
    * @retval Returned values are: @ref USBPD_ACCEPT, @ref USBPD_WAIT, @ref USBPD_REJECT
  */
  USBPD_StatusTypeDef(*USBPD_PE_EvaluatPRSwap)(uint8_t PortNum);

  /**
    * @brief  Callback used by the stack to notify an event.
    * @note   this callback is mandatory
    * @param  PortNum   Port number
    * @param  EventVal  @ref USBPD_NotifyEventValue_TypeDef
    * @retval none
    */
  void (*USBPD_PE_Notify)(uint8_t PortNum, USBPD_NotifyEventValue_TypeDef EventVal);

  /**
    * @brief  Callback used by the stack to inform DPM that an extended message has been received.
    * @param  PortNum port number value
    * @param  MsgId extended message id @USBPD_ExtendedMsg_TypeDef
    * @param  ptrData pointer on the data
    * @param  DataSize size of the data
    * @retval None
    */
  void (*USBPD_PE_ExtendedMessage)(uint8_t PortNum, USBPD_ExtendedMsg_TypeDef MsgId, uint8_t *ptrData, uint16_t DataSize);

  /**
    * @brief  Callbakc used by the stack to get information from DPM or PWR_IF.
    * @param  PortNum Port number
    * @param  DataId Type of data to be read from DPM based on @ref USBPD_CORE_DataInfoType_TypeDef
    * @param  Ptr Pointer on address where DPM data should be written (u8 pointer)
    * @param  Size Pointer on nb of bytes written by DPM
    * @retval None
    */
  void (*USBPD_PE_GetDataInfo)(uint8_t PortNum, USBPD_CORE_DataInfoType_TypeDef DataId , uint8_t *Ptr, uint32_t *Size);

  /**
    * @brief  Callbakc used by the stack to set information inside DPM or PWR_IF.
    * @param  PortNum Port number
    * @param  DataId Type of data to be read from DPM based on @ref USBPD_CORE_DataInfoType_TypeDef
    * @param  Ptr Pointer on address where DPM data to be updated could be read (u8 pointer)
    * @param  Size Nb of byte to be updated in DPM
    * @retval None
    */
  void (*USBPD_PE_SetDataInfo)(uint8_t PortNum, USBPD_CORE_DataInfoType_TypeDef DataId , uint8_t *Ptr, uint32_t Size);

  /**
    * @brief  Callback used by a SOURCE to evluate the SINK request
    * @param  PortNum Port number
    * @param  PtrPowerObject  Pointer on the power data object
    * @retval Returned values are: @ref USBPD_ACCEPT, @ref USBPD_REJECT, @ref USBPD_WAIT, @ref USBPD_GOTOMIN
    */
  USBPD_StatusTypeDef(*USBPD_PE_SRC_EvaluateRequest)(uint8_t PortNum, USBPD_CORE_PDO_Type_TypeDef *PtrPowerObject);

  /**
    * @brief  Callback used by a sink to evaluate the Source Capabilities.
    * @note   the callback is used in the SNK and DRP context.
    * @param  PortNum         Port number
    * @param  PtrRequestData  Pointer on selected request data object
    * @param  PtrPowerObject  Pointer on selected power data object
    * @retval None
    */
  void (*USBPD_PE_SNK_EvaluateCapabilities)(uint8_t PortNum, uint32_t *RequestData, USBPD_CORE_PDO_Type_TypeDef *PtrPowerObject);

  /**
    * @brief  Callback used during the different step of the power role swap
    * @note   see MSC documentation to get more detail
    * @param  PortNum Port number
    * @param  Role of the board @ref USBPD_PortPowerRole_TypeDef
    * @param  Status HR Status update @ref USBPD_PRS_Status_TypeDef
    * @retval None
    */
  void (*USBPD_PE_PowerRoleSwap)(uint8_t PortNum, USBPD_PortPowerRole_TypeDef CurrentRole, USBPD_PRS_Status_TypeDef Status);

  /**
    * @brief  Callback used to wakeup the current state machine
    * @note   this function is mandatory but has effect only in RTOS context
    * @param  PortNum Port number
    * @retval None
    */
  void (*USBPD_PE_WakeupCallback)(uint8_t PortNum);

  /**
    * @brief  Callback used to ask application the reply status for a Vconn swap request
    * @param  PortNum Port number
    * @retval Returned values are: @ref USBPD_ACCEPT, @ref USBPD_REJECT, @ref USBPD_WAIT
    */
  USBPD_StatusTypeDef(*USBPD_PE_EvaluateVconnSwap)(uint8_t PortNum);

  /**
    * @brief  Callback used to enable/disable the vconn power
    * @note   the function is mandatory to manage the VconnPower
    * @param  PortNum Port number
    * @param  State Enable or Disable VConn on CC lines
    * @retval Returned values are: @ref USBPD_ACCEPT, @ref USBPD_REJECT
    */
  USBPD_StatusTypeDef(*USBPD_PE_VconnPwr)(uint8_t PortNum, USBPD_FunctionalState State);

  /**
    * @brief  Callback used by PE to enter in error recovery state
    * @param  PortNum Port number
    * @retval None
    */
  void (*USBPD_PE_EnterErrorRecovery)(uint8_t PortNum);

  /**
    * @brief  Callback used to ask application the reply status for a DataRoleSwap request
    * @note   if the callback is not set (ie NULL) the stack will automatically reject the request
    * @param  PortNum Port number
    * @retval Returned values are: @ref USBPD_REJECT, @ref USBPD_ACCEPT
    */
  USBPD_StatusTypeDef(*USBPD_PE_DataRoleSwap)(uint8_t PortNum);

  /**
    * @brief  Callback used to check the VBUS status
    * @note   this function is mandatory for a SNK port and used inside the hardreset process
    * @param  PortNum Port number
    * @param  Vsafe   Vsafe status based on @ref USBPD_VSAFE_StatusTypeDef
    * @retval Returned values are: @ref USBPD_DISABLE or @ref USBPD_ENABLE
    */
  USBPD_FunctionalState(*USBPD_PE_IsPowerReady)(uint8_t PortNum, USBPD_VSAFE_StatusTypeDef Vsafe);

} USBPD_PE_Callbacks;

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup USBPD_CORE_PE_Exported_Functions USBPD CORE PE Exported Functions
  * @{
  */

/** @defgroup USBPD_CORE_PE_Exported_Functions_Group1 USBPD CORE PE Exported Functions to DPM CORE
  * @{
  */
/**
  * @brief  Initialize Policy Engine layer for a Port
  * @note   the value pSettings and pParams shall be presistante because the stack keep there memory location
  * @param  PortNum     Index of current used port
  * @param  pSettings   Pointer on @ref USBPD_SettingsTypeDef structure
  * @param  pParams     Pointer on @ref USBPD_ParamsTypeDef structure
  * @param  PECallbacks Callback function to be passed to PRL layer
  * @retval USBPD status @ref USBPD_OK or @ref USBPD_ERROR
  */
USBPD_StatusTypeDef USBPD_PE_Init(uint8_t PortNum, USBPD_SettingsTypeDef *pSettings, USBPD_ParamsTypeDef *pParams, const USBPD_PE_Callbacks *PECallbacks);

/**
  * @brief  Check coherence between lib selected and the lib include inside the project
  * @note   This function can be used only during developpment process to avoid mismatch
  * @param  LibId    Value fix defined with the switch lib.
  * @retval if the check is OK @ref USBPD_TRUE else @ref USBPD_FALSE
  */
uint32_t            USBPD_PE_CheckLIB(uint32_t LibId);

/**
  * @brief  Return the need of the stack in terms of dynamique allocation
  * @note   the value returned corresponds to the allocationt need for 2 ports so if application manage
  *         only one port the need is be divided by 2
  * @retval Memory size
  */
uint32_t            USBPD_PE_GetMemoryConsumption(void);

/**
  * @brief  Set the trace pointer and the debug level
  * @note   the recommanded value for debug trace level is 0x3
  *              0x1 : Policy engine state
  *              0x2 : Notification
  *              0x3 : Policy engine state + Notification
  * @param  Ptr function on trace
  * @param  Debug Trace debug level
  * @retval None
  */
void                USBPD_PE_SetTrace(TRACE_ENTRY_POINT Ptr, uint8_t Debug);

#if defined(USBPDCORE_SRC) || defined(USBPDCORE_DRP)
/**
  * @brief  Policy Engine Source state machine
  * @param  PortNum Index of current used port
  * @retval Timing in ms
  */
uint32_t            USBPD_PE_StateMachine_SRC(uint8_t PortNum);
#endif /* USBPDCORE_SRC || USBPDCORE_DRP */

#if defined(USBPDCORE_SNK) || defined(USBPDCORE_DRP)
/**
  * @brief  Policy Engine Sink state machine
  * @param  PortNum Index of current used port
  * @retval Timing in ms
  */
uint32_t            USBPD_PE_StateMachine_SNK(uint8_t PortNum);
#endif /* USBPDCORE_SNK || USBPDCORE_DRP */

#ifdef USBPDCORE_DRP
/**
  * @brief  Policy Engine DRP state machine
  * @param  PortNum Index of current used port
  * @retval Timing in ms
  */
uint32_t            USBPD_PE_StateMachine_DRP(uint8_t PortNum);
#endif /* USBPDCORE_DRP */

/**
  * @brief  Function called by DPM to set the cable status connected or disconnected.
  * @param  PortNum     Index of current used port
  * @param  IsConnected Cable connection status: 1 if connected and 0 is disconnected
  * @retval USBPD status @ref USBPD_OK
  */
USBPD_StatusTypeDef USBPD_PE_IsCableConnected(uint8_t PortNum, uint8_t IsConnected);

/**
  * @brief  Increment PE Timers tick
  * @note   This function must be called each elasped ms
  * @param  PortNum Index of current used port
  * @retval None
  */
void                USBPD_PE_TimerCounter(uint8_t PortNum);

/**
  * @brief  Update PE Timers tick
  * @note   This function must be called when exited the low power mode
  * @param  PortNum Index of current used port
  * @param  Tick value in ms
  * @retval None
  */
void                USBPD_PE_TimerCounteUpdate(uint8_t PortNum, uint32_t Tick);

/**
  * @}
  */

#if defined(USBPDCORE_SVDM) || defined(USBPDCORE_UVDM) || defined(USBPDCORE_VCONN_SUPPORT)
/** @defgroup USBPD_CORE_PE_Exported_Functions_Group2 USBPD CORE PE Exported Functions to VDM USER
  * @{
  */
/**
  * @brief  Initialize VDM callback functions in PE
  * @param  PortNum       Index of current used port
  * @param  VDMCallbacks  Pointer on VDM callbacks based on @ref USBPD_VDM_Callbacks
  * @retval None
  */
void                USBPD_PE_InitVDM_Callback(uint8_t PortNum, USBPD_VDM_Callbacks *VDMCallbacks);
/**
  * @}
  */
#endif /* USBPDCORE_SVDM || USBPDCORE_UVDM || USBPDCORE_VCONN_SUPPORT*/

/** @defgroup USBPD_CORE_PE_Exported_Functions_Group3 USBPD CORE PE Exported Functions to DPM USER
  * @{
  */

/**
  * @brief  This generic function to send a control message
  * @param  PortNum   Index of current used port
  * @param  CtrlMsg   Control message id @USBPD_ControlMsg_TypeDef
  * @param  SOPType   SOP Type based on @ref USBPD_SOPType_TypeDef
  * @retval status    @ref USBPD_OK, @ref USBPD_BUSY, @ref USBPD_ERROR or @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_Request_CtrlMessage(uint8_t PortNum, USBPD_ControlMsg_TypeDef CtrlMsg, USBPD_SOPType_TypeDef SOPType);

/**
  * @brief  This generic function is used to send data message
  * @note   the parameter pData is used only if DataMsg is equal to USBPD_DATAMSG_ALERT or USBPD_DATAMSG_GET_COUNTRY_INFO
  * @param  PortNum   Index of current used port
  * @param  DataMsg   Data message id based on @ref USBPD_DataMsg_TypeDef
  * @param  pData     Pointer on the data to send
  * @retval status    @ref USBPD_OK, @ref USBPD_BUSY, @ref USBPD_ERROR or @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_Request_DataMessage(uint8_t PortNum, USBPD_DataMsg_TypeDef DataMsg, uint32_t *pData);

/**
  * @brief  This function is used to force PE to perform an Hard Reset.
  * @param  PortNum Index of current used port
  * @retval status  @ref USBPD_OK
  */
USBPD_StatusTypeDef USBPD_PE_Request_HardReset(uint8_t PortNum);

/**
  * @brief  Request the PE to send a cable reset.
  * @note   Only a DFP Shall generate Cable Reset Signaling. A DFP Shall only generate Cable Reset Signaling within an Explicit Contract.
            The DFP has to be supplying VCONN prior to a Cable Reset
  * @param  PortNum The current port number
  * @retval USBPD Status
  */
USBPD_StatusTypeDef USBPD_PE_Request_CableReset(uint8_t PortNum);

#if defined(USBPDCORE_SNK) || defined(USBPDCORE_DRP)
/**
  * @brief  This function request PE to send a request message
  * @param  PortNum   Index of current used port
  * @param  Rdo       Requested data object
  * @param  PWobject  Power Object
  * @retval status    @ref USBPD_OK, @ref USBPD_BUSY, @ref USBPD_ERROR or @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_Send_Request(uint8_t PortNum, uint32_t Rdo, USBPD_CORE_PDO_Type_TypeDef PWobject);
#endif /* USBPDCORE_SNK || USBPDCORE_DRP */

#if defined(USBPD_REV30_SUPPORT)
/**
  * @brief  This function send an extended message
  * @Note   The managment of chunk or unchunked message is manage inside the
  * @param  PortNum      Port number value
  * @param  SOPType      Message type based on @ref USBPD_SOPType_TypeDef
  * @param  MessageType  Extended Msg type @ref USBPD_ExtendedMsg_TypeDef
  * @param  Ptrdata      Data pointer to PRL layer
  * @param  DataSize     Size of the data (<= @ref USBPD_MAX_EXTENDED_MSGLEN)
  * @retval status       @ref USBPD_OK, @ref USBPD_BUSY, @ref USBPD_ERROR or @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SendExtendedMessage(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_ExtendedMsg_TypeDef MessageType, uint8_t *Ptrdata, uint16_t DataSize);

#if defined(USBPDCORE_FASTROLESWAP)
/**
  * @brief  this function request PE to perform an FRS signalling.
  * @param  PortNum
  * @retval None
  */
void USBPD_PE_ExecFastRoleSwapSignalling(uint8_t PortNum);
#endif
#endif /* USBPD_REV30_SUPPORT */

#if defined(USBPDCORE_SVDM) || defined(USBPDCORE_VCONN_SUPPORT)
/**
  * @brief  Called by DPM to request the PE to perform a VDM identity request.
  * @note   On PD2.0, SVDM Identity cannot be sent by UFP.
  * @param  PortNum Index of current used port
  * @param  SOPType SOP Type (Only SOP or SOP')
  * @retval USBPD status
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestIdentity(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType);

/**
  * @brief  Called by DPM to request the PE to perform a VDM SVID request.
  * @param  PortNum Index of current used port
  * @param  SOPType SOP Type (only valid for SOP or SOP')
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestSVID(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType);

/**
  * @brief  Called by DPM to request the PE to perform a VDM Discovery mode message on one SVID.
  * @param  PortNum Index of current used port
  * @param  SOPType SOP Type
  * @param  SVID    SVID used for discovery mode message
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestMode(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, uint16_t SVID);

/**
  * @brief  Called by DPM to request the PE to perform a VDM Discovery mode message on one SVID.
  * @param  PortNum   Index of current used port
  * @param  SOPType   SOP Type
  * @param  SVID      SVID used for discovery mode message
  * @param  ModeIndex Index of the mode to be entered
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestModeEnter(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, uint16_t SVID, uint8_t ModeIndex);

/**
  * @brief  Called by DPM to request the PE to perform a VDM mode exit.
  * @param  PortNum   Index of current used port
  * @param  SOPType   SOP Type
  * @param  SVID      SVID used for discovery mode message
  * @param  ModeIndex Index of the mode to be exit
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestModeExit(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, uint16_t SVID, uint8_t ModeIndex);

/**
  * @brief  Called by DPM to request the PE to send a specific SVDM message.
  * @param  PortNum Index of current used port
  * @param  SOPType Received message type based on @ref USBPD_SOPType_TypeDef
  * @param  Command VDM command based on @ref USBPD_VDM_Command_Typedef
  * @param  SVID    Used SVID
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestSpecific(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, USBPD_VDM_Command_Typedef Command, uint16_t SVID);
#endif /* USBPDCORE_SVDM || USBPDCORE_VCONN_SUPPORT */

#if defined(USBPDCORE_SVDM)
/**
  * @brief  Called by DPM to request the PE to perform a VDM Attention.
  * @param  PortNum Index of current used port
  * @param  SOPType Received message type based on @ref USBPD_SOPType_TypeDef (valid with SOP only)
  * @param  SVID    Used SVID
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_SVDM_RequestAttention(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType, uint16_t SVID);
#endif /* USBPDCORE_SVDM */

#if defined(USBPDCORE_UVDM)
/**
  * @brief  Called by DPM to request the PE to send a UVDM message.
  * @param  PortNum Index of current used port
  * @param  SOPType Received message type based on @ref USBPD_SOPType_TypeDef
  * @retval USBPD status: @ref USBPD_BUSY, @ref USBPD_OK, @ref USBPD_FAIL
  */
USBPD_StatusTypeDef USBPD_PE_UVDM_RequestMessage(uint8_t PortNum, USBPD_SOPType_TypeDef SOPType);
#endif /* USBPDCORE_UVDM */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup USBPD_CORE_PRL
  * @{
  */

/** @defgroup USBPD_CORE_PRL_Exported_Functions USBPD CORE PRL Exported Functions
  * @{
  */

/** @defgroup USBPD_CORE_PRL_Exported_Functions_Group1 USBPD CORE PRL Interface function for timer
  * @{
  */

/**
  * @brief  Decrement The PRL Timers values
  * @param  PortNum port number value
  * @retval None
  */
void USBPD_PRL_TimerCounter(uint8_t PortNum);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#if defined(USBPDCORE_TCPM_SUPPORT)
/** @addtogroup USBPD_CORE_TCPM
  * @{
  */

/** @defgroup USBPD_CORE_TCPM_Exported_Functions USBPD CORE TCPM Exported Functions
  * @{
  */

/** @defgroup USBPD_CORE_TCPM_Exported_Functions_Grp1 USBPD CORE TCPM Exported Functions to applications (DPM and PWR_IF)
  * @{
  */

/**
  * @brief  Initialize TCPC devices
  * @param  PortNum           Port number value
  * @param  TCPC_ToggleRole   Indication if TCPC should perform a role toggle or not
  * @param  Params            Pointer on PE parameters based on @ref USBPD_ParamsTypeDef
  * @param  CallbackFunctions Pointer on CAD callbacks
  * @param  TCPC_Driver       Pointer on TCPC drivers based on @ref TCPC_DrvTypeDef
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_HWInit(uint8_t PortNum, uint8_t TCPC_ToggleRole, USBPD_ParamsTypeDef *Params, USBPD_CAD_Callbacks *CallbackFunctions, TCPC_DrvTypeDef *TCPC_Driver);

/**
  * @brief  Set CC line for PD connection
  * @param  PortNum Port number value
  * @param  Pull Power role can be one of the following values:
  *         @arg @ref TYPEC_CC_RA
  *         @arg @ref TYPEC_CC_RP
  *         @arg @ref TYPEC_CC_RD
  *         @arg @ref TYPEC_CC_OPEN
  * @param  State Activation or deactivation of connection
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_set_cc(uint32_t PortNum, TCPC_CC_Pull_TypeDef Pull, USBPD_FunctionalState State);

/**
  * @brief  Enable or disable VCONN
  * @param  PortNum Port number value
  * @param  State   Activation or deactivation of VCONN
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_set_vconn(uint32_t PortNum, USBPD_FunctionalState State);

/**
  * @brief  Enable VBUS
  * @param  PortNum The port handle.
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_VBUS_Enable(uint32_t PortNum);

/**
  * @brief  Disable VBUS
  * @param  PortNum The port handle.
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_VBUS_Disable(uint32_t PortNum);

/**
  * @brief  Get VBUS voltage
  * @param  PortNum   The port handle.
  * @retval VBUS value in mV
  */
uint16_t USBPD_TCPM_VBUS_GetVoltage(uint32_t PortNum);

/**
  * @brief  Retrieve the VBUS VSafe0 status for a specified port.
  * @param  PortNum The port handle.
  * @retval USBPD_StatusTypeDef
  */
USBPD_StatusTypeDef  USBPD_TCPM_VBUS_IsVsafe0V(uint32_t PortNum);

/**
  * @brief  Retrieve the VBUS Vsafe5V status for a specified port.
  * @param  PortNum The port handle.
  * @retval USBPD_StatusTypeDef
  */
USBPD_StatusTypeDef  USBPD_TCPM_VBUS_IsVsafe5V(uint32_t PortNum);

/**
  * @brief  Management of ALERT
  * @param  Event Event reported by application
  * @retval USBPD status
  */
USBPD_StatusTypeDef  USBPD_TCPM_alert(uint32_t Event);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#endif /* USBPDCORE_TCPM_SUPPORT */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_CORE_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
