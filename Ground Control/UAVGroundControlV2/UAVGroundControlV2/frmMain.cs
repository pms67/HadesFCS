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

namespace UAVGroundControlV2
{
    public partial class frmMain : Form
    {

        private SerialPort radio;
        private List<byte> rxBuffer;
        private bool comPortConnected;

        public frmMain()
        {
            InitializeComponent();
            
        }

        private void frmMain_Load(object sender, EventArgs e)
        {

            /* Add available COM ports to dropdown */
            string[] ports = SerialPort.GetPortNames();

            foreach (string port in ports)
            {
                cmbCOMPorts.Items.Add(port);
            }

            if (ports.Length > 0)
            {
                cmbCOMPorts.SelectedIndex = 0;
            }

            comPortConnected = false;

        }



        private void radioDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!radio.IsOpen)
            {
                return;
            }

            int nBytes = radio.BytesToRead;
            byte[] buffer = new byte[nBytes];

            radio.Read(buffer, 0, nBytes);

            for (int index = 0; index < nBytes; index++)
            {
                rxBuffer.Add(buffer[index]);

                if (buffer[index] == 0)
                {
                    // Decode packet
                    UAVDataLink dl = new UAVDataLink();
                    byte[] packetHeader = { 0, 0, 0, 0 };
                    bool csCheck  = false;

                    byte[] payload = dl.Unpack(rxBuffer.ToArray(), rxBuffer.Count, ref packetHeader, ref csCheck);

                    // Convert payload to floats (NEED TO CHANGE THIS DEPENDING ON RECEIVED PACKET!)
                    float[] payloadFloats = dl.PayloadToFloats(payload);

                    String payloadString = payloadFloats[0].ToString();
                    for (int n = 1; n < payloadFloats.Length; n++)
                    {
                        payloadString += "," + payloadFloats[n].ToString();
                    }

                    // Update data grid view
                    addRadioMessageRow(packetHeader, payloadString, csCheck);

                    // Clear buffer
                    rxBuffer.Clear();
                }

            }

        }

        delegate void addRadioMessageRowCallback(byte[] packetHeader, String payload, bool csCheck);

        private void addRadioMessageRow(byte[] packetHeader, String payload, bool csCheck)
        {

            if (radioReceivedList.InvokeRequired)
            {
                addRadioMessageRowCallback cb = new addRadioMessageRowCallback(addRadioMessageRow);
                Invoke(cb, new object[] { packetHeader, payload, csCheck });
            } else
            {
                radioReceivedList.Rows.Insert(0, DateTime.Now.ToString("h:mm:ss.fff"), packetHeader[0], packetHeader[1], packetHeader[2], packetHeader[3], payload, csCheck);
            }

        }

        private void cmdCOMPortOpenClose_Click(object sender, EventArgs e)
        {
            if (comPortConnected)
            {
                if (MessageBox.Show("Close serial port?", "COM Port", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    radio.Close();

                    txtToolStrip.Text = cmbCOMPorts.GetItemText(cmbCOMPorts.SelectedItem) + " closed.";
                    cmdCOMPortOpenClose.Text = "Open";
                    comPortConnected = false;
                }
            }
            else
            {
                radio = new SerialPort(cmbCOMPorts.GetItemText(cmbCOMPorts.SelectedItem), 115200, Parity.None, 8, StopBits.One);
                radio.DataReceived += new SerialDataReceivedEventHandler(radioDataReceived);

                rxBuffer = new List<byte>();

                radio.Open();

                if (radio.IsOpen)
                {
                    txtToolStrip.Text = cmbCOMPorts.GetItemText(cmbCOMPorts.SelectedItem) + " open.";
                    cmdCOMPortOpenClose.Text = "Close";
                    comPortConnected = true;
                }
            }
        }
    }
}
