#include "main_task.h"
#include "main_task_internal.h"
#include "cmsis_os.h"
#include "common.h"
#include "stm32g0xx_ll_ucpd.h"
#include "usbpd_core.h"
#include "usbpd_dpm_conf.h"
#include "usbpd_dpm_core.h"
#include "usbpd_hw.h"
#include "usbpd_pwr_if.h"
#include "usbpd_trace.h"

static osMessageQId MainEvtQueue;
static TimerHandle_t BtnTimer;
static TimerHandle_t DispTimer;
static uint8_t pe_disabled = 0; /* 0 by default, 1 if PE is disabled (in case of no PD device attached) */

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

void dTimerCallback(TimerHandle_t xTimer)
{
  Seg7_Refresh();
}

void Display_Measure()
{
  uint32_t volt = BSP_PWR_VBUSGetVoltage(0);
  Seg7_Update(volt/10, 1<<1);
}

/**
  * @brief  main demo function to manage all the appplication event and to update MMI in standalone mode
  * @param  Event
  * @retval None
  */
static void UCDC_Manage_event(uint32_t Event)
{
  static uint8_t   _tab_connect_status = 0;
  static enum display_state_t cur_display = DISP_MEASURE;

  switch(Event & UCDC_MSG_TYPE_MSK)
  {
  case UCDC_MSG_MMI:
    {
    //   if ((Event & UCDC_MMI_ACTION_Msk) != UCDC_MMI_ACTION_NONE)
    //     demo_timeout = HAL_GetTick();
      switch(Event & UCDC_MMI_ACTION_Msk)
      {
      case UCDC_MMI_ACTION_NONE:
        if(_tab_connect_status != 0)
        {
          if(cur_display == DISP_MEASURE)
          {
            Display_Measure();
          }
        //   if (((MENU_MEASURE == _tab_menu_val)))
        //   {
        //     Display_menuupdate_info(_tab_menu_val);
        //   }
        //   if (DPM_Params[0].PE_Power != USBPD_POWER_NO)
        //   {
        //     if (HAL_GetTick() - demo_timeout> 20000)
        //     {
        //       _tab_menu_val = MENU_MEASURE;
        //       demo_timeout = HAL_GetTick();
        //       Display_menuupdate_info(_tab_menu_val);
        //     }
        //   }
        }
        break;
      case UCDC_MMI_ACTION_RIGHT_PRESS:
      case UCDC_MMI_ACTION_LEFT_PRESS:
      case UCDC_MMI_ACTION_UP_PRESS:
      case UCDC_MMI_ACTION_DOWN_PRESS:
        // Display_menunav_info(_tab_menu_val, Event & UCDC_MMI_ACTION_Msk);
        break;
      case UCDC_MMI_ACTION_SEL_PRESS:
        // _tab_menu_val = Menu_manage_next(_tab_menu_val);
        // Display_menuupdate_info(_tab_menu_val);
        break;
      case UCDC_MMI_ACTION_SEL_LONGPRESS:
        {
        //   DEMO_MENU next_menu = Display_menuexec_info(_tab_menu_val);
        //   if (next_menu != MENU_INVALID) { /* If action successfull */
        //     _tab_menu_val = next_menu;
        //     Display_menuupdate_info(_tab_menu_val);
        //   }
          LED_ToggleOutEnable();
        }
        break;
      }
    }
    break;
  case UCDC_MSG_CAD :
    {
       switch((Event & UCDC_CAD_STATE_Msk)>> UCDC_CAD_STATE_Pos)
       {
       case USBPD_CAD_EVENT_ATTEMC :
       case USBPD_CAD_EVENT_ATTACHED :
         _tab_connect_status = 1;
        //  _tab_menu_val = MENU_PD_SPEC;
        //  demo_timeout = HAL_GetTick();
        //  Display_menuupdate_info(_tab_menu_val);
         break;
       case USBPD_CAD_EVENT_DETACHED :
         _tab_connect_status = 0;
         pe_disabled = 0; /* reset PE status information for no PD device attached */
        //  _tab_menu_val = MENU_PD_SPEC;
        //  Display_menuupdate_info(_tab_menu_val);

        //  BSP_LED_Off(LED_ORANGE);
         break;
       }
    }
  break;
  case UCDC_MSG_GETINFO  :
    {
      switch((Event & UCDC_MSG_DATA_Msk))
      {
      case UCDC_MSG_GETINFO_SNKCAPA :
        {
          if(0 != _tab_connect_status)
          {
            if( USBPD_OK == USBPD_PE_Request_CtrlMessage(0, USBPD_CONTROLMSG_GET_SNK_CAP, USBPD_SOPTYPE_SOP))
            {
              /* Request has accepted so switch to the next request */
              // UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SVID);
            }
            else
            {
              /* Request has not been accept by the stack so retry to send a message */
              UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SNKCAPA);
            }
          }
          break;
        }
      case UCDC_MSG_GETINFO_SVID :
        {
          if(0 != _tab_connect_status)
          {
            if( USBPD_OK != USBPD_DPM_RequestVDM_DiscoverySVID(0, USBPD_SOPTYPE_SOP))
            {
              /* Request has not been accept by the stack so retry to send a message */
              UCDC_PostGetInfoMessage(0, UCDC_MSG_GETINFO_SVID);
            }
          }
          break;
        }

      }
      break;
    }
  case UCDC_MSG_PENOTIFY :
    {
      switch((Event & UCDC_MSG_DATA_Msk)>> UCDC_MSG_DATA_Pos)
      {
      case USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT :
        DBG_MSG("USBPD_NOTIFY_POWER_EXPLICIT_CONTRACT\n");
        if (_tab_connect_status == 1)
        {
        //   _tab_menu_val = MENU_MEASURE;
        //   Display_menuupdate_info(_tab_menu_val);
          _tab_connect_status = 2;
        }
        break;
      case USBPD_NOTIFY_HARDRESET_RX :
        DBG_MSG("USBPD_NOTIFY_HARDRESET_RX\n");
        {
         pe_disabled = 0; /* reset PE state information in case of no PD device attached */
        //  _tab_menu_val = MENU_PD_SPEC;
        //  Display_menuupdate_info(_tab_menu_val);
         break;
        }
      case USBPD_NOTIFY_PE_DISABLED :
        DBG_MSG("USBPD_NOTIFY_PE_DISABLED\n");
        {
         pe_disabled = 1; /* means that attached device is not PD : PE is disabled */
        //  _tab_menu_val = MENU_MEASURE;
        //  Display_menuupdate_info(_tab_menu_val);
         break;
        }        
      case USBPD_NOTIFY_VDM_SVID_RECEIVED :
      case USBPD_NOTIFY_HARDRESET_TX :
      case USBPD_NOTIFY_SNK_GOTOMIN :
      case USBPD_NOTIFY_SNK_GOTOMIN_READY :
      case USBPD_NOTIFY_SOFTRESET_RECEIVED :
      case USBPD_NOTIFY_PING_RECEIVED :
      case USBPD_NOTIFY_GETSRCCAP_ACCEPTED :
      case USBPD_NOTIFY_GETSRCCAP_REJECTED :
      case USBPD_NOTIFY_GETSNKCAP_TIMEOUT :
      case USBPD_NOTIFY_REQUEST_ACCEPTED :
      case USBPD_NOTIFY_REQUEST_REJECTED :
      case USBPD_NOTIFY_REQUEST_WAIT :
      case USBPD_NOTIFY_REQUEST_GOTOMIN :
      case USBPD_NOTIFY_POWER_SWAP_TO_SNK_DONE :
      case USBPD_NOTIFY_POWER_SWAP_TO_SRC_DONE :
      case USBPD_NOTIFY_POWER_SWAP_REJ :
      case USBPD_NOTIFY_DATAROLESWAP_UFP:
      case USBPD_NOTIFY_DATAROLESWAP_DFP:
      case USBPD_NOTIFY_REQUEST_ENTER_MODE :
      case USBPD_NOTIFY_REQUEST_ENTER_MODE_ACK :
      case USBPD_NOTIFY_REQUEST_ENTER_MODE_NAK :
      case USBPD_NOTIFY_REQUEST_ENTER_MODE_BUSY :
      default :
        break;
      }
    }
  break;
  }
//   BSP_LCD_Refresh();
}


void UCDC_Task_Standalone(void const *arg) {
  //   Display_pd_spec_menu();

  DispTimer = xTimerCreate(/* Just a text name, not used by the RTOS
                            kernel. */
                          "DispTimer",
                          /* The timer period in ticks, must be
                             greater than 0. */
                          pdMS_TO_TICKS(13),
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
  xTimerStart( BtnTimer, 0 );
  xTimerStart( DispTimer, 0 );

  for (;;) {
    osEvent event = osMessageGet(MainEvtQueue, 100);
    UCDC_Manage_event(UCDC_MSG_MMI | UCDC_MMI_ACTION_NONE);
    if (osEventMessage == event.status) {
      UCDC_Manage_event(event.value.v);
    }
  }
}

void DEMO_InitBSP(void) {
  //   BSP_LED_Init(LED_ORANGE);
  //   BSP_JOY_Init(JOY_MODE_GPIO);
  DBG_MSG("DEMO_InitBSP\n");
  LED_7Seg_Init();
  JoyStick_Init();

  /*##-1- Initialize the LCD #################################################*/
  /* Initialize the LCD */
  //   BSP_LCD_Init();
  //   BSP_LCD_SetFont(&Font12);
}

void DEMO_InitTask(DEMO_MODE mode) {
  osThreadDef(STD, UCDC_Task_Standalone, osPriorityAboveNormal, 0, 1000);

  osMessageQDef(MainEvtQueue, 30, uint32_t);
  MainEvtQueue = osMessageCreate(osMessageQ(MainEvtQueue), NULL);

  osThreadCreate(osThread(STD), &MainEvtQueue);
}

/**
  * @brief  UserCableDetection reporting events on a specified port from CAD layer.
  * @param  PortNum   The handle of the port
  * @param  State     CAD state
  * @param  Cc        The Communication Channel for the USBPD communication
  * @retval None
  */
void DEMO_PostCADMessage(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc)
{
  uint32_t event = (UCDC_MSG_CAD |  (Cc << UCDC_CAD_CC_NUM_Pos | (PortNum << UCDC_CAD_PORT_NUM_Pos) | State ));
  osMessagePut(MainEvtQueue, event, 0);
}

/**
  * @brief  Callback function called by PE to inform DPM about PE event.
  * @param  PortNum   The handle of the port
  * @param  EventVal @ref USBPD_NotifyEventValue_TypeDef
  * @retval None
  */
void DEMO_PostNotificationMessage(uint8_t PortNum, USBPD_NotifyEventValue_TypeDef EventVal)
{
  uint32_t event = UCDC_MSG_PENOTIFY | PortNum << UCDC_MSG_PORT_NUM_Pos | EventVal;
  osMessagePut(MainEvtQueue, event, 0);
}

/**
  * @brief  function called by MMI to order a PE request.
  * @param  PortNum   The handle of the port
  * @param  GetInfoVal
  * @retval None
  */
static void UCDC_PostGetInfoMessage(uint8_t PortNum, uint16_t GetInfoVal)
{
  uint32_t event = UCDC_MSG_GETINFO | PortNum << UCDC_MSG_PORT_NUM_Pos | GetInfoVal;
  osMessagePut(MainEvtQueue, event, 0);
}

/**
  * @brief  Callback function called by PE to inform DPM about PE event.
  * @param  EventVal @ref USBPD_NotifyEventValue_TypeDef
  * @retval None
  */
void UCDC_PostMMIMessage(uint32_t EventVal)
{
  uint32_t event = UCDC_MSG_MMI | EventVal;
  osMessagePut(MainEvtQueue, event, 0);
}

// not implemented
void BSP_PWR_CaptiveCablePreConfig(uint8_t PortNum)
{
}

void BSP_PWR_CaptiveCablePostConfig(uint8_t PortNum)
{
}