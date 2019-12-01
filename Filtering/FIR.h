#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include <stdint.h>

typedef struct {
    float out;
	float *coeff;
	float *buf;
	uint8_t order;
	uint8_t putIndex;
} FIRFilter ;

void FIRFilter_Init(FIRFilter *filt, float *coeff, float *buf, const uint8_t order) {
	filt->out = 0.0f;
	
	filt->coeff = coeff;
	filt->buf   = buf;
	filt->order = order;
	filt->putIndex = 0;

	for (uint8_t n = 0; n < order; n++) {
		filt->buf[n] = 0.0f;
	}
}

float FIRFilter_Update(FIRFilter *filt, float in) {
    /* Store newest input value in circular buffer */
    filt->buf[filt->putIndex] = in;
    
    /* Compute filter output */
    uint8_t getIndex = filt->putIndex;
    
    filt->out = 0.0f;
    for (uint8_t n = 0; n < filt->order; n++) {
        filt->out = filt->out + filt->coeff[n] * filt->buf[getIndex];
        
        if (getIndex == 0) {
            getIndex = filt->order - 1;
        } else {
           getIndex--; 
        }
    }
    
    /* Increment buffer index */
    filt->putIndex++;
    if (filt->putIndex == filt->order) {
        filt->putIndex = 0;
    }
    
    /* Return output */
    return filt->out;
}

#endif
