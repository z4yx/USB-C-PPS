/**
 ******************************************************************************
 * @file    demo_disco.c
 * @author  MCD Application Team
 * @brief   demo application code
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbpd_core.h"
#include "usbpd_dpm_core.h"
#include "usbpd_dpm_conf.h"
#include "usbpd_pwr_if.h"
#include "usbpd_pwr_if.h"
#include "stm32g0xx_ll_ucpd.h"
#include "usbpd_hw.h"
#include "demo_disco.h"
#include "ucdc.h"
#include "string.h"
#include "common.h"
#include "cmsis_os.h"

/* Exported variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef enum {
  DETACHED,
  ATTACHED,
} DEMO_STATE;

typedef struct {
  uint8_t SOPx;
  uint8_t size;
  uint8_t data[30];
} USBPD_Msg;

/** @defgroup USBPD_MsgHeaderStructure_definition USB PD Message header
 * Structure definition
 * @brief USB PD Message header Structure definition
 * @{
 */
typedef union {
  uint16_t d16;
  struct {
    uint16_t MessageType : /*!< Message Header's message Type */
                           5;
    uint16_t PortDataRole : /*!< Message Header's Port Data Role */
                            1;
    uint16_t SpecificationRevision : /*!< Message Header's Spec Revision */
                                     2;
    uint16_t PortPowerRole_CablePlug
        : /*!< Message Header's Port Power Role/Cable Plug field  */
          1;
    uint16_t MessageID : /*!< Message Header's message ID */
                         3;
    uint16_t NumberOfDataObjects
        : /*!< Message Header's Number of data object             */
          3;
    uint16_t Extended : /*!< Reserved */
                        1;
  } b;
} DEMO_MsgHeader_TypeDef;

/* Private define ------------------------------------------------------------*/
#define STR_SIZE_MAX 18

/* Format of the different kind of message */
/*   - 31-28  4 bit for the message type   */
#define DEMO_MSG_TYPE_POS 28u
#define DEMO_MSG_TYPE_MSK (0xFu << DEMO_MSG_TYPE_POS)
#define DEMO_MSG_MMI (0u << DEMO_MSG_TYPE_POS)
#define DEMO_MSG_CAD (1u << DEMO_MSG_TYPE_POS)
#define DEMO_MSG_PENOTIFY (2u << DEMO_MSG_TYPE_POS)
#define DEMO_MSG_GETINFO (3u << DEMO_MSG_TYPE_POS)

/*  CAD
     - 27-26 2 bit for CC Pin
     - 25-24 2 bit port num
     - 15-00 16 bit CAD event
*/
#define DEMO_CAD_CC_NUM_Pos (26u)
#define DEMO_CAD_CC_NUM_Msk (0x3u << DEMO_CAD_CC_NUM_Pos)
#define DEMO_CAD_CC_NUM_NONE (0u << DEMO_CAD_CC_NUM_Pos)
#define DEMO_CAD_CC_NUM_CC1 (1u << DEMO_CAD_CC_NUM_Pos)
#define DEMO_CAD_CC_NUM_CC2 (2u << DEMO_CAD_CC_NUM_Pos)

#define DEMO_CAD_PORT_NUM_Pos (24u)
#define DEMO_CAD_PORT_NUM_Msk (0x3u << DEMO_CAD_PORT_NUM_Pos)
#define DEMO_CAD_PORT_NUM_0 (0x0u << DEMO_CAD_PORT_NUM_Pos)
#define DEMO_CAD_PORT_NUM_1 (0x1u << DEMO_CAD_PORT_NUM_Pos)

#define DEMO_CAD_STATE_Pos (0u)
#define DEMO_CAD_STATE_Msk (0xFFu << DEMO_CAD_STATE_Pos)

/*  PE
     - 27-26 2 bit none
     - 25-24 2 bit port num
     - 15-00 16 bit PE event
*/
#define DEMO_MSG_PORT_NUM_Pos (24u)
#define DEMO_MSG_PORT_NUM_Msk (0x3u << DEMO_MSG_PORT_NUM_Pos)
#define DEMO_MSG_PORT_NUM_0 (0x0u << DEMO_MSG_PORT_NUM_Pos)
#define DEMO_MSG_PORT_NUM_1 (0x1u << DEMO_MSG_PORT_NUM_Pos)

#define DEMO_MSG_DATA_Pos (0u)
#define DEMO_MSG_DATA_Msk (0xFFu << DEMO_MSG_DATA_Pos)

/* GET INFO Message */
#define DEMO_MSG_GETINFO_SNKCAPA (0x1u << DEMO_MSG_DATA_Pos)
#define DEMO_MSG_GETINFO_SVID (0x2u << DEMO_MSG_DATA_Pos)

#define MAX_LINE_PDO 4u
#define MAX_LINE_COMMAND 4u

typedef enum {
  MENU_PD_SPEC,
  MENU_MEASURE,
  MENU_SELECT_SOURCECAPA,
  MENU_VOLT_ADJUST,
  MENU_INVALID,
} DEMO_MENU;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t indexPDO = 0;
static uint32_t millivoltAPDO = 0;
static uint8_t pe_disabled = 0; /* 0 by default, 1 if PE is disabled (in case of
                                   no PD device attached) */

/*  contains the index of the selected item inside a menu*/
static uint8_t g_tab_menu_sel = {0};
/*  contains the menu selected position  */
static int8_t g_tab_menu_pos = {0};

/* Timer handler */
TimerHandle_t xTimers;

/* Private variables ---------------------------------------------------------*/
#define MAX_MSG_SIZE 4
#define MAX_MSG_LENGHT 18

osMessageQId DemoEvent;

/* timer for demo display */
uint32_t demo_timeout;

/* Private function prototypes -----------------------------------------------*/

static void Display_clear(void);
static void string_completion(uint8_t *str);

static DEMO_MENU Display_sourcecapa_menu_nav(uint8_t Or);
static uint8_t Display_sourcecapa_menu_exec(void);

static void Display_menu_version(void);

void DEMO_Task_Standalone(void const *);
static void DEMO_PostGetInfoMessage(uint8_t PortNum, uint16_t GetInfoVal);

void vTimerCallback(TimerHandle_t xTimer);

uint8_t DEMO_IsSpyMode(void) { return 0; }

/**
 * @brief  Demo BSP initialisation
 * @retval DEMO_ErrorCode
 */
DEMO_ErrorCode DEMO_InitBSP(void) {
  BSP_LED_Init(LED_ORANGE);
  BSP_JOY_Init(JOY_MODE_GPIO);

  /*##-1- Initialize the LCD #################################################*/
  /* Initialize the LCD */
  BSP_LCD_Init();
  BSP_LCD_SetFont(&Font12);

  return DEMO_OK;
}

DEMO_ErrorCode DEMO_InitTask(DEMO_MODE mode) {
  osThreadDef(STD, DEMO_Task_Standalone, osPriorityAboveNormal, 0, 1000);

  osMessageQDef(DemoEvent, 30, uint32_t);
  DemoEvent = osMessageCreate(osMessageQ(DemoEvent), NULL);

  osThreadCreate(osThread(STD), &DemoEvent);

  return DEMO_OK;
}

/**
 * @brief  UserCableDetection reporting events on a specified port from CAD
 * layer.
 * @param  PortNum   The handle of the port
 * @param  State     CAD state
 * @param  Cc        The Communication Channel for the USBPD communication
 * @retval None
 */
void DEMO_PostCADMessage(uint8_t PortNum, USBPD_CAD_EVENT State,
                         CCxPin_TypeDef Cc) {
  uint32_t event =
      (DEMO_MSG_CAD | (Cc << DEMO_CAD_CC_NUM_Pos |
                       (PortNum << DEMO_CAD_PORT_NUM_Pos) | State));
  osMessagePut(DemoEvent, event, 0);
}

/**
 * @brief  Callback function called by PE to inform DPM about PE event.
 * @param  PortNum   The handle of the port
 * @param  EventVal @ref USBPD_NotifyEventValue_TypeDef
 * @retval None
 */
void DEMO_PostNotificationMessage(uint8_t PortNum,
                                  USBPD_NotifyEventValue_TypeDef EventVal) {
  uint32_t event =
      DEMO_MSG_PENOTIFY | PortNum << DEMO_MSG_PORT_NUM_Pos | EventVal;
  osMessagePut(DemoEvent, event, 0);
}

/**
 * @brief  function called by MMI to order a PE request.
 * @param  PortNum   The handle of the port
 * @param  GetInfoVal
 * @retval None
 */
static void DEMO_PostGetInfoMessage(uint8_t PortNum, uint16_t GetInfoVal) {
  uint32_t event =
      DEMO_MSG_GETINFO | PortNum << DEMO_MSG_PORT_NUM_Pos | GetInfoVal;
  osMessagePut(DemoEvent, event, 0);
}

/**
 * @brief  Callback function called by PE to inform DPM about PE event.
 * @param  EventVal @ref USBPD_NotifyEventValue_TypeDef
 * @retval None
 */
void DEMO_PostMMIMessage(uint32_t EventVal) {
  uint32_t event = DEMO_MSG_MMI | EventVal;
  osMessagePut(DemoEvent, event, 0);
}

/**
 * @brief  check_cc_attached
 * @param  CC1Voltage   CC1 voltage
 * @param  CC2Voltage   CC2 voltage
 * @retval CC1 CC2 or CCNONE
 */
char str[40];

/**
 * @brief  Display clear info
 * @retval None
 */
static void Display_clear(void) { BSP_LCD_Clear(LCD_COLOR_BLACK); }

/**
 * @brief  Menu selection managment
 * @param  IndexMax    MAX index
 * @param  LineMax     MAX line
 * @param  Start       Pointer on Start
 * @param  End         Pointer on end
 * @param  Orientation Orientation (DEMO_MMI_ACTION_UP_PRESS,
 * DEMO_MMI_ACTION_DOWN_PRESS)
 * @retval None
 */
static void Menu_manage_selection(uint8_t IndexMax, uint8_t LineMax,
                                  uint8_t *Start, uint8_t *End,
                                  int8_t Orientation) {
  *Start = 0;
  *End = *Start + LineMax;

  if ((Orientation != DEMO_MMI_ACTION_UP_PRESS &&
       Orientation != DEMO_MMI_ACTION_DOWN_PRESS) ||
      (IndexMax == 0)) {
    *Start = 0;
    *End = (LineMax > IndexMax) ? IndexMax : LineMax;
    g_tab_menu_pos = g_tab_menu_sel = 0;
    return;
  }

  if (DEMO_MMI_ACTION_UP_PRESS == Orientation) {
    if (0 == g_tab_menu_sel) {
      /* got at the end of the selection */
      g_tab_menu_pos = LineMax > IndexMax ? IndexMax - 1 : LineMax - 1;
      g_tab_menu_sel = IndexMax - 1;
      *End = IndexMax;
      if (IndexMax > LineMax) {
        *Start = IndexMax - LineMax;
      } else {
        *Start = 0;
      }
    } else {
      if (0 == g_tab_menu_pos) {
        g_tab_menu_sel--;
        *Start = g_tab_menu_sel;
        if (IndexMax > LineMax)
          *End = *Start + LineMax;
        else
          *End = *Start + IndexMax;
      } else {
        *Start = g_tab_menu_sel - g_tab_menu_pos;
        if (IndexMax > LineMax)
          *End = g_tab_menu_sel + LineMax - g_tab_menu_pos;
        else
          *End = g_tab_menu_sel + IndexMax - g_tab_menu_pos;

        g_tab_menu_pos--;
        g_tab_menu_sel--;
      }
    }
  } else /* +1 == orientation */
  {
    if (((LineMax - 1) == g_tab_menu_pos) ||
        (((IndexMax - 1) == g_tab_menu_pos) &&
         (LineMax >=
          IndexMax))) { /* at the end of selection, so two case there is others
                           data or at the end of the data */
      if (g_tab_menu_sel == (IndexMax - 1)) {
        /* restart at the begin of the list */
        g_tab_menu_pos = 0;
        g_tab_menu_sel = 0;
        *Start = 0;
        *End = LineMax > IndexMax ? IndexMax : LineMax;
      } else {
        *Start = g_tab_menu_sel - (LineMax - 1) + 1;
        *End = *Start + LineMax;
        g_tab_menu_sel++;
      }
    } else {
      *Start = g_tab_menu_sel - g_tab_menu_pos;
      if (IndexMax > LineMax)
        *End = *Start + LineMax;
      else
        *End = *Start + IndexMax;
      g_tab_menu_sel++;
      g_tab_menu_pos++;
    }
  }
}

static DEMO_MENU Display_adj_menu_nav(uint8_t Nav) {
  const int16_t adjustment_value[] = {
      100, 10,
      2, // minimum granularity of PPS is 20mV
  };
  const char *const cursor[] = {
      " ^      ",
      "   ^    ",
      "    ^   ",
  };
  static uint8_t adjustment_digit = 0;
  static uint32_t _cur_mv = 0;
  uint8_t _str[30];
  uint16_t new_centivolt = 0;

  if (!_cur_mv)
    _cur_mv = millivoltAPDO; // set to currently applied value

  if (Nav == DEMO_MMI_ACTION_LEFT_PRESS) {
    if (adjustment_digit == 0) {
      _cur_mv = 0;
      return MENU_MEASURE;
    } else
      adjustment_digit--;

  } else if (Nav == DEMO_MMI_ACTION_RIGHT_PRESS) {
    if (adjustment_digit == 2) {

      // turn off load before applying new settings
      BSP_LED_Off(LED_ORANGE);
      USBPD_StatusTypeDef status =
          UCDC_Request_Output(indexPDO, _cur_mv / 10); // apply settings
      if (status == USBPD_OK)
        millivoltAPDO = _cur_mv;

      _cur_mv = 0;
      adjustment_digit = 0;
      return MENU_MEASURE;
    } else
      adjustment_digit++;

  } else if (Nav == DEMO_MMI_ACTION_UP_PRESS) {
    new_centivolt = UCDC_Search_Next_Voltage(
        _cur_mv / 10 + adjustment_value[adjustment_digit], 1, &indexPDO);

  } else if (Nav == DEMO_MMI_ACTION_DOWN_PRESS) {
    new_centivolt = UCDC_Search_Next_Voltage(
        _cur_mv / 10 - adjustment_value[adjustment_digit], 0, &indexPDO);
  }
  if (new_centivolt != 0) // valid value
  {
    _cur_mv = new_centivolt * 10;
  }

  BSP_LCD_SetFont(&Font20);
  sprintf((char *)str, "%2d.%02d  V", (int)_cur_mv / 1000,
          (int)_cur_mv % 1000 / 10);
  BSP_LCD_DisplayStringAt(0, 0 * 16 + 10, (uint8_t *)str, CENTER_MODE);
  strcpy((char *)str, cursor[adjustment_digit]);
  BSP_LCD_DisplayStringAt(0, 1 * 16 + 10, (uint8_t *)str, CENTER_MODE);
  return MENU_INVALID;
}

static void Display_adj_menu(void) {
  Display_adj_menu_nav(DEMO_MMI_ACTION_NONE);
}

/**
 * @brief  src capa menu display
 * @retval None
 */
static void Display_sourcecapa_menu(void) {

  /* Display menu source capa */
  {
    uint8_t _str[20];
    sprintf((char *)_str, "Select the profile:");
    string_completion(_str);
    BSP_LCD_DisplayStringAtLine(0, _str);
  }

  Display_sourcecapa_menu_nav(DEMO_MMI_ACTION_NONE);
}

/**
 * @brief  src capa menu navigation
 * @param  Nav (DEMO_MMI_ACTION_UP_PRESS, DEMO_MMI_ACTION_DOWN_PRESS)
 * @retval None
 */
static DEMO_MENU Display_sourcecapa_menu_nav(uint8_t Nav) {
  uint8_t _str[30];
  uint8_t _max = 0;
  uint8_t _start, _end, _pos = 0, _i_pdo = 0;
  uint32_t _apdo_mv;

  if (Nav == DEMO_MMI_ACTION_LEFT_PRESS)
    return MENU_MEASURE;

  for (int8_t index = 0; index < DPM_Ports[0].DPM_NumberOfRcvSRCPDO; index++)
    if (USBPD_PDO_TYPE_FIXED ==
            (DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] & USBPD_PDO_TYPE_Msk) ||
        USBPD_PDO_TYPE_APDO ==
            (DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] & USBPD_PDO_TYPE_Msk))
      _max++;

  Menu_manage_selection(_max, MAX_LINE_PDO, &_start, &_end, Nav);

  for (int8_t index = 0; index < DPM_Ports[0].DPM_NumberOfRcvSRCPDO; index++) {

    if (USBPD_PDO_TYPE_FIXED ==
        (DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] & USBPD_PDO_TYPE_Msk)) {
      uint32_t maxcurrent = ((DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] &
                              USBPD_PDO_SRC_FIXED_MAX_CURRENT_Msk) >>
                             USBPD_PDO_SRC_FIXED_MAX_CURRENT_Pos) *
                            10;
      uint32_t maxvoltage = ((DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] &
                              USBPD_PDO_SRC_FIXED_VOLTAGE_Msk) >>
                             USBPD_PDO_SRC_FIXED_VOLTAGE_Pos) *
                            50;
      sprintf((char *)_str, "     % 2luV    %lu.%luA", maxvoltage / 1000,
              maxcurrent / 1000, (maxcurrent % 1000 / 100));
    } else if (USBPD_PDO_TYPE_APDO ==
               (DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] & USBPD_PDO_TYPE_Msk)) {
      uint32_t maxcurrent = 50 * ((DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] &
                                   USBPD_PDO_SRC_APDO_MAX_CURRENT_Msk) >>
                                  USBPD_PDO_SRC_APDO_MAX_CURRENT_Pos);
      uint32_t minvoltage = 100 * ((DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] &
                                    USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Msk) >>
                                   USBPD_PDO_SRC_APDO_MIN_VOLTAGE_Pos);
      uint32_t maxvoltage = 100 * ((DPM_Ports[0].DPM_ListOfRcvSRCPDO[index] &
                                    USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Msk) >>
                                   USBPD_PDO_SRC_APDO_MAX_VOLTAGE_Pos);
      sprintf((char *)_str, "% 2lu.%luV~% 2lu.%luV %lu.%luA", minvoltage / 1000,
              (minvoltage % 1000 / 100), maxvoltage / 1000,
              (maxvoltage % 1000 / 100), maxcurrent / 1000,
              (maxcurrent % 1000 / 100));
      _apdo_mv = minvoltage;
    } else
      continue;

    if (_start <= _i_pdo && _i_pdo < _end) {
      if ((_i_pdo - _start) == g_tab_menu_pos) {
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      }
      string_completion(_str);
      BSP_LCD_DisplayStringAtLine(1 + _pos++, (uint8_t *)_str);

      if ((_i_pdo - _start) == g_tab_menu_pos) {
        BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

        indexPDO = index;
        millivoltAPDO = _apdo_mv;
      }
    }
    _i_pdo++;
  }
  return MENU_INVALID;
}

static DEMO_MENU Display_measure_menu_nav(uint8_t Nav) {
  DEMO_MENU ret = MENU_INVALID;
  if (Nav == DEMO_MMI_ACTION_UP_PRESS)
    BSP_LED_On(LED_ORANGE);
  else if (Nav == DEMO_MMI_ACTION_DOWN_PRESS)
    BSP_LED_Off(LED_ORANGE);
  else if (Nav == DEMO_MMI_ACTION_RIGHT_PRESS)
    ret = MENU_VOLT_ADJUST;
  return ret;
}

static void Display_pd_spec_menu(void) {
  char str[12];

  /* Menu command display */
  BSP_LCD_SetFont(&Font16);

  /* Display menu command */
  if ((DPM_Ports[0].DPM_NumberOfRcvSRCPDO ==
       0)) /* VBUS is there, but no source capa received yet */
  {
    if (pe_disabled == 1) /* i.e. if we received notification that the attached
                             device is not a PD device */
    {
      BSP_LCD_DisplayStringAt(0, 1 * 20, (uint8_t *)"USBPD not", CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, 2 * 20, (uint8_t *)"supported", CENTER_MODE);
    } else /* in case we are trying to communicate to attached device : some
              HARD RESET may be on going */
    {
      BSP_LCD_DisplayStringAt(0, 1 * 20, (uint8_t *)"Analysis", CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, 2 * 20, (uint8_t *)"on going", CENTER_MODE);
    }
  } else if (DPM_Ports[0].DPM_NumberOfRcvSRCPDO > 0) {
    BSP_LCD_DisplayStringAt(0, 1 * 20, (uint8_t *)"USB Type-C", CENTER_MODE);
    sprintf((char *)str, "PD%s capable",
            DPM_Params[0].PE_SpecRevision == USBPD_SPECIFICATION_REV3 ? "3"
                                                                      : "2");
    BSP_LCD_DisplayStringAt(0, 2 * 20, (uint8_t *)str, CENTER_MODE);
  } else {
    BSP_LCD_DisplayStringAt(0, 1 * 20, (uint8_t *)"USBPD not", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 2 * 20, (uint8_t *)"supported", CENTER_MODE);
  }
}

static void Display_measure_menu(void) {
  char str[20];

  /* Vbus measure value */
  uint32_t _vbusVoltage = 0;
  int32_t _vbusCurrent = 0;

  BSP_LCD_SetFont(&Font20);

  BSP_PWRMON_GetVoltage(ALERT_VBUS, &_vbusVoltage);
  BSP_PWRMON_GetCurrent(ALERT_VBUS, &_vbusCurrent);

  if (_vbusCurrent < 0) {
    _vbusCurrent = -_vbusCurrent;
  }

  /* Display measure voltage, current and power */
  sprintf((char *)str, "%2d.%03d V", (int)_vbusVoltage / 1000,
          (int)_vbusVoltage % 1000);
  BSP_LCD_DisplayStringAt(0, 0 * 16 + 10, (uint8_t *)str, CENTER_MODE);

  sprintf((char *)str, "%2d.%03d A", (int)_vbusCurrent / 1000,
          (int)_vbusCurrent % 1000);
  BSP_LCD_DisplayStringAt(0, 1 * 16 + 10, (uint8_t *)str, CENTER_MODE);

  // sprintf((char *)str, "%2d.%03d W",(int)(_vbusVoltage * _vbusCurrent)/
  // 1000000,(int)((_vbusVoltage * _vbusCurrent)%1000000)/1000);
  // BSP_LCD_DisplayStringAt(0,2*16+10, (uint8_t *)str, CENTER_MODE);

  if (!pe_disabled && DPM_Ports[0].DPM_NumberOfRcvSRCPDO > 0) {
    sprintf((char *)str, "USB PD%s",
            DPM_Params[0].PE_SpecRevision == USBPD_SPECIFICATION_REV3 ? "3"
                                                                      : "2");
    BSP_LCD_DisplayStringAt(0, 2 * 16 + 10, (uint8_t *)str, CENTER_MODE);
  } else {
    BSP_LCD_DisplayStringAt(0, 2 * 16 + 10, (uint8_t *)"No USB-PD",
                            CENTER_MODE);
  }
}

/**
 * @brief  manage menu display
 * @param  MenuSel
 * @retval None
 */
static void Display_menuupdate_info(DEMO_MENU MenuSel) {
  BSP_LCD_SetFont(&Font12);

  Display_clear();
  switch (MenuSel) {
  case MENU_PD_SPEC: /* Display menu command */
    Display_pd_spec_menu();
    break;
  case MENU_MEASURE: /* Display menu command */
    Display_measure_menu();
    break;
  case MENU_SELECT_SOURCECAPA: /* Display menu source capa */
    Display_sourcecapa_menu();
    break;
  case MENU_VOLT_ADJUST:
    Display_adj_menu();
    break;
  }
  BSP_LCD_SetFont(&Font12);
}

/**
 * @brief  src capa menu exec
 * @retval None
 */
uint8_t Display_sourcecapa_menu_exec(void) {
  USBPD_SNKRDO_TypeDef rdo;
  USBPD_PDO_TypeDef pdo;
  USBPD_StatusTypeDef ret = USBPD_FAIL;

  // turn off load before applying new settings
  BSP_LED_Off(LED_ORANGE);

  pdo.d32 = DPM_Ports[USBPD_PORT_0].DPM_ListOfRcvSRCPDO[indexPDO];
  DBG_MSG("selected PDO %u, type=%#x\r\n", indexPDO,
          pdo.GenericPDO.PowerObject);

  ret = UCDC_Request_Output(indexPDO, millivoltAPDO / 10);
  if (ret == USBPD_OK) {
    DBG_MSG("successfully applied\r\n");
  } else {
    ERR_MSG("failed with %d\r\n", ret);
  }
}

/**
 * @brief  manage menu selection
 * @param  MenuSel
 * @param  Nav (up, down, left, right)
 * @retval None
 */
static DEMO_MENU Display_menunav_info(uint8_t MenuSel, uint8_t Nav) {
  DEMO_MENU ret = MENU_INVALID;
  BSP_LCD_SetFont(&Font12);
  switch (MenuSel) {
  default:
    break;
  case MENU_MEASURE:
    ret = Display_measure_menu_nav(Nav);
    break;
  case MENU_SELECT_SOURCECAPA: /* Display menu source capa */
    ret = Display_sourcecapa_menu_nav(Nav);
    break;
  case MENU_VOLT_ADJUST:
    ret = Display_adj_menu_nav(Nav);
    break;
  }
  BSP_LCD_SetFont(&Font12);
  return ret;
}

/**
 * @brief  manage menu execution
 * @param  MenuSel
 * @retval None
 */
static DEMO_MENU Display_menuexec_info(uint8_t MenuSel) {
  switch (MenuSel) {
  default:
    break;
  case MENU_MEASURE:
    return MENU_SELECT_SOURCECAPA;
  case MENU_SELECT_SOURCECAPA: /* Display menu source capa */
    Display_sourcecapa_menu_exec();
    return MENU_MEASURE;
  case MENU_VOLT_ADJUST:
    return MENU_MEASURE;
  }
  return MENU_INVALID;
}

/**
 * @brief  main demo function to manage all the appplication event and to update
 * MMI in standalone mode
 * @param  Event
 * @retval None
 */
static void DEMO_Manage_event(uint32_t Event) {
  static DEMO_MENU _tab_menu_val = MENU_PD_SPEC;
  static uint8_t _tab_connect_status = 0;

  switch (Event & DEMO_MSG_TYPE_MSK) {
  case DEMO_MSG_MMI: {
    if ((Event & DEMO_MMI_ACTION_Msk) != DEMO_MMI_ACTION_NONE)
      demo_timeout = HAL_GetTick();
    switch (Event & DEMO_MMI_ACTION_Msk) {
    case DEMO_MMI_ACTION_NONE:
      if (_tab_connect_status != 0) {
        if (((MENU_MEASURE == _tab_menu_val))) {
          Display_menuupdate_info(_tab_menu_val);
        }
        if (DPM_Params[0].PE_Power != USBPD_POWER_NO) {
          if (HAL_GetTick() - demo_timeout > 20000) {
            _tab_menu_val = MENU_MEASURE;
            demo_timeout = HAL_GetTick();
            Display_menuupdate_info(_tab_menu_val);
          }
        }
      }
      break;
    case DEMO_MMI_ACTION_RIGHT_PRESS:
    case DEMO_MMI_ACTION_LEFT_PRESS:
    case DEMO_MMI_ACTION_UP_PRESS:
    case DEMO_MMI_ACTION_DOWN_PRESS: {
      DEMO_MENU next_menu =
          Display_menunav_info(_tab_menu_val, Event & DEMO_MMI_ACTION_Msk);
      if (next_menu != MENU_INVALID &&
          _tab_menu_val != next_menu) { /* If action successfull */
        _tab_menu_val = next_menu;
        Display_menuupdate_info(_tab_menu_val);
      }
    } break;
    case DEMO_MMI_ACTION_SEL_PRESS: {
      DEMO_MENU next_menu = Display_menuexec_info(_tab_menu_val);
      if (next_menu != MENU_INVALID) { /* If action successfull */
        _tab_menu_val = next_menu;
        Display_menuupdate_info(_tab_menu_val);
      }
    } break;
    case DEMO_MMI_ACTION_SEL_LONGPRESS:
      break;
    }
  } break;
  case DEMO_MSG_CAD: {
    switch ((Event & DEMO_CAD_STATE_Msk) >> DEMO_CAD_STATE_Pos) {
    case USBPD_CAD_EVENT_ATTEMC:
    case USBPD_CAD_EVENT_ATTACHED:
      _tab_connect_status = 1;
      _tab_menu_val = MENU_PD_SPEC;
      demo_timeout = HAL_GetTick();
      Display_menuupdate_info(_tab_menu_val);
      break;
    case USBPD_CAD_EVENT_DETACHED:
      _tab_connect_status = 0;
      pe_disabled =
          0; /* reset PE status information for no PD device attached */
      _tab_menu_val = MENU_PD_SPEC;
      Display_menuupdate_info(_tab_menu_val);

      BSP_LED_Off(LED_ORANGE);
      break;
    }
  } break;
  case DEMO_MSG_GETINFO: {
    switch ((Event & DEMO_MSG_DATA_Msk)) {
    case DEMO_MSG_GETINFO_SNKCAPA: {
      if (0 != _tab_connect_status) {
        if (USBPD_OK ==
            USBPD_PE_Request_CtrlMessage(0, USBPD_CONTROLMSG_GET_SNK_CAP,
                                         USBPD_SOPTYPE_SOP)) {
          /* Request has accepted so switch to the next request */
          // DEMO_PostGetInfoMessage(0, DEMO_MSG_GETINFO_SVID);
        } else {
          /* Request has not been accept by the stack so retry to send a message
           */
          DEMO_PostGetInfoMessage(0, DEMO_MSG_GETINFO_SNKCAPA);
        }
      }
      break;
    }
    case DEMO_MSG_GETINFO_SVID: {
      if (0 != _tab_connect_status) {
        if (USBPD_OK !=
            USBPD_DPM_RequestVDM_DiscoverySVID(0, USBPD_SOPTYPE_SOP)) {
          /* Request has not been accept by the stack so retry to send a message
           */
          DEMO_PostGetInfoMessage(0, DEMO_MSG_GETINFO_SVID);
        }
      }
      break;
    }
    }
    break;
  }
  case DEMO_MSG_PENOTIFY: {
    switch ((Event & DEMO_MSG_DATA_Msk) >> DEMO_MSG_DATA_Pos) {
    case USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT:
      if (_tab_connect_status == 1) {
        _tab_menu_val = MENU_MEASURE;
        Display_menuupdate_info(_tab_menu_val);
        xTimerStart(xTimers, 0);
        _tab_connect_status = 2;
      }
      break;
    case USBPD_NOTIFY_HARDRESET_RX: {
      pe_disabled =
          0; /* reset PE state information in case of no PD device attached */
      _tab_menu_val = MENU_PD_SPEC;
      Display_menuupdate_info(_tab_menu_val);
      break;
    }
    case USBPD_NOTIFY_PE_DISABLED: {
      pe_disabled =
          1; /* means that attached device is not PD : PE is disabled */
      _tab_menu_val = MENU_MEASURE;
      Display_menuupdate_info(_tab_menu_val);
      break;
    }
    case USBPD_NOTIFY_VDM_SVID_RECEIVED:
    case USBPD_NOTIFY_HARDRESET_TX:
    case USBPD_NOTIFY_SNK_GOTOMIN:
    case USBPD_NOTIFY_SNK_GOTOMIN_READY:
    case USBPD_NOTIFY_SOFTRESET_RECEIVED:
    case USBPD_NOTIFY_PING_RECEIVED:
    case USBPD_NOTIFY_GETSRCCAP_ACCEPTED:
    case USBPD_NOTIFY_GETSRCCAP_REJECTED:
    case USBPD_NOTIFY_GETSNKCAP_TIMEOUT:
    case USBPD_NOTIFY_REQUEST_ACCEPTED:
    case USBPD_NOTIFY_REQUEST_REJECTED:
    case USBPD_NOTIFY_REQUEST_WAIT:
    case USBPD_NOTIFY_REQUEST_GOTOMIN:
    case USBPD_NOTIFY_POWER_SWAP_TO_SNK_DONE:
    case USBPD_NOTIFY_POWER_SWAP_TO_SRC_DONE:
    case USBPD_NOTIFY_POWER_SWAP_REJ:
    case USBPD_NOTIFY_DATAROLESWAP_UFP:
    case USBPD_NOTIFY_DATAROLESWAP_DFP:
    case USBPD_NOTIFY_REQUEST_ENTER_MODE:
    case USBPD_NOTIFY_REQUEST_ENTER_MODE_ACK:
    case USBPD_NOTIFY_REQUEST_ENTER_MODE_NAK:
    case USBPD_NOTIFY_REQUEST_ENTER_MODE_BUSY:
    default:
      break;
    }
  } break;
  }
  BSP_LCD_Refresh();
}

/**
 * @brief  demo task function
 * @param  arg
 * @retval None
 */

void DEMO_Task_Standalone(void const *arg) {
  millivoltAPDO = 5000; // 5V output on start
  Display_pd_spec_menu();

  xTimers = xTimerCreate(/* Just a text name, not used by the RTOS
                            kernel. */
                         "Timer",
                         /* The timer period in ticks, must be
                            greater than 0. */
                         pdMS_TO_TICKS(20),
                         /* The timers will auto-reload themselves
                            when they expire. */
                         pdTRUE,
                         /* The ID is used to store a count of the
                            number of times the timer has expired, which
                            is initialised to 0. */
                         (void *)0,
                         /* Each timer calls the same callback when
                            it expires. */
                         vTimerCallback);

  DEMO_Manage_event(DEMO_MSG_MMI | DEMO_MMI_ACTION_NONE);

  for (;;) {
    osEvent event = osMessageGet(DemoEvent, 100);
    DEMO_Manage_event(DEMO_MSG_MMI | DEMO_MMI_ACTION_NONE);
    if (osEventMessage == event.status) {
      DEMO_Manage_event(event.value.v);
    }
  }
}

static void string_completion(uint8_t *str) {
  uint8_t size = strlen((char *)str);

  if (size > STR_SIZE_MAX)
    str[STR_SIZE_MAX] = '\0';
  else {
    /* add space */
    for (uint8_t index = size; index < STR_SIZE_MAX; index++) {
      str[index] = ' ';
    }
    str[STR_SIZE_MAX] = '\0';
  }
}

void vTimerCallback(TimerHandle_t xTimer) {
  const struct joystick_desc_t {
    JOYState_TypeDef input;
    uint16_t event;
  } joy_buttons[JOYn] = {
      {JOY_UP, DEMO_MMI_ACTION_UP_PRESS},
      {JOY_DOWN, DEMO_MMI_ACTION_DOWN_PRESS},
      {JOY_LEFT, DEMO_MMI_ACTION_LEFT_PRESS},
      {JOY_RIGHT, DEMO_MMI_ACTION_RIGHT_PRESS},
      {JOY_SEL, DEMO_MMI_ACTION_SEL_PRESS},
  };
  static JOYState_TypeDef current_on = JOY_NONE;
  const uint32_t long_press_thr = 100;
  static uint32_t hold_time = 0;

  JOYState_TypeDef which = BSP_JOY_GetState();
  if (which == current_on) {
    hold_time++;
    if (which == JOY_SEL && hold_time == long_press_thr) {
      osMessagePut(DemoEvent, DEMO_MSG_MMI | DEMO_MMI_ACTION_SEL_LONGPRESS, 0);
    }
    return;
  }
  if (current_on != JOY_NONE) {       // key released
    if (hold_time < long_press_thr) { // short press
      for (int i = 0; i < JOYn; i++) {
        if (current_on == joy_buttons[i].input) {
          osMessagePut(DemoEvent, DEMO_MSG_MMI | joy_buttons[i].event, 0);
        }
      }
    }
  }
  current_on = which;
  hold_time = 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
