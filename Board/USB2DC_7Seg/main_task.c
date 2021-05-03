#include "main_task.h"
#include "cmsis_os.h"
#include "ucdc.h"
#include "common.h"
#include "main_task_internal.h"
#include "stm32g0xx_ll_ucpd.h"
#include "usbpd_core.h"
#include "usbpd_dpm_conf.h"
#include "usbpd_dpm_core.h"
#include "usbpd_hw.h"
#include "usbpd_pwr_if.h"
#include "usbpd_trace.h"

#include <stdlib.h>

static osMessageQId MainEvtQueue;
static TimerHandle_t BtnTimer;
static TimerHandle_t DispTimer;
static uint8_t pe_disabled = 0; /* 0 by default, 1 if PE is disabled (in case of
                                   no PD device attached) */
static uint16_t applied_centivolt; // 1 cV = 10 mV
static uint16_t setting_centivolt;
static uint8_t setting_pdoindex;
static uint32_t setting_timeout;
static int16_t adjustment_value[] = {
    [DISP_SET_VOLT] = 100,
    [DISP_SET_DECIVOLT] = 10,
    [DISP_SET_CENTIVOLT] = 2, // minimum granularity of PPS is 20mV
};

void vTimerCallback(TimerHandle_t xTimer) {
  const struct joystick_desc_t {
    JOYState_TypeDef input;
    uint16_t event;
  } joy_buttons[JOYn] = {
      {JOY_UP, UCDC_MMI_ACTION_UP_PRESS},
      {JOY_DOWN, UCDC_MMI_ACTION_DOWN_PRESS},
      {JOY_LEFT, UCDC_MMI_ACTION_LEFT_PRESS},
      {JOY_RIGHT, UCDC_MMI_ACTION_RIGHT_PRESS},
      {JOY_SEL, UCDC_MMI_ACTION_SEL_PRESS},
  };
  static JOYState_TypeDef current_on = JOY_NONE;
  const uint32_t long_press_thr = 100;
  static uint32_t hold_time = 0;

  JOYState_TypeDef which = JoyStick_GetState();
  if (which == current_on) {
    hold_time++;
    if (which == JOY_SEL && hold_time == long_press_thr) {
      osMessagePut(MainEvtQueue, UCDC_MSG_MMI | UCDC_MMI_ACTION_SEL_LONGPRESS,
                   0);
    }
    return;
  }
  if (current_on != JOY_NONE) {       // key released
    if (hold_time < long_press_thr) { // short press
      for (int i = 0; i < JOYn; i++) {
        if (current_on == joy_buttons[i].input) {
          osMessagePut(MainEvtQueue, UCDC_MSG_MMI | joy_buttons[i].event, 0);
        }
      }
    }
  }
  current_on = which;
  hold_time = 0;
}

void dTimerCallback(TimerHandle_t xTimer) { Seg7_Refresh(); }

static void UCDC_Display_Measure() {
  uint32_t millivolt = BSP_PWR_VBUSGetVoltage(0);
  Seg7_Update(millivolt / 10, 1 << 1);
}

static inline void UCDC_Action_Reset_Timeout() {
  setting_timeout = HAL_GetTick();
}
static inline bool UCDC_Setting_Timeout() {
  return HAL_GetTick() - setting_timeout > 10000;
}
static void UCDC_Action_Enter_Settings() {
  setting_centivolt = applied_centivolt;
  setting_centivolt =
      UCDC_Search_Next_Voltage(setting_centivolt, 0, &setting_pdoindex);
  if (!setting_centivolt)
    setting_centivolt =
        UCDC_Search_Next_Voltage(setting_centivolt, 1, &setting_pdoindex);
  if (!setting_centivolt) {
    // TODO: error
  } else {
    Seg7_Update(setting_centivolt, 1 << 1);
    UCDC_Action_Reset_Timeout();
  }
}
static void UCDC_Action_Exit_Settings() { Seg7_SetBlink(0); }

static void UCDC_Action_Adjust(int16_t delta) {
  uint16_t new_centivolt = UCDC_Search_Next_Voltage(
      setting_centivolt + delta, delta > 0, &setting_pdoindex);
  if (!new_centivolt) {
    // out of boundary, ignore this action
    return;
  }
  setting_centivolt = new_centivolt;
  Seg7_Update(setting_centivolt, 1 << 1);
}

static void UCDC_Action_Apply() {
  USBPD_StatusTypeDef ret = USBPD_FAIL;

  if (!setting_centivolt ||
      setting_pdoindex >= DPM_Ports[USBPD_PORT_0].DPM_NumberOfRcvSRCPDO) {
    ERR_MSG("Invalid settings\n");
    return;
  }
  if (applied_centivolt == setting_centivolt) {
    DBG_MSG("Unchanged voltage\n");
    return;
  }

  LED_OutEnable(false); // turn off load before applying new settings
  ret = UCDC_Request_Output(setting_pdoindex, setting_centivolt);
  if (ret == USBPD_OK) {
    applied_centivolt = setting_centivolt;
  }
}

/**
 *
 * @brief  main demo function to manage all the appplication event and to update
 * MMI in standalone mode
 * @param  Event
 * @retval None
 */
static void UCDC_Manage_event(uint32_t Event) {
  static uint8_t _tab_connect_status = 0;
  static enum display_state_t cur_display = DISP_MEASURE;

#define CUR_DISP_IS_SET()                                                      \
  (cur_display == DISP_SET_VOLT || cur_display == DISP_SET_DECIVOLT ||         \
   cur_display == DISP_SET_CENTIVOLT)

  switch (Event & UCDC_MSG_TYPE_MSK) {
  case UCDC_MSG_MMI: {
    switch (Event & UCDC_MMI_ACTION_Msk) {
    case UCDC_MMI_ACTION_NONE:
      if (cur_display == DISP_MEASURE) {
        UCDC_Display_Measure();
      } else if (cur_display == DISP_SET_VOLT)
        Seg7_SetBlink(0b0011);
      else if (cur_display == DISP_SET_DECIVOLT)
        Seg7_SetBlink(0b0100);
      else if (cur_display == DISP_SET_CENTIVOLT)
        Seg7_SetBlink(0b1000);
      if (CUR_DISP_IS_SET() && UCDC_Setting_Timeout()) {
        UCDC_Action_Exit_Settings();
        cur_display = DISP_MEASURE;
      }
      break;
    case UCDC_MMI_ACTION_RIGHT_PRESS:
      UCDC_Action_Reset_Timeout();
      if (cur_display == DISP_MEASURE) {
        if (pe_disabled || _tab_connect_status != 2) {
          ERR_MSG("PD not available\n");
        } else {
          UCDC_Action_Enter_Settings();
          cur_display = DISP_SET_VOLT;
        }
      } else if (cur_display == DISP_SET_VOLT)
        cur_display = DISP_SET_DECIVOLT;
      else if (cur_display == DISP_SET_DECIVOLT)
        cur_display = DISP_SET_CENTIVOLT;
      else if (cur_display == DISP_SET_CENTIVOLT) {
        UCDC_Action_Exit_Settings();
        cur_display = DISP_MEASURE;
        UCDC_Action_Apply();
      }
      break;
    case UCDC_MMI_ACTION_LEFT_PRESS:
      UCDC_Action_Reset_Timeout();
      if (cur_display == DISP_SET_VOLT) {
        UCDC_Action_Exit_Settings();
        cur_display = DISP_MEASURE;
      } else if (cur_display == DISP_SET_DECIVOLT)
        cur_display = DISP_SET_VOLT;
      else if (cur_display == DISP_SET_CENTIVOLT)
        cur_display = DISP_SET_DECIVOLT;
      break;
    case UCDC_MMI_ACTION_UP_PRESS:
      UCDC_Action_Reset_Timeout();
      if (CUR_DISP_IS_SET())
        UCDC_Action_Adjust(adjustment_value[cur_display]);
      else
        LED_OutEnable(1);
      break;
    case UCDC_MMI_ACTION_DOWN_PRESS:
      UCDC_Action_Reset_Timeout();
      if (CUR_DISP_IS_SET())
        UCDC_Action_Adjust(-adjustment_value[cur_display]);
      else
        LED_OutEnable(0);
      break;
    case UCDC_MMI_ACTION_SEL_PRESS:
      break;
    case UCDC_MMI_ACTION_SEL_LONGPRESS:
      // LED_ToggleOutEnable();
      break;
    }
  } break;
  case UCDC_MSG_CAD: {
    switch ((Event & UCDC_CAD_STATE_Msk) >> UCDC_CAD_STATE_Pos) {
    case USBPD_CAD_EVENT_ATTEMC:
    case USBPD_CAD_EVENT_ATTACHED:
      _tab_connect_status = 1;
      break;
    case USBPD_CAD_EVENT_DETACHED:
      _tab_connect_status = 0;
      /* reset PE status information for no PD device attached */
      pe_disabled = 0;
      break;
    }
  } break;
  case UCDC_MSG_GETINFO: {
    switch ((Event & UCDC_MSG_DATA_Msk)) {
    case UCDC_MSG_GETINFO_SNKCAPA: {
      if (0 != _tab_connect_status) {
        if (USBPD_OK ==
            USBPD_PE_Request_CtrlMessage(0, USBPD_CONTROLMSG_GET_SNK_CAP,
                                         USBPD_SOPTYPE_SOP)) {
          /* Request has accepted so switch to the next request */
          // UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SVID);
        } else {
          /* Request has not been accept by the stack so retry to send a message
           */
          UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SNKCAPA);
        }
      }
      break;
    }
    case UCDC_MSG_GETINFO_SVID: {
      if (0 != _tab_connect_status) {
        if (USBPD_OK !=
            USBPD_DPM_RequestVDM_DiscoverySVID(0, USBPD_SOPTYPE_SOP)) {
          /* Request has not been accept by the stack so retry to send a message
           */
          UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SVID);
        }
      }
      break;
    }
    }
    break;
  }
  case UCDC_MSG_PENOTIFY: {
    switch ((Event & UCDC_MSG_DATA_Msk) >> UCDC_MSG_DATA_Pos) {
    case USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT:
      DBG_MSG("USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT\n");
      if (_tab_connect_status == 1) {
        _tab_connect_status = 2;
      }
      break;
    case USBPD_NOTIFY_HARDRESET_RX:
      DBG_MSG("USBPD_NOTIFY_HARDRESET_RX\n");
      { /* reset PE state information in case of no PD device attached */
        pe_disabled = 0;
        break;
      }
    case USBPD_NOTIFY_PE_DISABLED:
      DBG_MSG("USBPD_NOTIFY_PE_DISABLED\n");
      {
        /* means that attached device is not PD : PE is disabled */
        pe_disabled = 1;
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
}

void UCDC_Task_Standalone(void const *arg) {

  DispTimer = xTimerCreate(/* Just a text name, not used by the RTOS
                            kernel. */
                           "DispTimer",
                           /* The timer period in ticks, must be
                              greater than 0. */
                           pdMS_TO_TICKS(7),
                           /* The timers will auto-reload themselves
                              when they expire. */
                           pdTRUE,
                           /* The ID is used to store a count of the
                              number of times the timer has expired, which
                              is initialised to 0. */
                           (void *)0,
                           /* Each timer calls the same callback when
                              it expires. */
                           dTimerCallback);

  BtnTimer = xTimerCreate(/* Just a text name, not used by the RTOS
                            kernel. */
                          "BtnTimer",
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

  UCDC_Manage_event(UCDC_MSG_MMI | UCDC_MMI_ACTION_NONE);
  xTimerStart(BtnTimer, 0);
  xTimerStart(DispTimer, 0);

  applied_centivolt = DPM_USER_Settings[USBPD_PORT_0]
                          .DPM_SNKRequestedPower.OperatingVoltageInmVunits /
                      10;
  for (;;) {
    osEvent event = osMessageGet(MainEvtQueue, 100);
    UCDC_Manage_event(UCDC_MSG_MMI | UCDC_MMI_ACTION_NONE);
    if (osEventMessage == event.status) {
      UCDC_Manage_event(event.value.v);
    }
  }
}

void DEMO_InitBSP(void) {
  DBG_MSG("DEMO_InitBSP\n");
  LED_7Seg_Init();
  JoyStick_Init();
}

void DEMO_InitTask(DEMO_MODE mode) {
  osThreadDef(STD, UCDC_Task_Standalone, osPriorityAboveNormal, 0, 1000);

  osMessageQDef(MainEvtQueue, 30, uint32_t);
  MainEvtQueue = osMessageCreate(osMessageQ(MainEvtQueue), NULL);

  osThreadCreate(osThread(STD), &MainEvtQueue);
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
      (UCDC_MSG_CAD | (Cc << UCDC_CAD_CC_NUM_Pos |
                       (PortNum << UCDC_CAD_PORT_NUM_Pos) | State));
  osMessagePut(MainEvtQueue, event, 0);
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
      UCDC_MSG_PENOTIFY | PortNum << UCDC_MSG_PORT_NUM_Pos | EventVal;
  osMessagePut(MainEvtQueue, event, 0);
}

/**
 * @brief  function called by MMI to order a PE request.
 * @param  PortNum   The handle of the port
 * @param  GetInfoVal
 * @retval None
 */
static void UCDC_PostGetInfoMessage(uint8_t PortNum, uint16_t GetInfoVal) {
  uint32_t event =
      UCDC_MSG_GETINFO | PortNum << UCDC_MSG_PORT_NUM_Pos | GetInfoVal;
  osMessagePut(MainEvtQueue, event, 0);
}

/**
 * @brief  Callback function called by PE to inform DPM about PE event.
 * @param  EventVal @ref USBPD_NotifyEventValue_TypeDef
 * @retval None
 */
void UCDC_PostMMIMessage(uint32_t EventVal) {
  uint32_t event = UCDC_MSG_MMI | EventVal;
  osMessagePut(MainEvtQueue, event, 0);
}

// not implemented
void BSP_PWR_CaptiveCablePreConfig(uint8_t PortNum) {}

void BSP_PWR_CaptiveCablePostConfig(uint8_t PortNum) {}