#include "LowPassFilter.h"

void LPFTwoPole_Init(LPFTwoPole *lpf, uint8_t type, float cutoffFrequency, float sampleTime) {
	float wc = 6.28318530718f * cutoffFrequency;

	if (type == LPF_TYPE_BESSEL) {
		lpf->coeffNum = 1.6221f * wc * wc * sampleTime * sampleTime;
		lpf->coeffDen[0] = 1.0f / (1.0f + 2.206f * wc * sampleTime + lpf->coeffNum);
		lpf->coeffDen[1] = -(2.0f + 2.206f * wc * sampleTime);
	}

	lpf->buf[0] = 0.0f;
	lpf->buf[1] = 0.0f;

	lpf->out = 0.0f;
}

float LPFTwoPole_Update(LPFTwoPole *lpf, float val) {
	lpf->buf[1] = lpf->buf[0];
	lpf->buf[0] = lpf->out;

	lpf->out = lpf->coeffDen[0] * (lpf->coeffNum * val - (lpf->coeffDen[1] * lpf->buf[0] + lpf->buf[1]));

	return lpf->out;
}
