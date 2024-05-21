#ifndef INTERFACE_H_
#define INTERFACE_H_

uint8_t language;
uint8_t deviceID;

extern void (*const ShowMainModes[])(void);
extern void (*const ShowServiceModes[])(void);
extern void (*const ShowSupportModes[])(void);
#endif
