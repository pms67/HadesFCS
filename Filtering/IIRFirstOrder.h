/*
*
* IIR (Infinite Impulse Response) First-Order Filter
*
* Implements a discrete-time first-order IIR filter.
* Constant term alpha sets the amount of filtering. 0 = no filtering, 1 = max. filtering
* 
* Written by: Philip M. Salmony @ philsal.co.uk
* Last changed: 01 Dec 2019
*
*/

#ifndef IIR_FIRST_ORDER_H
#define IIR_FIRST_ORDER_H

typedef struct {
	float alpha;
	float out;
} IIRFirstOrder;

void IIRFirstOrder_Init(IIRFirstOrder *filt, float alpha) {
	filt->alpha = alpha;
	filt->out   = 0.0f;
}

float IIRFirstOrder_Update(IIRFirstOrder *filt, float in) {
	filt->out = filt->alpha * filt->out + (1.0f - filt->alpha) * in;
	return filt->out;
}

#endif