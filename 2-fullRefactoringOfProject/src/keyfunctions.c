#include "stm32f10x.h"
#include "modes.h"
#include "keyapi.h"
#include "keyfunctions.h"
#include "program.h"
#include "lcd1602.h"

/* All logic related to key presses are written here.
   There will be 3 arrays of functions, all dedicated to it own mode (Main, service, support).
*/

void Dummy(void)
{

}

/* Decreases given value and returns the result.
 *
 * keyHoldTime - value that represents how long key was holded for.
 * used to increase the decrement.
 *
 * divider - divides keyHoldTime to reduce speed of increase of decrement.
 * Bigger = slower increase.
 *
 *  */
uint32_t DecreaseValue(uint32_t value, uint32_t min, uint32_t max, \
							  uint32_t keyHoldTime, uint32_t divider)
{
	if (divider == 0)
		divider = 1;

	if (keyHoldTime >= 1000)
	{
		if (value > min)
			value -= 1 + (keyHoldTime - 1000) / divider;

		if (value > max || value < min)
			value = min;
	}
	else
	{
		if (value > min)
			value -= 1;
		else
			value = max;
	}

   if (value > max || value < min)
      value = max;

   return value;
}


/* Increases given value and returns the result.
 *
 * keyHoldTime - value that represents how long key was holded for.
 * used to increase the increment.
 *
 * divider - divides keyHoldTime to reduce speed of increase of increment.
 * Bigger = slower increase.
 *
 *  */
uint32_t IncreaseValue(uint32_t value, uint32_t min, uint32_t max, \
							  uint32_t keyHoldTime, uint32_t divider)
{
	if (divider == 0)
		divider = 1;

	if (keyHoldTime >= 1000)
	{
		if (value < max)
			value += 1 + (keyHoldTime - 1000) / divider;

		if (value > max || value < min)
			value = max;
	}
	else
	{
		if (value < max)
			value += 1;
		else
			value = min;
	}

   return value;
}

void StockDown(void)
{

}

void StockUp(void)
{

}

void EnterPrograms(void)
{
	SET_CLEAR_LCD;
   mode = M_PROGRAMS;
   keys[DOWN_KEY].flags.bits.autorepeat = 1;
   keys[UP_KEY].flags.bits.autorepeat = 1;
}

void EnterService(void)
{
	// SET_CLEAR_LCD;
   keys[DOWN_KEY].flags.bits.autorepeat = 1;
   keys[UP_KEY].flags.bits.autorepeat = 1;
   ChangeToServiceModeType(&serviceMode, &modeType, SERV_USERS_SERVICE);
}

void SwitchParameter(void)
{
   paramNumber += paramNumber < MAX_PARAMETER ? 1 : -6;
}

void DecreaseParameter(void)
{
   switch (paramNumber)
   {
   case P_NUMBER:
      programNumber = (uint8_t)DecreaseValue(programNumber,
															minParameters[paramNumber],
															maxParameters[paramNumber],
															keys[DOWN_KEY].counters.holdTime,
															16384);
      break;

   case P_TYPE:
      programs[programNumber].flags.bits.type =
      				(DecreaseValue(programs[programNumber].flags.bits.type,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[DOWN_KEY].counters.holdTime,
											 16384) & 0x03);
      break;

   case P_TARGET_TEMPERATURE:
      programs[programNumber].parameters.targetTemperature =
          (uint16_t)DecreaseValue(programs[programNumber].parameters.targetTemperature,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[DOWN_KEY].counters.holdTime,
											 256);
      break;

   case P_HOLDING_TIME:
      programs[programNumber].parameters.holdingTime =
          (uint16_t)DecreaseValue(programs[programNumber].parameters.holdingTime,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[DOWN_KEY].counters.holdTime,
											 512);
      break;

   case P_FLOW:
      programs[programNumber].flags.bits.flowTmp ^= 1;
      break;

   case P_TARGET_PRESSURE:
      programs[programNumber].parameters.targetPressure =
          (uint16_t)DecreaseValue(programs[programNumber].parameters.targetPressure,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[DOWN_KEY].counters.holdTime,
											 512);
      break;

   case P_PRESSING_TIME:
      programs[programNumber].parameters.pressingTime =
          (uint16_t)DecreaseValue(programs[programNumber].parameters.pressingTime,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[DOWN_KEY].counters.holdTime,
											 512);
      break;

   default:
      paramNumber = 0;
      break;
   }
}

void IncreaseParameter(void)
{
   switch (paramNumber)
   {
   case P_NUMBER:
      programNumber = (uint8_t)IncreaseValue(programNumber,
															minParameters[paramNumber],
															maxParameters[paramNumber],
															keys[UP_KEY].counters.holdTime,
															16384);
      break;

   case P_TYPE:
      programs[programNumber].flags.bits.type =
      				(IncreaseValue(programs[programNumber].flags.bits.type,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[UP_KEY].counters.holdTime,
											 16384) & 0x03);
      break;

   case P_TARGET_TEMPERATURE:
      programs[programNumber].parameters.targetTemperature =
          (uint16_t)IncreaseValue(programs[programNumber].parameters.targetTemperature,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[UP_KEY].counters.holdTime,
											 256);
      break;

   case P_HOLDING_TIME:
      programs[programNumber].parameters.holdingTime =
          (uint16_t)IncreaseValue(programs[programNumber].parameters.holdingTime,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[UP_KEY].counters.holdTime,
											 512);
      break;

   case P_FLOW:
      programs[programNumber].flags.bits.flowTmp ^= 1;
      break;

   case P_TARGET_PRESSURE:
      programs[programNumber].parameters.targetPressure =
          (uint16_t)IncreaseValue(programs[programNumber].parameters.targetPressure,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[UP_KEY].counters.holdTime,
											 512);
      break;

   case P_PRESSING_TIME:
      programs[programNumber].parameters.pressingTime =
          (uint16_t)IncreaseValue(programs[programNumber].parameters.pressingTime,
                                  minParameters[paramNumber],
                                  maxParameters[paramNumber],
											 keys[UP_KEY].counters.holdTime,
											 512);
      break;

   default:
      paramNumber = 0;
      break;
   }
}

void StartProgram(void)
{
   keys[DOWN_KEY].flags.bits.autorepeat = 0;
   keys[UP_KEY].flags.bits.autorepeat = 0;
}

void ReturnToEntry(void)
{
	SET_CLEAR_LCD;
   keys[DOWN_KEY].flags.bits.autorepeat = 0;
   keys[UP_KEY].flags.bits.autorepeat = 0;
   ChangeToMainModeType(&mode, &modeType, M_ENTRY);
}

void (*const MainModeKeyHandler[][8])(void) =
{/*  MENU_KEY        DOWN_KEY    UP_KEY      OK_KEY         L_MENU_KEY     L_DOWN_KEY  L_UP_KEY    L_OK_KEY */
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_TITLE 
   { Dummy,          StockDown,  StockUp,    EnterPrograms, EnterService,  Dummy,      Dummy,      Dummy }, // M_ENTRY
   { SwitchParameter, DecreaseParameter, IncreaseParameter, StartProgram, ReturnToEntry,Dummy,     Dummy,      Dummy }, // M_PROGRAMS
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_HEATING 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_HOLDING 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_PRESSING 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_COMPLETED 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_ALERT 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_REQUEST_UPDATE_FIRMWARE_PECH_MODE 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_REQUEST_UPDATE_FIRMWARE_PULT_MODE 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_WRITE_FIRMWARE_PECH 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_EXT_MANAGMENT 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // M_SYSTEM_RESET 

};

void ToServSelect(void)
{
   SET_CLEAR_LCD;
   serviceMode = SERV_USERS_SERVICE;
}

void PreviousService(void)
{
   serviceSelectCntr = DecreaseValue(serviceSelectCntr, 0, 8, 0, 1);
}

void NextService(void)
{
   serviceSelectCntr = IncreaseValue(serviceSelectCntr, 0, 8, 0, 1);
}

void ConfirmServChoice(void)
{

}

void (*const ServiceModeKeyHandler[][8])(void) =
{/*  MENU_KEY        DOWN_KEY    UP_KEY      OK_KEY         L_MENU_KEY     L_DOWN_KEY  L_UP_KEY    L_OK_KEY */
   { Dummy,   PreviousService,  NextService, ConfirmServChoice, ReturnToEntry, Dummy,      Dummy,      Dummy }, // SERV_USERS_SERVICE
   { Dummy,          Dummy,      Dummy,      Dummy,         ToServSelect,  Dummy,      Dummy,      Dummy }, // SERV_LANGUAGE
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_FLOW
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_ROUTER
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_MAINTAIN_TEMPERATURE
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_IP
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_REGISTRATION 
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_NETWORK_NUMBER
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SERV_VERSIONS
};

void ToSuppSelect(void)
{
   SET_CLEAR_LCD;
   supportMode = SUPP_SERVICE;
}

void PreviousSupport(void)
{
   supportSelectCntr = DecreaseValue(serviceSelectCntr, 0, 12, 0, 1);
}

void NextSupport(void)
{
   supportSelectCntr = IncreaseValue(serviceSelectCntr, 0, 12, 0, 1);
}

void ConfirmSuppChoice(void)
{

}

void (*const SupportModeKeyHandler[][8])(void) =
{/*  MENU_KEY        DOWN_KEY    UP_KEY      OK_KEY         L_MENU_KEY     L_DOWN_KEY  L_UP_KEY    L_OK_KEY */
   { Dummy,  PreviousSupport, NextSupport, ConfirmSuppChoice,Dummy,        Dummy,      Dummy,      Dummy }, // SUPP_SERVICE
   { Dummy,          Dummy,      Dummy,      Dummy,         ToSuppSelect,  Dummy,      Dummy,      Dummy }, // SUPP_LANGUAGE
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_FLOW
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_ROUTER
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_MAINTAIN_TEMPERATURE
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_BY_TROOM_SETUP
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_BY_THOT_SETUP
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_PRESS_SENS_SETUP
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_CYLINDER_TEST
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_IP
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_REGISTRATION
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_NETWORK_NUMBER
   { Dummy,          Dummy,      Dummy,      Dummy,         Dummy,         Dummy,      Dummy,      Dummy }, // SUPP_VERSIONS
};
