#pragma once

#include <stdint.h>

enum display_state_t{
     DISP_MEASURE,
};

/* Format of the different kind of message */
/*   - 31-28  4 bit for the message type   */
#define UCDC_MSG_TYPE_POS                 28u
#define UCDC_MSG_TYPE_MSK                 (0xFu << UCDC_MSG_TYPE_POS)
#define UCDC_MSG_MMI                      (0u   << UCDC_MSG_TYPE_POS)
#define UCDC_MSG_CAD                      (1u   << UCDC_MSG_TYPE_POS)
#define UCDC_MSG_PENOTIFY                 (2u   << UCDC_MSG_TYPE_POS)
#define UCDC_MSG_GETINFO                  (3u   << UCDC_MSG_TYPE_POS)

/*  MMI
     - 15-0 id event                       */
#define UCDC_MMI_ACTION_Pos                   (0U)
#define UCDC_MMI_ACTION_Msk                   (0xFF << UCDC_MMI_ACTION_Pos) /*!< 0x00FF */
#define UCDC_MMI_ACTION                       DPM_USER_ACTION_Msk
#define UCDC_MMI_ACTION_NONE                  (0u)
#define UCDC_MMI_ACTION_RIGHT_PRESS           (2u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_LEFT_PRESS            (3u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_UP_PRESS              (4u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_DOWN_PRESS            (5u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_SEL_PRESS             (6u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_HPD_DETECT_HIGH_PORT1 (7u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_HPD_DETECT_LOW_PORT1  (8u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_DISPLAY_VBUS_IBUS     (9u << UCDC_MMI_ACTION_Pos)
#define UCDC_MMI_ACTION_SEL_LONGPRESS         (10u << UCDC_MMI_ACTION_Pos)

/*  CAD
     - 27-26 2 bit for CC Pin
     - 25-24 2 bit port num
     - 15-00 16 bit CAD event
*/
#define UCDC_CAD_CC_NUM_Pos               (26u)
#define UCDC_CAD_CC_NUM_Msk               (0x3u << UCDC_CAD_CC_NUM_Pos)
#define UCDC_CAD_CC_NUM_NONE              (0u   << UCDC_CAD_CC_NUM_Pos)
#define UCDC_CAD_CC_NUM_CC1               (1u   << UCDC_CAD_CC_NUM_Pos)
#define UCDC_CAD_CC_NUM_CC2               (2u   << UCDC_CAD_CC_NUM_Pos)

#define UCDC_CAD_PORT_NUM_Pos             (24u)
#define UCDC_CAD_PORT_NUM_Msk             (0x3u << UCDC_CAD_PORT_NUM_Pos)
#define UCDC_CAD_PORT_NUM_0               (0x0u << UCDC_CAD_PORT_NUM_Pos)
#define UCDC_CAD_PORT_NUM_1               (0x1u << UCDC_CAD_PORT_NUM_Pos)

#define UCDC_CAD_STATE_Pos                (0u)
#define UCDC_CAD_STATE_Msk                (0xFFu << UCDC_CAD_STATE_Pos)

/*  PE
     - 27-26 2 bit none
     - 25-24 2 bit port num
     - 15-00 16 bit PE event
*/
#define UCDC_MSG_PORT_NUM_Pos             (24u)
#define UCDC_MSG_PORT_NUM_Msk             (0x3u << UCDC_MSG_PORT_NUM_Pos)
#define UCDC_MSG_PORT_NUM_0               (0x0u << UCDC_MSG_PORT_NUM_Pos)
#define UCDC_MSG_PORT_NUM_1               (0x1u << UCDC_MSG_PORT_NUM_Pos)

#define UCDC_MSG_DATA_Pos                (0u)
#define UCDC_MSG_DATA_Msk                (0xFFu << UCDC_MSG_DATA_Pos)

/* GET INFO Message */
#define UCDC_MSG_GETINFO_SNKCAPA         (0x1u << UCDC_MSG_DATA_Pos)
#define UCDC_MSG_GETINFO_SVID            (0x2u << UCDC_MSG_DATA_Pos)


static void UCDC_PostGetInfoMessage(uint8_t PortNum, uint16_t GetInfoVal);
