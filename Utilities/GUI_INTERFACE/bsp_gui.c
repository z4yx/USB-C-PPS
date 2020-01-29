/**
  ******************************************************************************
  * @file    bsp_gui.c
  * @author  MCD Application Team
  * @brief   This file contains phy interface control functions.
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

/* Includes ------------------------------------------------------------------*/
#if defined(_GUI_INTERFACE)
#include "bsp_gui.h"

/* Private typedef -----------------------------------------------------------*/
/* Private Defines */
extern USBPD_SettingsTypeDef       DPM_Settings[USBPD_PORT_COUNT];
#if defined(_VDM)
extern USBPD_VDM_SettingsTypeDef   DPM_VDM_Settings[USBPD_PORT_COUNT];
#endif /* _VDM */

/* Private function prototypes -----------------------------------------------*/
static GUI_StatusTypeDef        LoadPDOFromFlash(uint32_t Address, uint32_t *pListOfPDO);
static GUI_StatusTypeDef        LoadSettingsFromFlash(uint32_t Address, uint32_t *pSettings, uint32_t Size);
static GUI_StatusTypeDef        SavePDOInFlash(uint32_t Address, uint32_t* pListOfPDO);
static GUI_StatusTypeDef        SaveSettingsInFlash(uint32_t Address, uint32_t *pSettings, uint32_t Size);

GUI_StatusTypeDef BSP_GUI_LoadDataFromFlash(void)
{
  GUI_StatusTypeDef _status = GUI_ERROR;
  uint32_t _addr = GUI_FLASH_ADDR_NB_PDO_SNK_P0;

  /* Update GUI_NbPDO? */
  if (0xFFFFFFFFu != *((uint32_t*)_addr))
  {
    uint32_t* _ptr = (uint32_t*)GUI_NbPDO;
    USPBPD_WRITE32 (_ptr,*((uint32_t*)_addr));
    _status = GUI_OK;
  }

#if defined(_SRC) || defined(_DRP)
  /* Save PORT0_PDO_ListSRC */
  _status |= LoadPDOFromFlash(GUI_FLASH_ADDR_PDO_SRC_P0, PORT0_PDO_ListSRC);
#endif /* _SRC || _DRP */

#if defined(_SNK) || defined(_DRP)
  /* Save PORT0_PDO_ListSNK */
  _status |= LoadPDOFromFlash(GUI_FLASH_ADDR_PDO_SNK_P0, PORT0_PDO_ListSNK);
#endif /* _SNK || _DRP */

#if USBPD_PORT_COUNT==2
#if defined(_SRC) || defined(_DRP)
  /* Save PORT1_PDO_ListSRC */
  _status |= LoadPDOFromFlash(GUI_FLASH_ADDR_PDO_SRC_P1, PORT1_PDO_ListSRC);
#endif /* _SRC || _DRP */

#if defined(_SNK) || defined(_DRP)
  /* Save PORT1_PDO_ListSNK */
  _status |= LoadPDOFromFlash(GUI_FLASH_ADDR_PDO_SNK_P1, PORT1_PDO_ListSNK);
#endif /* _SNK || _DRP */
#endif /* USBPD_PORT_COUNT==2 */

  /* Save DPM_Settings of port 0 */
  _status |= LoadSettingsFromFlash(GUI_FLASH_ADDR_DPM_SETTINGS, (uint32_t*)DPM_Settings, sizeof(USBPD_SettingsTypeDef) * USBPD_PORT_COUNT);

  /* Save DPM_Settings of port 0 */
  _status |= LoadSettingsFromFlash(GUI_FLASH_ADDR_DPM_USER_SETTINGS, (uint32_t*)DPM_USER_Settings, sizeof(USBPD_USER_SettingsTypeDef) * USBPD_PORT_COUNT);

#if defined(_VDM)
  /* Save DPM_Settings of port 0 */
  _status |= LoadSettingsFromFlash(GUI_FLASH_ADDR_DPM_VDM_SETTINGS, (uint32_t*)DPM_VDM_Settings, sizeof(USBPD_VDM_SettingsTypeDef) * USBPD_PORT_COUNT);
#endif /* _VDM */

  return _status;
}

GUI_StatusTypeDef BSP_GUI_SaveDataInFlash(void)
{
  GUI_StatusTypeDef status = GUI_ERROR;
  FLASH_EraseInitTypeDef erase_init;
  uint32_t page_error;

  /* Disable interrupts */
  __disable_irq();

  /* Init Flash registers for writing */
  HAL_FLASH_Unlock();

  /* Erase the page associated to the GUI parameters */
  erase_init.TypeErase  = FLASH_TYPEERASE_PAGES;
#if defined(STM32F072xB)|| defined(STM32F051x8)
  erase_init.PageAddress        = ADDR_FLASH_LAST_PAGE;
#else
  erase_init.Page       = INDEX_PAGE;
#endif /* STM32F072xB || STM32F051x8 */
  erase_init.NbPages    = 1;

#if defined(STM32F072xB)  || defined(STM32F051x8)
#else
  /* TEMPORARY */
  if(FLASH->SR != 0x00)
  {
    FLASH->SR = FLASH_SR_OPTVERR;
  }
#endif
  status = HAL_OK == HAL_FLASHEx_Erase(&erase_init, &page_error) ? GUI_OK : GUI_ERASE_ERROR;

  /* If Erase is OK, program the new data */
  if ((0xFFFFFFFF == page_error) && (GUI_OK == status))
  {
#ifdef GUI_FLASH_ADDR_NB_PDO_SNK_P0
    /* Save the nb of sink and src PDO */
    uint64_t value = 0;
    value |= GUI_NbPDO[0];
    value |= (GUI_NbPDO[1] << 8);
    value |= (GUI_NbPDO[2] << 16);
    value |= (GUI_NbPDO[3] << 24);
   status = HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, GUI_FLASH_ADDR_NB_PDO_SNK_P0, value)? GUI_OK : GUI_WRITE_ERROR;
#endif  /* GUI_FLASH_ADDR_NB_PDO_SNK_P0 */

#if defined(_SRC) || defined(_DRP)
    /* Save PORT0_PDO_ListSRC */
    if (GUI_OK == status)
    {
      status = SavePDOInFlash(GUI_FLASH_ADDR_PDO_SRC_P0, PORT0_PDO_ListSRC);
    }
#endif /* _SRC || _DRP */

#if defined(_SNK) || defined(_DRP)
    /* Save PORT0_PDO_ListSNK */
    if (GUI_OK == status)
    {
      status = SavePDOInFlash(GUI_FLASH_ADDR_PDO_SNK_P0, PORT0_PDO_ListSNK);
    }
#endif /* _SNK || _DRP */

#if USBPD_PORT_COUNT==2
#if defined(_SRC) || defined(_DRP)
        /* Save PORT1_PDO_ListSRC */
        if (GUI_OK == status)
        {
          status = SavePDOInFlash(GUI_FLASH_ADDR_PDO_SRC_P1, PORT1_PDO_ListSRC);
        }
#endif /* _SRC || _DRP */

#if defined(_SNK) || defined(_DRP)
        /* Save PORT1_PDO_ListSNK */
        if (GUI_OK == status)
        {
          status = SavePDOInFlash(GUI_FLASH_ADDR_PDO_SNK_P1, PORT1_PDO_ListSNK);
        }
#endif /* _SNK || _DRP */
#endif /* USBPD_PORT_COUNT==2 */

    /* Save DPM_Settings of port 0 */
    if (GUI_OK == status)
    {
      status = SaveSettingsInFlash(GUI_FLASH_ADDR_DPM_SETTINGS, (uint32_t*)DPM_Settings, sizeof(USBPD_SettingsTypeDef));
    }

    /* Save DPM_Settings of port 0 */
    if (GUI_OK == status)
    {
      status = SaveSettingsInFlash(GUI_FLASH_ADDR_DPM_USER_SETTINGS, (uint32_t*)DPM_USER_Settings, sizeof(USBPD_USER_SettingsTypeDef));
    }

#if defined(_VDM)
    /* Save DPM_Settings of port 0 */
    if (GUI_OK == status)
    {
      status = SaveSettingsInFlash(GUI_FLASH_ADDR_DPM_VDM_SETTINGS, (uint32_t*)DPM_VDM_Settings, sizeof(USBPD_VDM_SettingsTypeDef));
    }
#endif /* _VDM */
  }

  /* Lock the flash afer end of operations */
  HAL_FLASH_Lock();

  /* Disable interrupts */
  __enable_irq();

  return status;
}

static GUI_StatusTypeDef SavePDOInFlash(uint32_t Address, uint32_t *pListOfPDO)
{
  uint64_t data_in_64;
  uint32_t index, index_flash;
  uint32_t value[2];
  GUI_StatusTypeDef status = GUI_OK;

  for (index = 0, index_flash = 0; ((index < USBPD_MAX_NB_PDO) && (GUI_OK == status)); index++, index_flash++)
  {
    value[0] = pListOfPDO[index];
    index++;
    if (index < USBPD_MAX_NB_PDO)
    {
      value[1] = pListOfPDO[index];
    }
    else
    {
      value[1] = (0xFFFFFFFF);
    }

    data_in_64 = value[0] | (uint64_t)value[1] << 32;

    /* Save in the FLASH */
    status = HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (Address + (8 * index_flash)) , data_in_64) ? GUI_OK : GUI_WRITE_ERROR;
  }
  return status;
}

static GUI_StatusTypeDef SaveSettingsInFlash(uint32_t Address, uint32_t *pSettings, uint32_t Size)
{
  uint64_t data_in_64;
  uint32_t index, index_flash;
  uint32_t value[2];
  GUI_StatusTypeDef status = GUI_OK;

  uint32_t nb_double = ((Size * USBPD_PORT_COUNT) / 4);
  uint8_t remaining = ((Size * USBPD_PORT_COUNT) % 4);

  /* Save Settings in the FLASH */
  for (index = 0, index_flash = 0; ((index < nb_double) && (GUI_OK == status)); index++, index_flash++)
  {
    value[0] = pSettings[index];
    index++;
    if (index < nb_double)
    {
      value[1] = pSettings[index];
    }
    else
    {
      if (0 == remaining)
        value[1] = (0xFFFFFFFF);
      else
        while(1);
    }

    data_in_64 = value[0] | (uint64_t)value[1] << 32;

    /* Save in the FLASH */
    status = HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (Address + (8 * index_flash)) , data_in_64)? GUI_OK : GUI_WRITE_ERROR;
  }
  return status;
}

static GUI_StatusTypeDef LoadPDOFromFlash(uint32_t Address, uint32_t *pListOfPDO)
{
  uint32_t _addr = Address;
  GUI_StatusTypeDef _status = GUI_ERROR;

  /* Check if FLASH is not empty to retrieve the data. Nethertheless keep data in the RAM */
  if (0xFFFFFFFFu != *((uint32_t*)_addr))
  {
    uint32_t _index;
    for (_index = 0; _index < USBPD_MAX_NB_PDO; _index++)
    {
      pListOfPDO[_index] = *((uint32_t*)_addr);
      _addr = _addr + 4u;
    }
    _status = GUI_OK;
  }
  return _status;
}

static GUI_StatusTypeDef LoadSettingsFromFlash(uint32_t Address, uint32_t *pSettings, uint32_t Size)
{
  uint32_t _addr = Address;
  GUI_StatusTypeDef _status = GUI_ERROR;

  /* Check if FLASH is not empty to retrieve the data. Nethertheless keep data in the RAM */
  if (0xFFFFFFFFu != *((uint32_t*)_addr))
  {
    memcpy(pSettings, ((uint32_t*)_addr), Size);
    _status = GUI_OK;
  }
  return _status;
}
#endif /* _GUI_INTERFACE */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
