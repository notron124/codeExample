#ifndef KEYFUNCTIONS_H_
#define KEYFUNCTIONS_H_

extern void (*const MainModeKeyHandler[][8])(void);
extern void (*const ServiceModeKeyHandler[][8])(void);
extern void (*const SupportModeKeyHandler[][8])(void);
#endif
