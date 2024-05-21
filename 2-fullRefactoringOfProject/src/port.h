#ifndef PORT_H_
#define PORT_H_

// GPIOA
// OUT
#define P_SND        GPIO_ODR_ODR0
#define P_W_RST      GPIO_ODR_ODR1
#define P_VALVE1     GPIO_ODR_ODR2
#define P_HEATER     GPIO_ODR_ODR3
#define P_VALVE2     GPIO_ODR_ODR7
#define P_WIFI_LED   GPIO_ODR_ODR11

// GPIOB


// IN
#define KEY_GPIO     GPIOB
#define P_KEY1       GPIO_ODR_ODR4
#define P_KEY2       GPIO_ODR_ODR5
#define P_KEY3       GPIO_ODR_ODR6
#define P_KEY4       GPIO_ODR_ODR7

#endif