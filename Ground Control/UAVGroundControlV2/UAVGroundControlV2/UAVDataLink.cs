using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UAVGroundControlV2
{
    public class UAVDataLink
    {

        public byte[] Unpack(byte[] rxBuffer, int nBytes, ref byte[] packetHeader, ref bool csCheck)
        {

            /* Decode COBS framing */
            byte[] decodeBuffer = DecodeCOBS(rxBuffer, nBytes);

            /* Extract packet parameters and place in header */
            packetHeader[0] = decodeBuffer[0]; /* SEQUENCE */
            packetHeader[1] = decodeBuffer[1]; /* IDA */
            packetHeader[2] = decodeBuffer[2]; /* IDB */
            packetHeader[3] = decodeBuffer[3]; /* PAYLOADLENGTH */

            /* Extract packet payload and compute checksum */
            List<byte> payload = new List<byte>();
            int cs = 0;

            for (int n = 0; n < (int) packetHeader[3]; n++)
            {
                payload.Add(decodeBuffer[4 + n]);
                cs ^= payload[n];
            }

            /* Check if checksums match */
            csCheck = (cs == decodeBuffer[4 + packetHeader[3]]);

            return payload.ToArray();

        }

        public float[] PayloadToFloats(byte[] payload)
        {
            /* Check if payload length is divisible by four, i.e. four bytes per float */
            if (payload.Length % 4 != 0)
            {
                return new float[] { 0.0f };
            }

            int nFloats = payload.Length / 4;
            float[] extractedFloats = new float[nFloats];

            for (int n = 0; n < nFloats; n++)
            {

                extractedFloats[n] = ToFloat(payload[4 * n], payload[4 * n + 1], payload[4 * n + 2], payload[4 * n + 3]);
                
            }

            return extractedFloats;
        }

        private float ToFloat(byte b0, byte b1, byte b2, byte b3)
        {
            byte[] newArray = new[] { b0, b1, b2, b3 };
            return BitConverter.ToSingle(newArray, 0);
        }

        private byte[] DecodeCOBS(byte[] rxBuffer, int nBytes)
        {

            int dataOutLength = 0;
            int nextZeroIndex = (int) rxBuffer[0];

            List<byte> dataOut = new List<byte>();

            for (int dataInIndex = 1; dataInIndex < nBytes - 1; dataInIndex++)
            {

                if (dataInIndex == nextZeroIndex)
                {

                    dataOut.Add(0);
                    nextZeroIndex = dataInIndex + (int)rxBuffer[dataInIndex];

                } else
                {

                    dataOut.Add(rxBuffer[dataInIndex]);

                }

                dataOutLength++;

            }

            return dataOut.ToArray();

        }

    }
}
