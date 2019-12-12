#ifndef GPS_NMEA_PARSER_H
#define GPS_NMEA_PARSER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define GPS_USE_GGA
//#define GPS_USE_RMC

typedef enum {
	NONE,
	GGA,
	RMC,
	VTG
} NMEASentenceType;

typedef struct {
	float latitude_dec;
	float longitude_dec;
	float altitude_m;
	float groundSpeed_mps;
	float course_deg;
	float magVariation_deg;
	float meanSeaLevel_m;
	uint8_t fix;
	uint8_t fixQuality;
	uint8_t numSatellites;
	uint8_t latitudeNS;
	uint8_t longitudeEW;
	
	/* Parsing variables */
	NMEASentenceType curSentence;
	uint8_t readingHeader;
	uint8_t readingSentenceData;
	char headerBuf[6];
	uint8_t headerBufIndex;
	char segmentBuf[16];
	uint8_t segmentBufIndex;
	uint8_t segmentCount;
} GPSData;


void GPSNMEAParser_Init(GPSData *gpsData);
void GPSNMEAParser_ExtractGGA(GPSData *gpsData);
void GPSNMEAParser_ExtractRMC(GPSData *gpsData);
void GPSNMEAParser_Feed(GPSData *gpsData, char c);

#endif

