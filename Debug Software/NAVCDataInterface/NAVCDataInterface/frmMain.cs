using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO.Ports;

namespace NAVCDataInterface
{
    public partial class frmMain : Form
    {
        /* Serial port */
        SerialPort port;
        bool serialPortConnected;
        int numBytesRx;
        int numBytesTx;

        /* Data packet handler */
        UAVDataLinkHandler packetHandler;
        int numValidPackets;

        /* NAVC data container */
        float[] NavDataContainer;

        public frmMain()
        {
            InitializeComponent();

            serialPortConnected = false;
            numBytesRx = 0;
            numBytesTx = 0;

            packetHandler = new UAVDataLinkHandler(256);
            numValidPackets = 0;

            NavDataContainer = new float[20];
        }

        private void frmMain_Load(object sender, EventArgs e)
        {
            /* Get available serial ports */
            string[] ports = SerialPort.GetPortNames();

            /* Set selected combo box items */
            if (ports.Length > 0)
            {
                foreach (string s in ports)
                {
                    cmbPort.Items.Add(s);
                }

                cmbPort.SelectedIndex = 0;
            }

            /* Default baud rate is 256000 */
            cmbBaud.SelectedIndex = 7;
        }

        private void btnRefreshPorts_Click(object sender, EventArgs e)
        {
            /* Remove items already in list */
            cmbPort.Items.Clear();

            /* Get available serial ports */
            string[] ports = SerialPort.GetPortNames();

            /* Set selected combo box item */
            if (ports.Length > 0)
            {
                foreach (string s in ports)
                {
                    cmbPort.Items.Add(s);
                }

                cmbPort.SelectedIndex = 0;
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (serialPortConnected)
            {
                /* Check for confirmation */
                if (MessageBox.Show("Disconnect from serial port?", "Disconnect", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {

                    port.Close();
                    port = null;

                    serialPortConnected = false;

                    btnConnect.Text = "Connect";
                    statusLabel.Text = "Serial port closed.";

                }

            } else
            {

                try
                {

                    port = new SerialPort(cmbPort.Text, Convert.ToInt32(cmbBaud.Text));

                    port.Open();

                    if (port.IsOpen)
                    {
                        serialPortConnected = true;

                        /* Add callback */
                        port.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

                        numBytesRx = 0;
                        numBytesTx = 0;
                        numValidPackets = 0;

                        btnConnect.Text = "Disconnect";
                        statusLabel.Text = "Serial port opened.";
                    }

                } catch (Exception ex)
                {
                    MessageBox.Show("Could not connect to serial port (Exception: " + ex.ToString() + ").", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

            }
        }

        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            if (port.IsOpen)
            {
                /* Read bytes in buffer */
                int numBytes = port.BytesToRead;

                byte[] buffer = new byte[numBytes];
                port.Read(buffer, 0, numBytes);

                /* Parse data */           
                bool validPacketReceived = packetHandler.Feed(buffer, numBytes);

                if (validPacketReceived)
                {
                    numValidPackets++;

                    ConvertPacket();                                  
                }                

                numBytesRx += numBytes;

                statusLabel.Text = "RX: " + numBytesRx.ToString() + " TX: " + numBytesTx.ToString() + " P: " + numValidPackets.ToString();
            }            

        }

        private void ConvertPacket()
        {
            /* Sensor, GPS, and EKF data packet */
            if (packetHandler.IDA == 0 && packetHandler.IDB == 0)
            {
                byte[] bytes = new byte[4];

                string dataPacketString = "";

                for (int n = 0; n < 20; n++)
                {
                    bytes[0] = packetHandler.PAYLOAD[4 * n];
                    bytes[1] = packetHandler.PAYLOAD[4 * n + 1];
                    bytes[2] = packetHandler.PAYLOAD[4 * n + 2];
                    bytes[3] = packetHandler.PAYLOAD[4 * n + 3];

                    NavDataContainer[n] = BytesToFloat(bytes);
                    
                    dataPacketString += NavDataContainer[n].ToString();
                    if (n != 19)
                    {
                        dataPacketString += ",";
                    }
                }

                /* Print to log */
                Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(dataPacketString); }));
                Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(Environment.NewLine); }));

                /* Update GUI */
                UpdateGUIText();
            } else
            {
                PrintToLog();
            }
        }

        private void UpdateGUIText()
        {
            Invoke(new MethodInvoker(delegate () { lblAccX.Text = NavDataContainer[0].ToString("F2"); lblAccY.Text = NavDataContainer[1].ToString("F2"); lblAccZ.Text = NavDataContainer[2].ToString("F2");
                lblGyrX.Text = (NavDataContainer[3] * 180.0f / Math.PI).ToString("F2"); lblGyrY.Text = (NavDataContainer[4] * 180.0f / Math.PI).ToString("F2"); lblGyrZ.Text = (NavDataContainer[5] * 180.0f / Math.PI).ToString("F2");
                lblMagX.Text = NavDataContainer[6].ToString("F2"); lblMagY.Text = NavDataContainer[7].ToString("F2"); lblMagZ.Text = NavDataContainer[8].ToString("F2");
                lblBarPressure.Text = NavDataContainer[9].ToString("F2"); lblBarAlt.Text = AltitudeFromPressure(NavDataContainer[9]).ToString("F2");
                lblPitotPressure.Text = NavDataContainer[10].ToString("F2"); lblPitotAirspeed.Text = "0.0";
                lblGPSFix.Text = NavDataContainer[11].ToString("F0"); lblGPSLat.Text = NavDataContainer[12].ToString(); lblGPSLon.Text = NavDataContainer[13].ToString(); lblGPSAlt.Text = NavDataContainer[14].ToString("F2"); lblGPSGroundSpeed.Text = NavDataContainer[15].ToString("F2"); lblGPSCourse.Text = NavDataContainer[16].ToString("F2");
                lblEKFRoll.Text = NavDataContainer[17].ToString("F2"); lblEKFPitch.Text = NavDataContainer[18].ToString("F2"); lblEKFHeading.Text = NavDataContainer[19].ToString("F2"); }));
        }

        private void PrintToLog()
        {
            /* Packet header on separate line */
            string dataPacketHeaderString = "[" + numValidPackets.ToString() + "][#" + packetHandler.SEQUENCE.ToString() + "][A" + packetHandler.IDA.ToString() + "|B" + packetHandler.IDB.ToString() + "][N=" + packetHandler.PAYLOADLENGTH.ToString() + "][CS" + packetHandler.RXCHECKSUM + "]";
            Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(dataPacketHeaderString); }));
            Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(Environment.NewLine); }));

            /* Packet payload */            
            string dataPacketString = "";
            for (int n = 0; n < packetHandler.PAYLOADLENGTH - 1; n++)
            {
                dataPacketString += packetHandler.PAYLOAD[n].ToString() + ",";
            }
            

            dataPacketString += packetHandler.PAYLOAD[packetHandler.PAYLOADLENGTH - 1];

            Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(dataPacketString); }));
            Invoke(new MethodInvoker(delegate () { txtRawOutput.AppendText(Environment.NewLine); }));
        }

        private float BytesToFloat(byte[] bytes)
        {
            return BitConverter.ToSingle(bytes, 0);
        }

        private float AltitudeFromPressure(float pressure)
        {
            float altitude = -((float) Math.Log(pressure / 101325.0f)) * 8.3144598f * 288.15f / (9.80665f * 0.0289644f);

            return altitude;
        }
    }
}