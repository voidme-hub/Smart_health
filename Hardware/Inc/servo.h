#ifndef __SERVO_H__
#define __SERVO_H__

#include "ch32v30x.h"

void Servo_Init(void);
void Servo_SetAngle(float angle);
u8 parse_servo_angle(const char *s, float *out);

#endif

