using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NAVCDataInterface
{
    class UAVDataLinkHandler
    {
        private int rxBufSize;
        private int rxBufIndex;
        private byte[] rxBuf;

        public int packetLength;
        public byte[] packetBuf;

        public int SEQUENCE;
        public int IDA;
        public int IDB;
        public int PAYLOADLENGTH;
        public byte[] PAYLOAD;
        public bool CHECKSUMCORRECT;
        public byte CHECKSUM;
        public byte RXCHECKSUM;

        public UAVDataLinkHandler(int rxBufSize)
        {
            /* Initialise variables */
            this.rxBufSize = rxBufSize;
                        
            rxBuf = new byte[rxBufSize];
            for (int n = 0; n < rxBufSize; n++)
            {
                rxBuf[n] = 0;
            }

            rxBufIndex = 0;

            packetBuf = new byte[rxBufSize];
            packetLength = 0;
        }

        public bool Feed(byte[] buf, int numRx)
        {
            bool validPacketReceived = false;

            for (int n = 0; n < numRx; n++)
            {
                if (buf[n] == 0)
                {
                    /* Decode COBS */
                    DecodeCOBS();

                    /* Extract packet parameters */
                    SEQUENCE = packetBuf[0];
                    IDA = packetBuf[1];
                    IDB = packetBuf[2];
                    PAYLOADLENGTH = packetBuf[3];
                    PAYLOAD = new byte[PAYLOADLENGTH];

                    /* Extract packet payload and compute checksum */
                    CHECKSUM = 0;

                    for (int i = 0; i < PAYLOADLENGTH; i++)
                    {
                        PAYLOAD[i] = packetBuf[4 + i];
                        CHECKSUM ^= PAYLOAD[i];
                    }

                    /* Check if checksums match */
                    RXCHECKSUM = rxBuf[4 + PAYLOADLENGTH + 1];
                    if (CHECKSUM == RXCHECKSUM)
                    {
                        CHECKSUMCORRECT = true;
                        validPacketReceived = true;
                    }

                    /* Reset RX buffer */
                    for (int i = 0; i < rxBufIndex; i++)
                    {
                        rxBuf[i] = 0;
                    }
                    rxBufIndex = 0;

                    /* Reset packet buffer */
                    for (int i = 0; i < packetLength; i++)
                    {
                        packetBuf[i] = 0;
                    }
                } else
                {
                    rxBuf[rxBufIndex] = buf[n];
                    rxBufIndex++;
                }
            }

            return validPacketReceived;
        }

        private void DecodeCOBS()
        {
            packetLength = 0;
            int nextZeroIndex = rxBuf[0]; /* First byte sets location of first zero */

            for (int dataInIndex = 1; dataInIndex < rxBufIndex; dataInIndex++)
            {
                if (dataInIndex == nextZeroIndex)
                {
                    packetBuf[packetLength] = 0;
                    nextZeroIndex = dataInIndex + rxBuf[dataInIndex];
                } else
                {
                    packetBuf[packetLength] = rxBuf[dataInIndex];
                }

                packetLength++;
            }
        }

    }
}
