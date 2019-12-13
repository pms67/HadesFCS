#include <stdio.h>
#include "GPSNMEAParser.h"

int main(int argc, char *argv[]) {

	/* Need to pass filename as argument */
	if (argc != 2) {
		printf("Usage: GPSNMEAParser.exe -filename\n");
		return -1;		
	}	
		
	/* Open file and check if readable */
	FILE *fp;
	
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Error opening file: %s", argv[1]);
	}	
	
	
	/* Create GPS data container object */
	GPSData gpsData;
	GPSNMEAParser_Init(&gpsData);
	
	/* Process data char by char */
	char c;
	uint32_t count = 0;
	
	while ( (c = fgetc(fp)) ) {
		
		if (c == EOF) {
			break;
		}
		
		GPSNMEAParser_Feed(&gpsData, c);
		
		printf("Fix: %d | Num: %d | Lat: %f | Lon: %f | Alt: %f | Spd: %f | Crs: %f | Mag: %f | MSL: %f \r\n", gpsData.fixQuality, gpsData.numSatellites, 
																												gpsData.latitude_dec, gpsData.longitude_dec, gpsData.altitude_m,
																												gpsData.groundSpeed_mps, gpsData.course_deg, gpsData.magVariation_deg, gpsData.meanSeaLevel_m);

		count++;
	}
	
	printf("Number of characters in file: %d\n", count);
	
	/* Close file */
	fclose(fp);

	return 0;
}


