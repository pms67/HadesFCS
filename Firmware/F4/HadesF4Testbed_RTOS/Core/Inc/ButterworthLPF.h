#ifndef BUTTERWORTHLPF_H
#define BUTTERWORTHLPF_H

typedef struct {
	float out;
	float coeffNum;
	float coeffDen[3];
	float samplesIn[2];
	float samplesOut[2];
} ButterworthLPF;

void ButterworthLPF_Init(ButterworthLPF *lpf, float cutoffFrequencyHz, float sampleTime) {
	float wc = 6.28318530718f * cutoffFrequencyHz;

	lpf->coeffNum = wc * wc * sampleTime * sampleTime;
	lpf->coeffDen[0] = 1.0f / (4.0f + 2.82842712475f * wc * sampleTime + lpf->coeffNum);
	lpf->coeffDen[1] = 2.0f * (lpf->coeffNum - 4.0f);
	lpf->coeffDen[2] = 4.0f - 2.82842712475f * wc * sampleTime + lpf->coeffNum;

	lpf->samplesIn[1]  = 0.0f;  lpf->samplesIn[0] = 0.0f;
	lpf->samplesOut[1] = 0.0f; lpf->samplesOut[0] = 0.0f;
	lpf->out = 0.0f;
}

float ButterworthLPF_Update(ButterworthLPF *lpf, float in) {
	/* Compute new filter output */
	lpf->out  = lpf->coeffNum * (in + 2.0f * lpf->samplesIn[0] + lpf->samplesIn[1]);
	lpf->out -= lpf->coeffDen[1] * lpf->samplesOut[0] + lpf->coeffDen[2] * lpf->samplesOut[1];
	lpf->out *= lpf->coeffDen[0];

	/* Shift samples */
	lpf->samplesIn[1] = lpf->samplesIn[0];
	lpf->samplesIn[0] = in;

	lpf->samplesOut[1] = lpf->samplesOut[0];
	lpf->samplesOut[0] = lpf->out;

	return lpf->out;
}

#endif
