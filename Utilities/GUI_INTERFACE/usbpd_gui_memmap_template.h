/**
  ******************************************************************************
  * @file    usbpd_gui_memmap.h
  * @author  MCD Application Team
  * @brief   This file contains memory mapping configuration to be able to run
  *          Cube-Monitor-UCPD on embedded side.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) $COPYRIGHT_YEAR$ STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __USBPD_GUI_MEMMAP_H_
#define __USBPD_GUI_MEMMAP_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxxx.h"
#include "usbpd_def.h"

/** @addtogroup STM32_USBPD_APPLICATION
  * @{
  */

/** @addtogroup STM32_USBPD_APPLICATION_GUI
  * @{
  */

/* Exported typedef ----------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/

/* Start of the FLASH base address used to save the USBPD parameters */
#define INDEX_PAGE              (FLASH_PAGE_NB - 1)          /* Index of latest page                      */
#define ADDR_FLASH_LAST_PAGE    (FLASH_BASE + (INDEX_PAGE * FLASH_PAGE_SIZE))  /* Base @ of latest pages  */
#define ADDR_FLASH_PAGE_END     (ADDR_FLASH_LAST_PAGE + FLASH_PAGE_SIZE - 1) 

#define GUI_FLASH_ADDR_NB_PDO_SNK_P0  (ADDR_FLASH_LAST_PAGE)
#define GUI_FLASH_ADDR_NB_PDO_SRC_P0  (ADDR_FLASH_LAST_PAGE + 1U)
#define GUI_FLASH_ADDR_NB_PDO_SNK_P1  (ADDR_FLASH_LAST_PAGE + 2U)
#define GUI_FLASH_ADDR_NB_PDO_SRC_P1  (ADDR_FLASH_LAST_PAGE + 3U)

#define GUI_FLASH_ADDR_PDO_SRC_P0      (ADDR_FLASH_LAST_PAGE + 8U) 
/* New Address should be modulo 8 */
/* New Address = (Previous Address + (size / 8 * 8) + ((size % 8) + 7) / 8 * 8) */
#define GUI_FLASH_ADDR_PDO_SNK_P0      (GUI_FLASH_ADDR_PDO_SRC_P0 + ((USBPD_MAX_NB_PDO * 4) / 8 * 8) + ((((USBPD_MAX_NB_PDO * 4) & 0x07) + 7) / 8 * 8 ))
#define GUI_FLASH_ADDR_PDO_SRC_P1      (GUI_FLASH_ADDR_PDO_SNK_P0 + ((USBPD_MAX_NB_PDO * 4) / 8 * 8) + ((((USBPD_MAX_NB_PDO * 4) & 0x07) + 7) / 8 * 8 ))
#define GUI_FLASH_ADDR_PDO_SNK_P1      (GUI_FLASH_ADDR_PDO_SRC_P1 + ((USBPD_MAX_NB_PDO * 4) / 8 * 8) + ((((USBPD_MAX_NB_PDO * 4) & 0x07) + 7) / 8 * 8 ))

#define GUI_FLASH_ADDR_DPM_SETTINGS    (GUI_FLASH_ADDR_PDO_SNK_P1 + ((USBPD_MAX_NB_PDO * 4) / 8 * 8) + ((((USBPD_MAX_NB_PDO * 4) & 0x07) + 7) / 8 * 8 ))

/* Define by default USBPD_PORT_COUNT to 2 */
#define GUI_FLASH_ADDR_DPM_USER_SETTINGS  (GUI_FLASH_ADDR_DPM_SETTINGS + (sizeof(USBPD_SettingsTypeDef) * 2 /*USBPD_PORT_COUNT*/))

#define GUI_FLASH_ADDR_DPM_VDM_SETTINGS   (GUI_FLASH_ADDR_DPM_USER_SETTINGS + (sizeof(USBPD_USER_SettingsTypeDef) * 2 /*USBPD_PORT_COUNT*/))

#define GUI_FLASH_ADDR_DPM_ID_SETTINGS    (GUI_FLASH_ADDR_DPM_VDM_SETTINGS + (sizeof(USBPD_VDM_SettingsTypeDef) * 2 /*USBPD_PORT_COUNT*/))

#define GUI_FLASH_ADDR_RESERVED           (GUI_FLASH_ADDR_DPM_ID_SETTINGS + (sizeof(USBPD_IdSettingsTypeDef) * 2 /*USBPD_PORT_COUNT*/))

#define GUI_FLASH_SIZE_RESERVED           (ADDR_FLASH_PAGE_END - GUI_FLASH_ADDR_RESERVED)

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_GUI_MEMMAP_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
