#include <stdio.h>
#include <stdlib.h>

#include "UAVDataLink.h"

struct DataContainer_INS {
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    float altitude;
} INSData;

int main()
{

    /* Prepare INS data packet */
    INSData.rollDeg = 5.0;
    INSData.pitchDeg = -2.4;
    INSData.yawDeg = 69.69;
    INSData.altitude = 100.0;

    /* Encode as UAV data packet */
    uint8_t UAVDataPacket[256];
    uint8_t UAVDataPacketLength;
    UAVDataPacketLength = UAVDataLink_pack(3, 69, sizeof(INSData), (const uint8_t *) &INSData, UAVDataPacket);

    printf("Packed length: %i\r\n", UAVDataPacketLength);

    for (uint8_t n = 0; n < UAVDataPacketLength; n++) {
        printf("%i ", UAVDataPacket[n]);
    }

    uint8_t rxHeader[4];
    uint8_t rxPayload[256];
    uint8_t checksumCorrect = UAVDataLink_unpack(UAVDataPacket, UAVDataPacketLength, rxHeader, rxPayload);

    printf("\r\nChecksum correct = %i\r\n", checksumCorrect);
    printf("Header: SEQUENCE=%i ID_A=%i ID_B=%i PAYLOADLENGTH=%i\r\n", rxHeader[0], rxHeader[1], rxHeader[2], rxHeader[3]);

    float extractedFloats[4];
    uint8_t nFloats = UAVDataLink_payloadToFloats(rxPayload, rxHeader[3], extractedFloats);

    printf("Recovered data: %f %f %f %f", extractedFloats[0], extractedFloats[1], extractedFloats[2], extractedFloats[3]);

    getchar();
    return 0;
}
