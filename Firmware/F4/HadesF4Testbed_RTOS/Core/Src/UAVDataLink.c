#include "UAVDataLink.h"

uint8_t UAVDataLink_Pack(const uint8_t IDA, const uint8_t IDB, const uint8_t PAYLOADLENGTH, const uint8_t *PAYLOAD, uint8_t *byteStreamOut) {

    static uint8_t SEQUENCE = 1;
    uint8_t n;

    /* Create array to store packet data */
    uint8_t rawDataLength = 4 + PAYLOADLENGTH + 1; /* 4 header bytes, payload bytes, and checksum byte */
    uint8_t rawData[rawDataLength];

    /* Set packet header */
    rawData[0] = SEQUENCE;
    rawData[1] = IDA;
    rawData[2] = IDB;
    rawData[3] = PAYLOADLENGTH;

    /* Attach payload */
    for (n = 0; n < PAYLOADLENGTH; n++) {

        rawData[4 + n] = PAYLOAD[n];

    }

    /* Calculate checksum and set as last byte of packet */
    uint8_t cs = 0;
    for (n = 0; n < PAYLOADLENGTH; n++) {

        cs ^= PAYLOAD[n];

    }

    rawData[rawDataLength - 1] = cs;

    /* Encode with consistent overhead byte stuffing */
    uint8_t encodedPacketLength = UAVDataLink_EncodeCOBS(rawData, rawDataLength, byteStreamOut);

    /* Increment sequence number */
    if (SEQUENCE == 255) {

        SEQUENCE = 1;

    } else {

        SEQUENCE++;

    }

    return encodedPacketLength;

}

uint8_t UAVDataLink_Unpack(uint8_t *receivedPacket, uint8_t receivedPacketLength, uint8_t *packetHeader, uint8_t *payload) {

    /* Decode COBS framing */
    uint8_t decodeBuffer[receivedPacketLength]; /* CHECK LENGTH OF ARRAY !!!!!!!!!!!!!! */
    UAVDataLink_DecodeCOBS(receivedPacket, receivedPacketLength, decodeBuffer);

    /* Extract packet parameters and place in header */
    packetHeader[0] = decodeBuffer[0]; /* SEQUENCE */
    packetHeader[1] = decodeBuffer[1]; /* IDA */
    packetHeader[2] = decodeBuffer[2]; /* IDB */
    packetHeader[3] = decodeBuffer[3]; /* PAYLOADLENGTH */

    /* Extract packet payload and compute checksum */
    uint8_t cs = 0;

    for (uint8_t n = 0; n < packetHeader[3]; n++) {

        payload[n] = decodeBuffer[4 + n];
        cs ^= payload[n];

    }

    /* Check if checksums match */
    if (cs == decodeBuffer[4 + packetHeader[3]]) {

        return 1;

    } else {

        return 0;

    }

}

uint8_t UAVDataLink_PayloadToFloats(const uint8_t *payload, const uint8_t payloadLength, float *extractedFloats) {

    /* Check if payload length is divisible by four, i.e. four bytes per float */
    if (payloadLength % 4 != 0) {
        return 0;
    }

    FloatConverter fc;

    uint8_t nFloats = payloadLength / 4;

    for (uint8_t n = 0; n < nFloats; n++) {

        for (uint8_t f = 0; f < 4; f++) {
            fc.bytes[f] = payload[4 * n + f];
        }

        extractedFloats[n] = fc.val;

    }

    return nFloats;

}

uint8_t UAVDataLink_EncodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut) {

    uint8_t dataOutLength = 1; /* At least one header byte (set here) and one end byte (0x00) (set at end of function) */
    uint8_t dataOutIndex  = 0;
    uint8_t nextZeroIndex = 1; /* Initially, assume first byte is a zero */

    for (uint8_t dataInIndex = 0; dataInIndex < dataInLength; dataInIndex++) {

        if (dataIn[dataInIndex] == 0) {

            dataOut[dataOutIndex] = nextZeroIndex;

            nextZeroIndex = 1;
            dataOutIndex  = dataOutLength;

        } else {

            dataOut[dataOutLength] = dataIn[dataInIndex];

            nextZeroIndex++;

        }

        dataOutLength++;

    }

    dataOut[dataOutIndex] = nextZeroIndex;

    /* Append final, delimiting zero to mark end of packet */
    dataOut[dataOutLength] = 0;
    dataOutLength++;

    return dataOutLength;

}

uint8_t UAVDataLink_DecodeCOBS(const uint8_t *dataIn, const uint8_t dataInLength, uint8_t *dataOut) {

    uint8_t dataOutLength = 0;
    uint8_t nextZeroIndex = dataIn[0]; /* First bytes sets location of first zero */

    for (uint8_t dataInIndex = 1; dataInIndex < dataInLength - 1; dataInIndex++) {

        if (dataInIndex == nextZeroIndex) {

            dataOut[dataOutLength] = 0;
            nextZeroIndex = dataInIndex + dataIn[dataInIndex];

        } else {

            dataOut[dataOutLength] = dataIn[dataInIndex];

        }

        dataOutLength++;

    }

    return dataOutLength;

}

uint8_t UAVDataLink_Checksum(const uint8_t *arr, const uint8_t arrLength) {

    uint8_t cs = 0;

    for (uint8_t index = 0; index < arrLength; index++) {
        cs ^= arr[index];
    }

    return cs;

}
