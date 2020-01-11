#ifndef PWM_LOOKUP_H
#define PWM_LOOKUP_H

#define PWMLOOKUP_NUM_ENTRIES 6

uint16_t PWMLookup_PWM[] = {1300, 1400, 1500, 1600, 1700, 1800};
float    PWMLookup_F[]   = {0.0981f, 0.5886f, 1.3734f, 2.4525f, 3.6297f, 4.7088f};
float	 PWMLookup_dPWMdF[]  = {1020, 204, 127, 93, 85, 93};

int16_t PWMLookup_GetDeltaPWM(uint16_t basePWM, float deltaF) {
	for (int n = 0; n < PWMLOOKUP_NUM_ENTRIES - 1; n++) {
		if (basePWM >= PWMLookup_PWM[n] && basePWM <= PWMLookup_PWM[n + 1]) {
			int16_t deltaPWM = (int16_t) (PWMLookup_dPWM[n] * deltaF);
			return deltaPWM;
		}
	}

	/* Only will reach here if basePWM is out of bounds */
	return 0;
}

#endif
