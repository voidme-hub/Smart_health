#ifndef KEY_H_
#define KEY_H_

#include "ch32v30x.h"

#define KEY_GPIO GPIOC
#define KEY_PIN GPIO_Pin_1

void key_init(void);
u8 key_is_pressed(void);

#endif



