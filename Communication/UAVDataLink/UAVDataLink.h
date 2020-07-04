#ifndef UAV_DATA_LINK_H
#define UAV_DATA_LINK_H

#include <stdint.h>

/*
*
*   Title: UAVDataLink
*   Author: Philip M. Salmony
*   Date: 3 October 2019
*
*   Description: Used to frame a payload and a payload header for reliable communication.
*                Packet is structured as follows [SEQUENCE, IDA, IDB, PAYLOADLENGTH, PAYLOAD, CHECKSUM]
*
*                SEQUENCE: Used to identify packetloss. Runs from 1 to 255 and then wraps round to 1.
*                IDA: Main packet identifier (e.g. to address a particular system)
*                IDB: Sub packet identifier (e.g. to address a particular subsystem, belonging to the main system)
*                PAYLOADLENGTH: Number of bytes that make up the PAYLOAD.
*                PAYLOAD: Main message body formatted as uint8_t's.
*                CHECKSUM: Simple checksum to check for integrity of packet upon receiving them.
*
*/

/* Function definitions */
uint8_t UAVDataLink_pack(const uint8_t IDA, const uint8_t IDB, const uint8_t PAYLOADLENGTH, const uint8_t *PAYLOAD, uint8_t *byteStreamOut);
uint8_t UAVDataLink_unpack(uint8_t *receivedPacket, uint8_t receivedPacketLength, uint8_t *packetHeader, uint8_t *payload);
uint8_t UAVDataLink_encodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_decodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_payloadToFloats(const uint8_t *payload, const uint8_t payloadLength, float *extractedFloats);
uint8_t UAVDataLink_checksum(const uint8_t *arr, const uint8_t arrLength);

typedef union FloatConverterUnion {
    float val;
    uint8_t bytes[4];
} FloatConverter;

uint8_t UAVDataLink_pack(const uint8_t IDA, const uint8_t IDB, const uint8_t PAYLOADLENGTH, const uint8_t *PAYLOAD, uint8_t *byteStreamOut);
uint8_t UAVDataLink_unpack(uint8_t *receivedPacket, uint8_t receivedPacketLength, uint8_t *packetHeader, uint8_t *payload);
uint8_t UAVDataLink_payloadToFloats(const uint8_t *payload, const uint8_t payloadLength, float *extractedFloats);
uint8_t UAVDataLink_encodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_decodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_checksum(const uint8_t *arr, const uint8_t arrLength);

#endif // UAV_DATA_LINK_H