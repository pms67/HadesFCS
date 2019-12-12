#include "GPSNMEAParser.h"

void GPSNMEAParser_Init(GPSData *gpsData) {
	gpsData->curSentence = NONE;
	gpsData->readingHeader = 0;
	gpsData->readingSentenceData = 0;
	gpsData->headerBufIndex = 0;
	gpsData->segmentBufIndex = 0;
	gpsData->segmentCount = 0;
	
	/* Null-terminate header buffer (needed for strcmp function) */
	gpsData->headerBuf[5] = '\0';
}

void GPSNMEAParser_ExtractGGA(GPSData *gpsData) {
	if (gpsData->segmentCount == 2) { /* Latitude */
		
		/* Extract degrees */
		char latDegBuf[] = {gpsData->segmentBuf[0], gpsData->segmentBuf[1]};
			
		/* Extract minutes */
		char cLatMin[gpsData->segmentBufIndex - 2];
		for (int n = 2; n < gpsData->segmentBufIndex; n++) {
			cLatMin[n - 2] = gpsData->segmentBuf[n];
		}
		
		/* Convert to decimal */
		gpsData->latitude_dec = ((float) atoi(latDegBuf)) + ((float) atof(cLatMin)) / 60.0f;	
		
	} else if (gpsData->segmentCount == 3) { /* N/S */
	
		gpsData->latitudeNS = (gpsData->segmentBuf[0] == 'S');
		
	} else if (gpsData->segmentCount == 4) { /* Longitude */
	
		/* Extract degrees */
		char lonDegBuf[] ={gpsData->segmentBuf[0], gpsData->segmentBuf[1], gpsData->segmentBuf[2]};
	
		/* Extract minutes */
		char cLonMin[gpsData->segmentBufIndex - 3];
		for (int n = 3; n < gpsData->segmentBufIndex; n++) {
			cLonMin[n - 3] = gpsData->segmentBuf[n];
		}
		
		/* Convert to decimal */
		gpsData->longitude_dec = ((float) atoi(lonDegBuf)) + ((float) atof(cLonMin)) / 60.0f;	
		
	} else if (gpsData->segmentCount == 5) { /* E/W */
	
		gpsData->longitudeEW = (gpsData->segmentBuf[0] == 'W');
		
	} else if (gpsData->segmentCount == 6) { /* Fix quality */
	
		gpsData->fixQuality = (uint8_t) (gpsData->segmentBuf[0] - '0'); /* Convert char to int */
		
	} else if (gpsData->segmentCount == 7) { /* Number of satellites being tracked */
	
		gpsData->numSatellites = ((uint8_t) (gpsData->segmentBuf[0] - '0')) * 10 + (uint8_t) (gpsData->segmentBuf[1] - '0');
	
	} else if (gpsData->segmentCount == 9) { /* Altitude above mean sea level */
	
		/* Extract only altitude from buffer */
		char cAlt[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cAlt[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->altitude_m = (float) atof(cAlt);	
	
	} else if (gpsData->segmentCount == 11) { /* Height of geoid (mean sea level) above WGS84 ellipsoid */
		
		/* Extract only altitude from buffer */
		char cMSL[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cMSL[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->meanSeaLevel_m = (float) atof(cMSL);	
		
	}
}

void GPSNMEAParser_ExtractRMC(GPSData *gpsData) {
	if (gpsData->segmentCount == 2) { /* Fix (active or void) */
	
		gpsData->fix = (gpsData->segmentBuf[0] == 'A');
	
	} else if (gpsData->segmentCount == 3) { /* Latitude */
		
		/* Extract degrees */
		char latDegBuf[] = {gpsData->segmentBuf[0], gpsData->segmentBuf[1]};
			
		/* Extract minutes */
		char cLatMin[gpsData->segmentBufIndex - 2];
		for (int n = 2; n < gpsData->segmentBufIndex; n++) {
			cLatMin[n - 2] = gpsData->segmentBuf[n];
		}
		
		/* Convert to decimal */
		gpsData->latitude_dec = ((float) atof(latDegBuf)) + ((float) atof(cLatMin)) / 60.0f;		
		
	} else if (gpsData->segmentCount == 4) { /* N/S */
	
		gpsData->latitudeNS = (gpsData->segmentBuf[0] == 'S');
		
	} else if (gpsData->segmentCount == 5) { /* Longitude */
	
		/* Extract only longitude from buffer */
		char cLon[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cLon[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->longitude_dec = (float) atof(cLon);	
		
	} else if (gpsData->segmentCount == 6) { /* E/W */
	
		gpsData->longitudeEW = (gpsData->segmentBuf[0] == 'W');
		
	} else if (gpsData->segmentCount == 7) { /* Ground speed (in knots) */
	
		/* Extract only ground speed from buffer */
		char cSpd[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cSpd[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->groundSpeed_mps = 0.514f * ((float) atof(cSpd)); /* Convert knots to m/s */
		
	} else if (gpsData->segmentCount == 8) { /* Course (in deg) */
	
		/* Extract only ground speed from buffer */
		char cCrs[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cCrs[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->course_deg = 0.01745329251f * ((float) atof(cCrs)); /* Convert degrees to radians */
	
	} else if (gpsData->segmentCount == 10) { /* Magnetic variation */
	
		/* Extract only magnetic variation from buffer */
		char cMag[gpsData->segmentBufIndex];
		for (int n = 0; n < gpsData->segmentBufIndex; n++) {
			cMag[n] = gpsData->segmentBuf[n];
		}
		
		gpsData->magVariation_deg = 0.01745329251f * ((float) atof(cMag));	/* Convert degrees to radians */
	
	}

	/*
	* MAGNETIC VARIATION COMES WITH N/W/S/E ??? 
	*/
	
}

void GPSNMEAParser_Feed(GPSData *gpsData, char c) {		
	
	if (gpsData->readingHeader) {
				
			gpsData->headerBuf[gpsData->headerBufIndex] = c;
			gpsData->headerBufIndex++;
			
			/* Check if all header characters have been read in */
			if (gpsData->headerBufIndex == 5) {
				gpsData->readingHeader = 0;
				gpsData->readingSentenceData = 1;
				
				/* Extract sentence type */
				if (!strcmp(gpsData->headerBuf, "GNGGA")) {
					gpsData->curSentence = GGA;
				} else if (!strcmp(gpsData->headerBuf, "GNRMC")) {
					gpsData->curSentence = RMC;
				} else if (!strcmp(gpsData->headerBuf, "GNVTG")) {
					gpsData->curSentence = VTG;
				} else { /* Unknown sentence type (or not implemented yet) */
					gpsData->curSentence = 0;
					gpsData->readingSentenceData = 0;
				}
					
								
				/* Reset sentence segment buffer and segment counter */
				gpsData->segmentBufIndex = 0;
				gpsData->segmentCount = 0;								
			}
			
	} else {
			
		if (c == '$') { /* Start of sentence */
			gpsData->readingHeader = 1;
			gpsData->readingSentenceData = 0;
			gpsData->headerBufIndex = 0;
		}
		
		if (gpsData->readingSentenceData) {	/* Extract sentence data */		
			
			if (c == '\r' || c == '\n') { /* End of sentence */
				
				gpsData->readingSentenceData = 0;
				
			} else {
				
				if (c == ',') { /* End of segment */						
					
					if (gpsData->segmentCount > 1) { /* Start from second 'argument' */
						
						if (gpsData->curSentence == GGA) { /* GGA */
#ifdef GPS_USE_GGA
							GPSNMEAParser_ExtractGGA(gpsData);
#endif			
						} else if (gpsData->curSentence == RMC) { /* RMC */
#ifdef GPS_USE_RMC
							GPSNMEAParser_ExtractRMC(gpsData);
#endif			
						} else if (gpsData->curSentence == VTG) { /* VTG */
							
							/*
							* NOT IMPLEMENTED YET (SEEMS LIKE DATA IS AVAILABLE IN RMC...)
							*/
							
						}							
		
					}
					
					gpsData->segmentBufIndex = 0;
					gpsData->segmentCount++;
					
				} else {
					gpsData->segmentBuf[gpsData->segmentBufIndex] = c;						
					gpsData->segmentBufIndex++;
				}
				
			}			
			
		}
			
	}
	
}
