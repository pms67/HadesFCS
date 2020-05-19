#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "GDSPOverdrive.h"

int main(int argc, char *argv[]) {
	FIRLPF lpf;
	FIRLPF_Init(&lpf, 10.0f, 100.0f, 32);
	
	for (uint16_t n = 0; n < 32; n++) {
		printf("%f ", *lpf.coeff++);
	}
	
	return 0;
}
