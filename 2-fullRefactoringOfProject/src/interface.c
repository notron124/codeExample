#include "stm32f10x.h"
#include "lcd1602.h"
#include "interface.h"
#include "sensors.h"
#include "program.h"
#include "modes.h"

const uint8_t pressureSym[] =    "P=";
const uint8_t temperatureSym[] = "T=";
const uint8_t timeSym[] =        "t=";


const uint8_t ruLogo[] = "АВЕРОН";
const uint8_t enLogo[] = "AVERON";

const uint8_t *const MsgLogo[][2] =
{
   {ruLogo, enLogo},
};

const uint8_t ruTitle0[] = "ТЕРМОПРЕСС";
const uint8_t enTitle0[] = "TERMOPRESS";

const uint8_t *const MsgTitle[][2] =
{
   {ruTitle0, enTitle0},
};

const uint8_t TrpVer[] = "3";

const uint8_t TrpType0[] =    " ";
const uint8_t TrpType1[] =    "M";
const uint8_t ruTrpType2[] =  "НЬЮ";
const uint8_t enTrpType2[] =  "NEW";

const uint8_t *const MsgTrpType[][2] =
{
   {TrpType0, TrpType0},
   {TrpType1, TrpType1},
   {ruTrpType2, enTrpType2},
};

/* TERMOPRESS 3 NEW
       AVERON       */
void ShowTitle(void)
{
   /* TERMOPRESS X typ*/
   PutStr_LCD(0, 0, MsgTitle[0][language], 0);
   PutChr_LCD(_data, ' ');
   PutStr_LCD(255, 0, TrpVer, 0);
   PutChr_LCD(_data, ' ');
   PutStr_LCD(255, 0, MsgTrpType[deviceID][language], 0);

   /* AVERON */
   PutStr_LCD(4, 1, MsgLogo[0][language], 0);


}

const uint8_t ruEntry0[] = "Положение поршня";
const uint8_t enEntry0[] = "Piston position";

const uint8_t *const MsgEntry[][2] =
{
   {ruEntry0, enEntry0},
};

/* Will be changed, when stocks logic implemented */
void ShowEntry(void)
{
   PutStr_LCD(0, 0, MsgEntry[0][language], 0);
   PutStr_LCD(0, 1, pressureSym, 0);

   if (pressureSens.convertedValue >= 20)
   {
      PutDgt_LCD(255, 1, 1, pressureSens.convertedValue / 10, 0, 0);
      PutChr_LCD(_data, '.');
      PutDgt_LCD(255, 1, 1, pressureSens.convertedValue % 10, 0, 0);
   }
   else
   {
      PutStr_LCD(255, 1, (uint8_t*)"---", 0);
   }

   PutChr_LCD(_data, ' ');
   PutChr_LCD(_data, 0x04);
   PutChr_LCD(_data, ' ');
   PutChr_LCD(_data, 0x05);
   PutChr_LCD(_data, ' ');

   PutStr_LCD(255, 1, temperatureSym, 0);
   PutDgt_LCD(255, 1, 3, tCouple.convertedValue, 0, ' ');
}

const uint8_t progType0[] =   "А";
const uint8_t ruProgType1[] = "П";
const uint8_t enProgType1[] = "S";
const uint8_t ruProgType2[] = "Р";
const uint8_t enProgType2[] = "M";

const uint8_t* const MsgProgType[][2] =
{
   {progType0, progType0},
   {ruProgType1, enProgType1},
   {ruProgType2, enProgType2},
};

const uint8_t ruTimeHold[] =  "tв=";
const uint8_t enTimeHold[] =  "th=";
const uint8_t ruTimePress[] = "tп=";
const uint8_t enTimePress[] = "tp=";

const uint8_t *const MsgTime[][2] =
{
   {ruTimeHold, enTimeHold},
   {ruTimePress, enTimePress},
};


void ShowPrograms(void)
{
   PutDgt_LCD(0, 0, 2, (programNumber + 1), (paramNumber == P_NUMBER) * BLINK, 0);

   PutStr_LCD(2, 0, MsgProgType[programs[programNumber].flags.bits.type][language], (paramNumber == P_TYPE) * BLINK);

   PutStr_LCD(5, 0, temperatureSym, 0);
   PutDgt_LCD(7, 0, 3, programs[programNumber].parameters.targetTemperature, (paramNumber == P_TARGET_TEMPERATURE) * BLINK, ' ');

   PutStr_LCD(11, 0, MsgTime[0][language], 0);
   PutDgt_LCD(14, 0, 2, programs[programNumber].parameters.holdingTime, (paramNumber == P_HOLDING_TIME) * BLINK, ' ');

   PutChr_LCD(_comm, 0xC0);
   PutChr_LCD(_data, '(');
   if (programs[programNumber].flags.bits.flowTmp)
      PutStr_LCD(1, 1, (uint8_t*)"*", (paramNumber == P_FLOW) * BLINK);
   else
      PutStr_LCD(1, 1, (uint8_t *)"-", (paramNumber == P_FLOW) * BLINK);
   PutChr_LCD(_data, ')');

   PutStr_LCD(5, 1, pressureSym, 0);
   PutDgt_LCD(7, 1, 1, programs[programNumber].parameters.targetPressure / 10, (paramNumber == P_TARGET_PRESSURE) * BLINK, '0');
   PutChr_LCD(_data, '.');
   PutDgt_LCD(9, 1, 1, programs[programNumber].parameters.targetPressure % 10, (paramNumber == P_TARGET_PRESSURE) * BLINK, '0');

   PutStr_LCD(11, 1, MsgTime[1][language], 0);
   PutDgt_LCD(14, 1, 2, programs[programNumber].parameters.pressingTime, (paramNumber == P_PRESSING_TIME) * BLINK, ' ');
}

void (*const ShowMainModes[])(void) =
{
   ShowTitle,
   ShowEntry,
   ShowPrograms,
};

const uint8_t ruUserService[] = "Сервисные       ";
const uint8_t enUserService[] = "Services        ";

const uint8_t ruUserService0[] = "Language        ";
const uint8_t enUserService0[] = "Язык            ";
const uint8_t ruUserService1[] = "Вентилятор      ";
const uint8_t enUserService1[] = "Fan             ";
const uint8_t ruUserService2[] = "Роутер          ";
const uint8_t enUserService2[] = "Router          ";
const uint8_t ruUserService3[] = "Поддержание T   ";
const uint8_t enUserService3[] = "T maintaining   ";
const uint8_t UserService4[]   = "IP              ";
const uint8_t ruUserService5[] = "Регистрация     ";
const uint8_t enUserService5[] = "Registration    ";
const uint8_t ruUserService6[] = "Сетевой номер   ";
const uint8_t enUserService6[] = "Network number  ";
const uint8_t ruUserService7[] = "Версии          ";
const uint8_t enUserService7[] = "Versions        ";

const uint8_t *const MsgUserService[][2] = 
{
   {ruUserService0, enUserService0},
   {ruUserService1, enUserService1},
   {ruUserService2, enUserService2},
   {ruUserService3, enUserService3},
   {  UserService4,   UserService4},
   {ruUserService5, enUserService5},
   {ruUserService6, enUserService6},
   {ruUserService7, enUserService7},
   { ruUserService,  enUserService}, // This one should be the last one ALWAYS
};

void ShowUsersService(void)
{
   PutStr_LCD(0, 0, MsgUserService[sizeof(MsgUserService[1])][language], 0);
   PutStr_LCD(0, 1, MsgUserService[serviceSelectCntr][language], 0);
}

void (*const ShowServiceModes[])(void) =
{
   ShowUsersService,
};

const uint8_t ruSupport0[] = "Настр. по Т ком.";
const uint8_t enSupport0[] = "Conf. by T room ";
const uint8_t ruSupport1[] = "Настр. по Т выс.";
const uint8_t enSupport1[] = "Conf. by T high ";
const uint8_t ruSupport2[] = "Настр. давления ";
const uint8_t enSupport2[] = "Config. pressure";
const uint8_t ruSupport3[] = "Тест поршня     ";
const uint8_t enSupport3[] = "Cylinder test   ";

const uint8_t *const MsgSupport[][2] = 
{
   {ruUserService0, enUserService0},
   {ruUserService1, enUserService1},
   {ruUserService2, enUserService2},
   {ruUserService3, enUserService3},
   {    ruSupport0,     enSupport0},
   {    ruSupport1,     enSupport1},
   {    ruSupport2,     enSupport1},
   {    ruSupport3,     enSupport1},
   {  UserService4,   UserService4},
   {ruUserService5, enUserService5},
   {ruUserService6, enUserService6},
   {ruUserService7, enUserService7},
   { ruUserService,  enUserService}, // This one should be the last one ALWAYS
};

void ShowSupport(void)
{
   PutStr_LCD(0, 0, MsgSupport[sizeof(MsgSupport[1])][language], 0);
   PutStr_LCD(0, 1, MsgSupport[supportSelectCntr][language], 0);
}

void (*const ShowSupportModes[])(void) =
{
   ShowSupport,
};
