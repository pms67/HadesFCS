/*
*
*   Title: UAVDataLink
*   Author: Philip M. Salmony
*   Date: 3 October 2019
*
*   Description: Used to frame a payload and a payload header for reliable communication.
*                Packet is structured as follows [SEQUENCE, IDA, IDB, PAYLOADLENGTH, PAYLOAD, CHECKSUM]
*
*                SEQUENCE: Used to identify packetloss. Runs from 1 to 255 and then wraps back around to 1.
*                IDA: Main packet identifier (e.g. to address a particular system).
*                IDB: Sub packet identifier (e.g. to address a particular subsystem, belonging to the main system of IDA).
*                PAYLOADLENGTH: Number of bytes that make up the PAYLOAD.
*                PAYLOAD: Main message body formatted as uint8_t's.
*                CHECKSUM: Simple checksum to check for integrity of packet upon receiving them.
*
*/

#ifndef UAV_DATA_LINK_H
#define UAV_DATA_LINK_H

#include <stdint.h>

/* Function definitions */
uint8_t UAVDataLink_Pack(const uint8_t IDA, const uint8_t IDB, const uint8_t PAYLOADLENGTH, const uint8_t *PAYLOAD, uint8_t *byteStreamOut);
uint8_t UAVDataLink_Unpack(uint8_t *receivedPacket, uint8_t receivedPacketLength, uint8_t *packetHeader, uint8_t *payload);
uint8_t UAVDataLink_EncodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_DecodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut);
uint8_t UAVDataLink_PayloadToFloats(const uint8_t *payload, const uint8_t payloadLength, float *extractedFloats);
uint8_t UAVDataLink_Checksum(const uint8_t *arr, const uint8_t arrLength);

typedef union FloatConverterUnion {
    float val;
    uint8_t bytes[4];
} FloatConverter;

#endif // UAV_DATA_LINK_H
