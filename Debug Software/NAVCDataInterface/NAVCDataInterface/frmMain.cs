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

        public frmMain()
        {
            InitializeComponent();

            serialPortConnected = false;
            numBytesRx = 0;
            numBytesTx = 0;

            packetHandler = new UAVDataLinkHandler(256);
            numValidPackets = 0;
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

            /* Default baud rate is 115200 */
            cmbBaud.SelectedIndex = 5;
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

                    PrintToLog();                                      
                }                

                numBytesRx += numBytes;

                statusLabel.Text = "RX: " + numBytesRx.ToString() + " TX: " + numBytesTx.ToString() + " P: " + numValidPackets.ToString();
            }            

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
    }
}