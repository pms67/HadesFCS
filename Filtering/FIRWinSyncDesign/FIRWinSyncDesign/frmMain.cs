using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Math;

namespace FIRWinSyncDesign
{
    public partial class frmMain : Form
    {

        enum WindowType
        {
            Rectangular,
            Triangular,
            Welch,
            Sine,
            Hann,
            Hamming,
            Blackman,
            Nuttall,
            BlackmanNuttall,
            BlackmanHarris,
            FlatTop
        }

        enum FilterType
        {
            LowPass,
            HighPass,
            BandPass,
            BandStop
        }

        /* Parameters */
        double SAMPLE_TIME_S;
        double CUTOFF_FREQUENCY_HZ;
        double CUTOFF_FREQUENCY2_HZ;
        int NUM_TOTAL_SAMPLES;
        int NUM_SHIFT_SAMPLES;
        int NUM_FREQ_SAMPLES;
        WindowType WIN_TYPE;
        FilterType FILT_TYPE;

        /* Time Domain */
        double[] timeVector;
        double[] impulseResponse;
        double[] stepResponse;
        double[] window;
        double[] windowedImpulseResponse;
        double[] windowedStepResponse;

        /* Frequency Domain */
        double[] frequencyVectorHz;
        double[] impRespMag;
        double[] winRespMag;
        double[] winMag;
        
        public frmMain()
        {
            InitializeComponent();
        }

        private void frmMain_Load(object sender, EventArgs e)
        {
            /* Initial settings */
            SAMPLE_TIME_S = 0.01;
            CUTOFF_FREQUENCY_HZ = 20.0;
            CUTOFF_FREQUENCY2_HZ = 20.0;
            NUM_TOTAL_SAMPLES = 64;
            NUM_SHIFT_SAMPLES = 32;
            WIN_TYPE = WindowType.Hamming;
            FILT_TYPE = FilterType.LowPass;
            NUM_FREQ_SAMPLES = 256;

            cmbWindow.SelectedIndex = 5;
            radViewImpulse.Checked = true;
            cmbDisplay.SelectedIndex = 0;

            ComputeTimeVector();
            ComputeWindow();
            ComputeResponses();
            ComputeWindowedResponses();

            ComputeFrequencyVector();
            ComputeRespBode();
            ComputeWindowDFT();
            
            UpdateCharts();
        }

        private void UpdateCharts()
        {
            chrtFilterTimeDomain.Series[0].Points.DataBindXY(timeVector, impulseResponse);
            chrtFilterTimeDomain.Series[1].Points.DataBindXY(timeVector, windowedImpulseResponse);
            chrtFilterTimeDomain.Series[2].Points.DataBindXY(timeVector, stepResponse);
            chrtFilterTimeDomain.Series[3].Points.DataBindXY(timeVector, windowedStepResponse);
            chrtFilterTimeDomain.Series[2].Enabled = false;
            chrtFilterTimeDomain.Series[3].Enabled = false;

            chrtFilterTimeDomain.ChartAreas[0].RecalculateAxesScale();
            chrtFilterTimeDomain.Update();

            chrtWindowTimeDomain.Series[0].Points.DataBindXY(timeVector, window);
            chrtWindowTimeDomain.ChartAreas[0].RecalculateAxesScale();

            chrtFilterFrequencyDomain.Series[0].Points.DataBindXY(frequencyVectorHz, impRespMag);
            chrtFilterFrequencyDomain.Series[1].Points.DataBindXY(frequencyVectorHz, winRespMag);
            chrtFilterFrequencyDomain.ChartAreas[0].AxisX.Interval = 10.0 * (0.5 / SAMPLE_TIME_S) / ((double)NUM_FREQ_SAMPLES - 1.0);
            chrtFilterFrequencyDomain.ChartAreas[0].RecalculateAxesScale();

            chrtWindowFrequencyDomain.Series[0].Points.DataBindXY(frequencyVectorHz, winMag);
            chrtWindowFrequencyDomain.ChartAreas[0].AxisX.Interval = 10.0 * (0.5 / SAMPLE_TIME_S) / ((double)NUM_FREQ_SAMPLES - 1.0);
            chrtWindowFrequencyDomain.ChartAreas[0].RecalculateAxesScale();
        }

        /* Time Domain Functions */
        private void ComputeTimeVector()
        {
            timeVector = new double[NUM_TOTAL_SAMPLES];

            for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
            {
                timeVector[n] = n * SAMPLE_TIME_S;
            }
        }
        
        private void ComputeResponses()
        {
            impulseResponse = new double[NUM_TOTAL_SAMPLES];
            stepResponse = new double[NUM_TOTAL_SAMPLES];

            for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
            {
                if (n != NUM_SHIFT_SAMPLES)
                {
                    switch (FILT_TYPE)
                    {
                        case FilterType.LowPass:
                            impulseResponse[n] = Sin(2.0 * PI * CUTOFF_FREQUENCY_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES)) / (PI * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES));
                            break;
                        case FilterType.HighPass:                            
                            impulseResponse[n] = (Sin(PI * (n - NUM_SHIFT_SAMPLES)) - Sin(2.0 * PI * CUTOFF_FREQUENCY_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES))) / (PI * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES));
                            break;
                        case FilterType.BandPass:
                            impulseResponse[n] = (Sin(2.0 * PI * CUTOFF_FREQUENCY2_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES)) - Sin(2.0 * PI * CUTOFF_FREQUENCY_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES))) / (PI * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES));
                            break;
                        case FilterType.BandStop:
                            impulseResponse[n] = (Sin(2.0 * PI * CUTOFF_FREQUENCY_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES)) - Sin(2.0 * PI * CUTOFF_FREQUENCY2_HZ * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES)) + Sin(PI * (n - NUM_SHIFT_SAMPLES))) / (PI * SAMPLE_TIME_S * (n - NUM_SHIFT_SAMPLES));
                            break;
                    }
                } else /* Avoid divide-by-zero, limit is 2*fc */
                {
                    switch (FILT_TYPE)
                    {
                        case FilterType.LowPass:
                            impulseResponse[n] = 2.0 * CUTOFF_FREQUENCY_HZ;
                            break;
                        case FilterType.HighPass:
                            impulseResponse[n] = 1.0 / SAMPLE_TIME_S - 2.0 * CUTOFF_FREQUENCY_HZ;
                            break;
                        case FilterType.BandPass:
                            impulseResponse[n] = 2.0 * CUTOFF_FREQUENCY2_HZ - 2.0 * CUTOFF_FREQUENCY_HZ;
                            break;
                        case FilterType.BandStop:
                            impulseResponse[n] = 2.0 * CUTOFF_FREQUENCY_HZ - 2.0 * CUTOFF_FREQUENCY2_HZ + 1.0 / SAMPLE_TIME_S;
                            break;
                    }
                }
                           
            }

            /* Normalise by DC gain to achieve 0dB gain at DC and then compute step response */
            for (int n =  0; n < NUM_TOTAL_SAMPLES; n++)
            {
                impulseResponse[n] *= SAMPLE_TIME_S;

                if (n == 0)
                {
                    stepResponse[n] = 0.5 * impulseResponse[n];
                }
                else
                {
                    stepResponse[n] = stepResponse[n - 1] + 0.5 * (impulseResponse[n] + impulseResponse[n - 1]);
                }
            }

        }

        private void ComputeWindow()
        {
            window = new double[NUM_TOTAL_SAMPLES];
            
            for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
            {
                switch (WIN_TYPE)
                {
                    case WindowType.Rectangular:
                        window[n] = 1.0;
                        break;

                    case WindowType.Triangular:
                        window[n] = 1.0 - Abs((n - 0.5 * NUM_TOTAL_SAMPLES) / (0.5 * NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.Welch:
                        window[n] = 1.0 - Pow((n - 0.5 * NUM_TOTAL_SAMPLES) / (0.5 * NUM_TOTAL_SAMPLES), 2.0);
                        break;

                    case WindowType.Sine:
                        window[n] = Sin(PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.Hann:
                        window[n] = 0.5 * (1 - Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)));
                        break;

                    case WindowType.Hamming:
                        window[n] = (25.0 / 46.0) - (21.0 / 46.0) * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.Blackman:
                        window[n] = 0.42 - 0.5 * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.08 * Cos(4.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.Nuttall:
                        window[n] = 0.355768 - 0.487396 * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.144232 * Cos(4.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) - 0.012604 * Cos(6.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.BlackmanNuttall:
                        window[n] = 0.3635819 - 0.4891775 * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.1365995 * Cos(4.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) - 0.0106411 * Cos(6.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.BlackmanHarris:
                        window[n] = 0.35875 - 0.48829 * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.14128 * Cos(4.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) - 0.01168 * Cos(6.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    case WindowType.FlatTop:
                        window[n] = 0.21557895 - 0.41663158 * Cos(2.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.277263158 * Cos(4.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) - 0.083578947 * Cos(6.0 * PI * n / ((double) NUM_TOTAL_SAMPLES)) + 0.006947368 * Cos(8.0 * PI * n / ((double) NUM_TOTAL_SAMPLES));
                        break;

                    default:
                        window[n] = 1.0;
                        break;
                }
            }

        }

        private void ComputeWindowedResponses()
        {
            windowedImpulseResponse = new double[NUM_TOTAL_SAMPLES];
            windowedStepResponse = new double[NUM_TOTAL_SAMPLES];

            
            for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
            {
                windowedImpulseResponse[n] = impulseResponse[n] * window[n];


                if (n == 0)
                {
                    windowedStepResponse[n] = 0.5 * windowedStepResponse[n];
                }
                else
                {
                    windowedStepResponse[n] = windowedStepResponse[n - 1] + 0.5 * (windowedImpulseResponse[n] + windowedImpulseResponse[n - 1]);
                }
            }

        }

        /* Frequency Domain Functions */
        private void ComputeFrequencyVector()
        {
            frequencyVectorHz = new double[NUM_FREQ_SAMPLES];

            double df = (0.5 / SAMPLE_TIME_S) / ((double) NUM_FREQ_SAMPLES - 1.0);

            for (int n = 0; n < NUM_FREQ_SAMPLES; n++)
            {
                frequencyVectorHz[n] = n * df;
            }
        }


        private void ComputeRespBode()
        {
            impRespMag   = new double[NUM_FREQ_SAMPLES];
            winRespMag   = new double[NUM_FREQ_SAMPLES];

            for (int fIndex = 0; fIndex < NUM_FREQ_SAMPLES; fIndex++)
            {
                double re = 0.0;
                double im = 0.0;
                double reWin = 0.0;
                double imWin = 0.0;

                for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
                {
                    re = re + impulseResponse[n] * Cos(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                    im = im - impulseResponse[n] * Sin(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                    reWin = reWin + windowedImpulseResponse[n] * Cos(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                    imWin = imWin - windowedImpulseResponse[n] * Sin(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                }

                impRespMag[fIndex] = 10.0 * Log10(re * re + im * im);
                winRespMag[fIndex] = 10.0 * Log10(reWin * reWin + imWin * imWin);
            }
        }

        private double GetGainAtCutOff()
        {
            double re = 0.0;
            double im = 0.0;

            for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
            {
                re = re + impulseResponse[n] * Cos(2.0 * PI * CUTOFF_FREQUENCY_HZ * n * SAMPLE_TIME_S);
                im = im - impulseResponse[n] * Sin(2.0 * PI * CUTOFF_FREQUENCY_HZ * n * SAMPLE_TIME_S);
            }

            return (10.0 * Log10(re * re + im * im));
        }

        private void ComputeWindowDFT()
        {
            winMag = new double[NUM_FREQ_SAMPLES];

            for (int fIndex = 0; fIndex < NUM_FREQ_SAMPLES; fIndex++)
            {
                double re = 0.0;
                double im = 0.0;

                for (int n = 0; n < NUM_TOTAL_SAMPLES; n++)
                {
                    re = re + window[n] * Cos(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                    im = im - window[n] * Sin(2.0 * PI * frequencyVectorHz[fIndex] * n * SAMPLE_TIME_S);
                }

                winMag[fIndex] = 10.0 * Log10(re * re + im * im);
            }
        }

        private void UpdatePlotSettings()
        {
            if (radViewImpulse.Checked)
            {
                if (cmbDisplay.SelectedIndex == 0)
                {
                    chrtFilterTimeDomain.Series[0].Enabled = true;
                    chrtFilterTimeDomain.Series[1].Enabled = true;
                }
                else if (cmbDisplay.SelectedIndex == 1)
                {
                    chrtFilterTimeDomain.Series[0].Enabled = true;
                    chrtFilterTimeDomain.Series[1].Enabled = false;
                }
                else if (cmbDisplay.SelectedIndex == 2)
                {
                    chrtFilterTimeDomain.Series[0].Enabled = false;
                    chrtFilterTimeDomain.Series[1].Enabled = true;
                }

                chrtFilterTimeDomain.Series[2].Enabled = false;
                chrtFilterTimeDomain.Series[3].Enabled = false;

            }
            else
            {
                if (cmbDisplay.SelectedIndex == 0)
                {
                    chrtFilterTimeDomain.Series[2].Enabled = true;
                    chrtFilterTimeDomain.Series[3].Enabled = true;
                }
                else if (cmbDisplay.SelectedIndex == 1)
                {
                    chrtFilterTimeDomain.Series[2].Enabled = true;
                    chrtFilterTimeDomain.Series[3].Enabled = false;
                }
                else if (cmbDisplay.SelectedIndex == 2)
                {
                    chrtFilterTimeDomain.Series[2].Enabled = false;
                    chrtFilterTimeDomain.Series[3].Enabled = true;
                }

                chrtFilterTimeDomain.Series[0].Enabled = false;
                chrtFilterTimeDomain.Series[1].Enabled = false;
            }

            if (cmbDisplay.SelectedIndex == 0)
            {
                chrtFilterFrequencyDomain.Series[0].Enabled = true;
                chrtFilterFrequencyDomain.Series[1].Enabled = true;
            }
            else if (cmbDisplay.SelectedIndex == 1)
            {
                chrtFilterFrequencyDomain.Series[0].Enabled = true;
                chrtFilterFrequencyDomain.Series[1].Enabled = false;
            }
            else if (cmbDisplay.SelectedIndex == 2)
            {
                chrtFilterFrequencyDomain.Series[0].Enabled = false;
                chrtFilterFrequencyDomain.Series[1].Enabled = true;
            }

            chrtFilterTimeDomain.ChartAreas[0].RecalculateAxesScale();
        }

        private void radViewImpulse_CheckedChanged(object sender, EventArgs e)
        {
            UpdatePlotSettings();
        }

        private void cmbDisplay_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdatePlotSettings();
        }

        private void btnDesignFilter_Click(object sender, EventArgs e)
        {
            SAMPLE_TIME_S = 1.0 / Convert.ToDouble(txtSamplingFrequency.Text);
            NUM_TOTAL_SAMPLES = Convert.ToInt32(txtFilterLength.Text);
            NUM_SHIFT_SAMPLES = Convert.ToInt32(txtShiftSamples.Text);
            WIN_TYPE = (WindowType) Convert.ToInt32(cmbWindow.SelectedIndex);

            if (radLP.Checked)
            {
                FILT_TYPE = FilterType.LowPass;
                CUTOFF_FREQUENCY_HZ = Convert.ToDouble(txtCutOffFrequencyHigh.Text);
            }
            else if (radHP.Checked)
            {
                FILT_TYPE = FilterType.HighPass;
                CUTOFF_FREQUENCY_HZ = Convert.ToDouble(txtCutOffFrequency.Text);
            }
            else if (radBP.Checked)
            {
                FILT_TYPE = FilterType.BandPass;
                CUTOFF_FREQUENCY_HZ = Convert.ToDouble(txtCutOffFrequency.Text);
                CUTOFF_FREQUENCY2_HZ = Convert.ToDouble(txtCutOffFrequencyHigh.Text);
            }
            else if (radBS.Checked)
            {
                FILT_TYPE = FilterType.BandStop;
                CUTOFF_FREQUENCY_HZ = Convert.ToDouble(txtCutOffFrequency.Text);
                CUTOFF_FREQUENCY2_HZ = Convert.ToDouble(txtCutOffFrequencyHigh.Text);
            }

            if (SAMPLE_TIME_S < 0.0)
            {
                MessageBox.Show("Sampling frequency cannot be negative.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CUTOFF_FREQUENCY_HZ >= 0.5 / SAMPLE_TIME_S || CUTOFF_FREQUENCY2_HZ >= 0.5 / SAMPLE_TIME_S)
            {
                MessageBox.Show("Cut-off frequency has to be less than the Nyquist frequency (i.e. sampling frequency / 2).", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
           
            if (NUM_TOTAL_SAMPLES < 0 || NUM_SHIFT_SAMPLES < 0)
            {
                MessageBox.Show("Total number of samples and sample shift number both need to be integers, greater than zero.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            ComputeTimeVector();
            ComputeResponses();
            ComputeWindow();
            ComputeWindowedResponses();

            ComputeFrequencyVector();
            ComputeRespBode();
            ComputeWindowDFT();
            
            UpdateCharts();
            UpdatePlotSettings();
        }

        private void cmbWindow_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cmbWindow.SelectedIndex >= 0)
            {
                WIN_TYPE = (WindowType)Convert.ToInt32(cmbWindow.SelectedIndex);

                ComputeFrequencyVector();
                ComputeWindow();
                ComputeWindowDFT();

                chrtWindowTimeDomain.Series[0].Points.DataBindXY(timeVector, window);

                chrtWindowFrequencyDomain.Series[0].Points.DataBindXY(frequencyVectorHz, winMag);

                chrtWindowTimeDomain.ChartAreas[0].RecalculateAxesScale();
                chrtWindowFrequencyDomain.ChartAreas[0].RecalculateAxesScale();
            }
        }

        private void infoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("FIR Filter Designer\nWritten by Philip M. Salmony\n29 November 2019\nphilsal.co.uk", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void FilterTypeChange()
        {
            if (radLP.Checked)
            {
                txtCutOffFrequency.Enabled = false;
                txtCutOffFrequencyHigh.Enabled = true;
            } else if (radHP.Checked)
            {
                txtCutOffFrequency.Enabled = true;
                txtCutOffFrequencyHigh.Enabled = false;
            } else if (radBP.Checked)
            {
                txtCutOffFrequency.Enabled = true;
                txtCutOffFrequencyHigh.Enabled = true;
            } else if (radBS.Checked)
            {
                txtCutOffFrequency.Enabled = true;
                txtCutOffFrequencyHigh.Enabled = true;
            }
        }

        private void radLP_CheckedChanged(object sender, EventArgs e)
        {
            FilterTypeChange();
        }

        private void radHP_CheckedChanged(object sender, EventArgs e)
        {
            FilterTypeChange();
        }

        private void radBP_CheckedChanged(object sender, EventArgs e)
        {
            FilterTypeChange();
        }

        private void radBS_CheckedChanged(object sender, EventArgs e)
        {
            FilterTypeChange();
        }

        private void exportCoefficientsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog.Filter = "Text File|*.txt";
            saveFileDialog.Title  = "Export Filter Coefficients";
            saveFileDialog.ShowDialog();

            if (saveFileDialog.FileName != "")
            {
                string[] data = new string[3];
                data[0] = "Filter Order: " + NUM_TOTAL_SAMPLES + " Sampling Frequency (Hz): " + (1.0 / SAMPLE_TIME_S).ToString("F6") + " Cut-Off Frequency Lo (Hz): " + CUTOFF_FREQUENCY_HZ.ToString("F6") + " Cut-Off Frequency Hi (Hz): " + CUTOFF_FREQUENCY2_HZ.ToString("F6") + "\n\n";

                data[1] = windowedImpulseResponse[0].ToString("F7");
                data[2] = "float coeff[] = {" + windowedImpulseResponse[0].ToString("F7") + "f";
                for (int n = 1; n < NUM_TOTAL_SAMPLES; n++)
                {
                    data[1] += "," + windowedImpulseResponse[n].ToString("F9");
                    data[2] += "," + windowedImpulseResponse[n].ToString("F7") + "f";
                }
                data[1] += "\n\n";
                data[2] += "};";
                
                System.IO.File.WriteAllLines(saveFileDialog.FileName, data);
                MessageBox.Show("Coefficients written to file!", "Export Coefficients", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void exportTimeDomainDataToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog.Filter = "Text File|*.txt";
            saveFileDialog.Title  = "Export Time Domain Data";
            saveFileDialog.ShowDialog();

            if (saveFileDialog.FileName != "")
            {
                string[] data = new string[4];
                data[0] = "[TIME DOMAIN DATA (TIME/IMPULSE/STEP)] Filter Order: " + NUM_TOTAL_SAMPLES + " Sampling Frequency (Hz): " + (1.0 / SAMPLE_TIME_S).ToString("F6") + " Cut-Off Frequency Lo (Hz): " + CUTOFF_FREQUENCY_HZ.ToString("F6") + " Cut-Off Frequency Hi (Hz): " + CUTOFF_FREQUENCY2_HZ.ToString("F6") + "\n\n";

                data[1] = timeVector[0].ToString("F6");
                data[2] = windowedImpulseResponse[0].ToString("F9");
                data[3] = windowedStepResponse[0].ToString("F9");
                for (int n = 1; n < NUM_TOTAL_SAMPLES; n++)
                {
                    data[1] += "," + timeVector[n].ToString("F6");
                    data[2] += "," + windowedImpulseResponse[n].ToString("F9");
                    data[3] += "," + windowedStepResponse[n].ToString("F9") ;
                }

                data[1] += "\n\n";
                data[2] += "\n\n";

                System.IO.File.WriteAllLines(saveFileDialog.FileName, data);
                MessageBox.Show("Data written to file!", "Export Time Domain Data", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void exportFrequencyDomainDataToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog.Filter = "Text File|*.txt";
            saveFileDialog.Title  = "Export Frequency Domain Data";
            saveFileDialog.ShowDialog();

            if (saveFileDialog.FileName != "")
            {
                string[] data = new string[4];
                data[0] = "[FREQUENCY DOMAIN DATA (FREQ/RAW/WINDOWED)] Filter Order: " + NUM_TOTAL_SAMPLES + " Sampling Frequency (Hz): " + (1.0 / SAMPLE_TIME_S).ToString("F6") + " Cut-Off Frequency Lo (Hz): " + CUTOFF_FREQUENCY_HZ.ToString("F6") + " Cut-Off Frequency Hi (Hz): " + CUTOFF_FREQUENCY2_HZ.ToString("F6") + "\n\n";

                data[1] = frequencyVectorHz[0].ToString("F6");
                data[2] = impRespMag[0].ToString("F9");
                data[3] = winRespMag[0].ToString("F9");
                for (int n = 1; n < NUM_FREQ_SAMPLES; n++)
                {
                    data[1] += "," + frequencyVectorHz[n].ToString("F6");
                    data[2] += "," + impRespMag[n].ToString("F9");
                    data[3] += "," + winRespMag[n].ToString("F9");
                }
                
                data[1] += "\n\n";
                data[2] += "\n\n";

                System.IO.File.WriteAllLines(saveFileDialog.FileName, data);
                MessageBox.Show("Data written to file!", "Export Frequency Domain Data", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
    }
} 