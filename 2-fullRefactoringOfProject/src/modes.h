#ifndef MODES_H_
#define MODES_H_
typedef enum
{
   MAIN_MODE = 0,
   SERVICE_MODE,
   SUPPORT_MODE,
} ModeTypes;

typedef enum
{
   M_TITLE = 0,
   M_ENTRY,
   M_PROGRAMS,
   M_HEATING,
   M_HOLDING,
   M_PRESSING,
   M_COMPLETED,
   M_ALERT,
   M_REQUEST_UPDATE_FIRMWARE_PECH_MODE,
   M_REQUEST_UPDATE_FIRMWARE_PULT_MODE,
   M_WRITE_FIRMWARE_PECH,
   M_EXT_MANAGMENT,
   M_SYSTEM_RESET,
} MainModes;

typedef enum
{
   SERV_USERS_SERVICE = 0,
   SERV_LANGUAGE,
   SERV_FLOW,
   SERV_ROUTER,
   SERV_MAINTAIN_TEMPERATURE,
   SERV_IP,
   SERV_REGISTRATION,
   SERV_NETWORK_NUMBER,
   SERV_VERSIONS,
} ServiceModes;

typedef enum
{
   SUPP_SERVICE = 0,
   SUPP_LANGUAGE,
   SUPP_FLOW,
   SUPP_ROUTER,
   SUPP_MAINTAIN_TEMPERATURE,
   SUPP_BY_TROOM_SETUP,
   SUPP_BY_THOT_SETUP,
   SUPP_PRESS_SENS_SETUP,
   SUPP_CYLINDER_TEST,
   SUPP_IP,
   SUPP_REGISTRATION,
   SUPP_NETWORK_NUMBER,
   SUPP_VERSIONS,
} SupportModes;

void ChangeToMainModeType(MainModes *pMode, ModeTypes *pModeType, MainModes mode);
void ChangeToSupportModeType(SupportModes *pMode, ModeTypes *pModeType, SupportModes mode);
void ChangeToServiceModeType(ServiceModes *pMode, ModeTypes *pModeType, ServiceModes mode);
void HandleMainModes(MainModes *mode, int8_t *keyCode);
void HandleServiceModes(ServiceModes *serviceMode, int8_t *keyCode);
void HandleSupportModes(SupportModes *supportMode, int8_t *keyCode);

ModeTypes modeType;       // current mode type holder
MainModes mode;           // current main mode holder
ServiceModes serviceMode; // current service mode holder
uint8_t serviceSelectCntr;  // for choosing service mode to enter
SupportModes supportMode; // current support mode holder
uint8_t supportSelectCntr;
#endif
