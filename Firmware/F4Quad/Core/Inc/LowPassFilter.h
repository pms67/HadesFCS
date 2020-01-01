#ifndef LPF_H
#define LPF_H

#include <stdint.h>

#define LPF_TYPE_BESSEL 1

typedef struct {
	float out;
	float buf[2];
	float coeffNum;
	float coeffDen[2];
} LPFTwoPole;

void LPFTwoPole_Init(LPFTwoPole *lpf, uint8_t type, float cutoffFrequency, float sampleTime);
float LPFTwoPole_Update(LPFTwoPole *lpf, float val);

#endif
