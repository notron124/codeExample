#include "stm32f10x.h"
#include "modes.h"
#include "keyapi.h"
#include "lcd1602.h"
#include "esp8266.h"
#include "packet.h"
#include "communication.h"
#include "keyfunctions.h"
#include "interface.h"

/* Note: Take into count that all mode handlers will be called 100 times per second (in 100Hz handler)
   so one second equals to 100 ticks here
*/

/* Used, when you go from any type of mode to main mode.
   - pMode is a pointer to variable, that holds current main mode.
   - pModeType is a pointer to variable, that holds current mode type.
*/
void ChangeToMainModeType(MainModes *pMode, ModeTypes *pModeType, MainModes mode)
{
   *pModeType = MAIN_MODE;
   *pMode = mode;
}

/* Used, when you go from any type of mode to support mode.
   - pMode is a pointer to variable, that holds current support mode.
   - pModeType is a pointer to variable, that holds current mode type.
*/
void ChangeToSupportModeType(SupportModes *pMode, ModeTypes *pModeType, SupportModes mode)
{
   *pModeType = SUPPORT_MODE;
   *pMode = mode;
}

/* Used, when you go from any type of mode to service mode.
   - pMode is a pointer to variable, that holds current service mode.
   - pModeType is a pointer to variable, that holds current mode type.
*/
void ChangeToServiceModeType(ServiceModes *pMode, ModeTypes *pModeType, ServiceModes mode)
{
   *pModeType = SERVICE_MODE;
   *pMode = mode;
}

void ShowMode(void (*const func)(void))
{
   if (REFRESH_LCD)
   {
      RESET_REFRESH_LCD;

      if (CLEAR_LCD)
      {
      	RESET_CLEAR_LCD;
         PutClear_LCD();
      }

      (*func)();
   }
}

#define TITLE_TIME 400

void Title(MainModes *mode)
{
   static uint16_t showTitleTimer;

   if (keys[DOWN_KEY].flags.bits.pressed && keys[UP_KEY].flags.bits.pressed)
   {
      ChangeToSupportModeType(&supportMode, &modeType, SUPP_SERVICE);
      PutClear_LCD();
   }
   else if (++showTitleTimer >= TITLE_TIME)
      (*mode) = M_ENTRY;
}

void Entry(MainModes *mode)
{
   if (communication.flags.bits.firmware_downloaded)
   {
      PutClear_LCD();
      (*mode) = M_REQUEST_UPDATE_FIRMWARE_PULT_MODE;
   }
}

void HandleMainModes(MainModes *mode, int8_t *keyCode)
{
   if ((*keyCode) >= 0)
   {
   	SET_REFRESH_LCD;
   	RESET_BLINK_PARAM;
      MainModeKeyHandler[(*mode)][(*keyCode)]();
      (*keyCode) = -1;
   }

   switch ((*mode))
   {
   case M_TITLE:
      Title(mode);
      break;

   case M_ENTRY:
      Entry(mode);
      break;

   case M_PROGRAMS:
      break;

   case M_HEATING:
      // Heating(mode);
      break;

   case M_HOLDING:
      // Holding(mode);
      break;

   case M_PRESSING:

      break;

   case M_COMPLETED:
      break;

   case M_ALERT:
      // Alert(mode);
      break;

   case M_REQUEST_UPDATE_FIRMWARE_PECH_MODE:
      break;

   case M_REQUEST_UPDATE_FIRMWARE_PULT_MODE:
      break;

   case M_WRITE_FIRMWARE_PECH:
      // WriteFrimwarePech(mode);
      break;

   case M_EXT_MANAGMENT:
      // ExtManagment(mode);
      break;

   case M_SYSTEM_RESET:
      // SystemReset(mode);
      break;

   default:
      /* mode under number 0 is should be some kind of an device title/intro,
         this will jump to it if unknown mode is present */
      *mode = 0;
      break;
   }

   if (REFRESH_LCD)
   {
      RESET_REFRESH_LCD;

      if (CLEAR_LCD)
      {
      	RESET_CLEAR_LCD;
         PutClear_LCD();
      }

      ShowMainModes[(*mode)]();
   }

//   ShowMode(ShowMainModes[(*mode)]); // more memory efficient, but slower
}

void HandleServiceModes(ServiceModes *serviceMode, int8_t *keyCode)
{
   if ((*keyCode) >= 0)
   {
      SET_REFRESH_LCD;
      RESET_BLINK_PARAM;
      ServiceModeKeyHandler[(*serviceMode)][(*keyCode)]();
      (*keyCode) = -1;
   }

   switch ((*serviceMode))
   {
   case SERV_USERS_SERVICE:

      break;

   case SERV_LANGUAGE:
      break;

   case SERV_FLOW:
      break;

   case SERV_ROUTER:
      break;

   case SERV_MAINTAIN_TEMPERATURE:
      break;

   case SERV_IP:
      break;

   case SERV_REGISTRATION:
      break;

   case SERV_NETWORK_NUMBER:
      break;

   case SERV_VERSIONS:
      break;

   default:
      (*serviceMode) = 0;
      break;
   }

   if (REFRESH_LCD)
   {
      RESET_REFRESH_LCD;

      if (CLEAR_LCD)
      {
         RESET_CLEAR_LCD;
         PutClear_LCD();
      }

      ShowServiceModes[(*serviceMode)]();
   }
}

void HandleSupportModes(SupportModes *supportMode, int8_t *keyCode)
{
   if ((*keyCode) >= 0)
   {
      SET_REFRESH_LCD;
      RESET_BLINK_PARAM;
      SupportModeKeyHandler[(*supportMode)][(*keyCode)]();
      (*keyCode) = -1;
   }

   switch ((*supportMode))
   {
   case SUPP_SERVICE:
      break;

   case SUPP_LANGUAGE:
      break;

   case SUPP_FLOW:
      break;

   case SUPP_ROUTER:
      break;

   case SUPP_MAINTAIN_TEMPERATURE:
      break;

   case SUPP_BY_TROOM_SETUP:
      break;

   case SUPP_BY_THOT_SETUP:
      break;

   case SUPP_PRESS_SENS_SETUP:
      break;

   case SUPP_CYLINDER_TEST:
      break;

   case SUPP_IP:
      break;

   case SUPP_REGISTRATION:
      break;

   case SUPP_NETWORK_NUMBER:
      break;

   case SUPP_VERSIONS:
      break;

   default:
      (*supportMode) = 0;
      break;
   }

   if (REFRESH_LCD)
   {
      RESET_REFRESH_LCD;

      if (CLEAR_LCD)
      {
         RESET_CLEAR_LCD;
         PutClear_LCD();
      }

      ShowSupportModes[(*supportMode)]();
   }
}
