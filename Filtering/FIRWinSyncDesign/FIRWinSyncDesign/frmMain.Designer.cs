namespace FIRWinSyncDesign
{
    partial class frmMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series3 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series4 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea2 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series5 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea3 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series6 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series7 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea4 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series8 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmMain));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.chrtFilterTimeDomain = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.chrtWindowTimeDomain = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.chrtFilterFrequencyDomain = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.chrtWindowFrequencyDomain = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.radBS = new System.Windows.Forms.RadioButton();
            this.radBP = new System.Windows.Forms.RadioButton();
            this.radHP = new System.Windows.Forms.RadioButton();
            this.radLP = new System.Windows.Forms.RadioButton();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.txtCutOffFrequencyHigh = new System.Windows.Forms.TextBox();
            this.btnDesignFilter = new System.Windows.Forms.Button();
            this.cmbDisplay = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.radViewStep = new System.Windows.Forms.RadioButton();
            this.radViewImpulse = new System.Windows.Forms.RadioButton();
            this.label6 = new System.Windows.Forms.Label();
            this.cmbWindow = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtCutOffFrequency = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtShiftSamples = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtFilterLength = new System.Windows.Forms.TextBox();
            this.txtSamplingFrequency = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.mnuMain = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportCoefficientsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportTimeDomainDataToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportFrequencyDomainDataToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.infoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chrtFilterTimeDomain)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chrtWindowTimeDomain)).BeginInit();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chrtFilterFrequencyDomain)).BeginInit();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chrtWindowFrequencyDomain)).BeginInit();
            this.groupBox5.SuspendLayout();
            this.panel1.SuspendLayout();
            this.mnuMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.chrtFilterTimeDomain);
            this.groupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox1.Location = new System.Drawing.Point(12, 34);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(480, 300);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Filter Respones in Time Domain";
            // 
            // chrtFilterTimeDomain
            // 
            this.chrtFilterTimeDomain.BackColor = System.Drawing.SystemColors.Control;
            chartArea1.AxisX.Minimum = 0D;
            chartArea1.AxisX.Title = "Time (s)";
            chartArea1.BackColor = System.Drawing.Color.White;
            chartArea1.Name = "ChartArea1";
            this.chrtFilterTimeDomain.ChartAreas.Add(chartArea1);
            this.chrtFilterTimeDomain.Location = new System.Drawing.Point(6, 19);
            this.chrtFilterTimeDomain.Name = "chrtFilterTimeDomain";
            series1.BorderDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Dot;
            series1.BorderWidth = 2;
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series1.Color = System.Drawing.Color.DimGray;
            series1.Name = "IR";
            series2.BorderWidth = 2;
            series2.ChartArea = "ChartArea1";
            series2.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series2.Color = System.Drawing.Color.Black;
            series2.Name = "IR (Win)";
            series3.BorderDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Dot;
            series3.BorderWidth = 2;
            series3.ChartArea = "ChartArea1";
            series3.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series3.Color = System.Drawing.Color.DimGray;
            series3.Name = "SR";
            series4.BorderWidth = 2;
            series4.ChartArea = "ChartArea1";
            series4.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series4.Color = System.Drawing.Color.Black;
            series4.Name = "SR (Win)";
            this.chrtFilterTimeDomain.Series.Add(series1);
            this.chrtFilterTimeDomain.Series.Add(series2);
            this.chrtFilterTimeDomain.Series.Add(series3);
            this.chrtFilterTimeDomain.Series.Add(series4);
            this.chrtFilterTimeDomain.Size = new System.Drawing.Size(468, 275);
            this.chrtFilterTimeDomain.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.chrtWindowTimeDomain);
            this.groupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox2.Location = new System.Drawing.Point(516, 34);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(480, 300);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Window Function in Time Domain";
            // 
            // chrtWindowTimeDomain
            // 
            this.chrtWindowTimeDomain.BackColor = System.Drawing.SystemColors.Control;
            chartArea2.AxisX.Minimum = 0D;
            chartArea2.AxisX.Title = "Time (s)";
            chartArea2.BackColor = System.Drawing.Color.White;
            chartArea2.Name = "ChartArea1";
            this.chrtWindowTimeDomain.ChartAreas.Add(chartArea2);
            this.chrtWindowTimeDomain.Location = new System.Drawing.Point(6, 19);
            this.chrtWindowTimeDomain.Name = "chrtWindowTimeDomain";
            series5.BorderWidth = 2;
            series5.ChartArea = "ChartArea1";
            series5.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series5.Color = System.Drawing.Color.Black;
            series5.IsVisibleInLegend = false;
            series5.Name = "Window";
            this.chrtWindowTimeDomain.Series.Add(series5);
            this.chrtWindowTimeDomain.Size = new System.Drawing.Size(468, 275);
            this.chrtWindowTimeDomain.TabIndex = 1;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.chrtFilterFrequencyDomain);
            this.groupBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox3.Location = new System.Drawing.Point(12, 340);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(480, 300);
            this.groupBox3.TabIndex = 1;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Filter Frequency Response";
            // 
            // chrtFilterFrequencyDomain
            // 
            this.chrtFilterFrequencyDomain.BackColor = System.Drawing.SystemColors.Control;
            chartArea3.AxisX.LabelStyle.Format = "#";
            chartArea3.AxisX.Minimum = 0D;
            chartArea3.AxisX.Title = "Frequency (Hz)";
            chartArea3.BackColor = System.Drawing.Color.White;
            chartArea3.Name = "ChartArea1";
            this.chrtFilterFrequencyDomain.ChartAreas.Add(chartArea3);
            this.chrtFilterFrequencyDomain.Location = new System.Drawing.Point(6, 19);
            this.chrtFilterFrequencyDomain.Name = "chrtFilterFrequencyDomain";
            series6.BorderDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Dot;
            series6.BorderWidth = 2;
            series6.ChartArea = "ChartArea1";
            series6.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series6.Color = System.Drawing.Color.DimGray;
            series6.IsVisibleInLegend = false;
            series6.Name = "Magnitude";
            series7.BorderWidth = 2;
            series7.ChartArea = "ChartArea1";
            series7.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series7.Color = System.Drawing.Color.Black;
            series7.Name = "MagnitudeWin";
            this.chrtFilterFrequencyDomain.Series.Add(series6);
            this.chrtFilterFrequencyDomain.Series.Add(series7);
            this.chrtFilterFrequencyDomain.Size = new System.Drawing.Size(468, 266);
            this.chrtFilterFrequencyDomain.TabIndex = 2;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.chrtWindowFrequencyDomain);
            this.groupBox4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox4.Location = new System.Drawing.Point(516, 340);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(480, 300);
            this.groupBox4.TabIndex = 2;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Window Function Frequency Response";
            // 
            // chrtWindowFrequencyDomain
            // 
            this.chrtWindowFrequencyDomain.BackColor = System.Drawing.SystemColors.Control;
            chartArea4.AxisX.LabelStyle.Format = "#";
            chartArea4.AxisX.Minimum = 0D;
            chartArea4.AxisX.Title = "Frequency (Hz)";
            chartArea4.BackColor = System.Drawing.Color.White;
            chartArea4.Name = "ChartArea1";
            this.chrtWindowFrequencyDomain.ChartAreas.Add(chartArea4);
            this.chrtWindowFrequencyDomain.Location = new System.Drawing.Point(6, 19);
            this.chrtWindowFrequencyDomain.Name = "chrtWindowFrequencyDomain";
            series8.BorderWidth = 2;
            series8.ChartArea = "ChartArea1";
            series8.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
            series8.Color = System.Drawing.Color.Black;
            series8.IsVisibleInLegend = false;
            series8.Name = "Magnitude";
            this.chrtWindowFrequencyDomain.Series.Add(series8);
            this.chrtWindowFrequencyDomain.Size = new System.Drawing.Size(468, 266);
            this.chrtWindowFrequencyDomain.TabIndex = 4;
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.panel1);
            this.groupBox5.Controls.Add(this.label9);
            this.groupBox5.Controls.Add(this.label8);
            this.groupBox5.Controls.Add(this.txtCutOffFrequencyHigh);
            this.groupBox5.Controls.Add(this.btnDesignFilter);
            this.groupBox5.Controls.Add(this.cmbDisplay);
            this.groupBox5.Controls.Add(this.label7);
            this.groupBox5.Controls.Add(this.radViewStep);
            this.groupBox5.Controls.Add(this.radViewImpulse);
            this.groupBox5.Controls.Add(this.label6);
            this.groupBox5.Controls.Add(this.cmbWindow);
            this.groupBox5.Controls.Add(this.label5);
            this.groupBox5.Controls.Add(this.txtCutOffFrequency);
            this.groupBox5.Controls.Add(this.label4);
            this.groupBox5.Controls.Add(this.txtShiftSamples);
            this.groupBox5.Controls.Add(this.label3);
            this.groupBox5.Controls.Add(this.txtFilterLength);
            this.groupBox5.Controls.Add(this.txtSamplingFrequency);
            this.groupBox5.Controls.Add(this.label2);
            this.groupBox5.Controls.Add(this.label1);
            this.groupBox5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox5.Location = new System.Drawing.Point(12, 646);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(984, 93);
            this.groupBox5.TabIndex = 3;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Parameters";
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.radBS);
            this.panel1.Controls.Add(this.radBP);
            this.panel1.Controls.Add(this.radHP);
            this.panel1.Controls.Add(this.radLP);
            this.panel1.Location = new System.Drawing.Point(566, 13);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(93, 58);
            this.panel1.TabIndex = 19;
            // 
            // radBS
            // 
            this.radBS.AutoSize = true;
            this.radBS.Location = new System.Drawing.Point(49, 26);
            this.radBS.Name = "radBS";
            this.radBS.Size = new System.Drawing.Size(41, 17);
            this.radBS.TabIndex = 26;
            this.radBS.Text = "BS";
            this.radBS.UseVisualStyleBackColor = true;
            this.radBS.CheckedChanged += new System.EventHandler(this.radBS_CheckedChanged);
            // 
            // radBP
            // 
            this.radBP.AutoSize = true;
            this.radBP.Location = new System.Drawing.Point(3, 26);
            this.radBP.Name = "radBP";
            this.radBP.Size = new System.Drawing.Size(41, 17);
            this.radBP.TabIndex = 25;
            this.radBP.Text = "BP";
            this.radBP.UseVisualStyleBackColor = true;
            this.radBP.CheckedChanged += new System.EventHandler(this.radBP_CheckedChanged);
            // 
            // radHP
            // 
            this.radHP.AutoSize = true;
            this.radHP.Location = new System.Drawing.Point(49, 3);
            this.radHP.Name = "radHP";
            this.radHP.Size = new System.Drawing.Size(42, 17);
            this.radHP.TabIndex = 24;
            this.radHP.Text = "HP";
            this.radHP.UseVisualStyleBackColor = true;
            this.radHP.CheckedChanged += new System.EventHandler(this.radHP_CheckedChanged);
            // 
            // radLP
            // 
            this.radLP.AutoSize = true;
            this.radLP.Checked = true;
            this.radLP.Location = new System.Drawing.Point(3, 3);
            this.radLP.Name = "radLP";
            this.radLP.Size = new System.Drawing.Size(40, 17);
            this.radLP.TabIndex = 23;
            this.radLP.TabStop = true;
            this.radLP.Text = "LP";
            this.radLP.UseVisualStyleBackColor = true;
            this.radLP.CheckedChanged += new System.EventHandler(this.radLP_CheckedChanged);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(501, 19);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(59, 13);
            this.label9.TabIndex = 18;
            this.label9.Text = "Filter Type:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(252, 45);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(107, 13);
            this.label8.TabIndex = 17;
            this.label8.Text = "High Frequency (Hz):";
            // 
            // txtCutOffFrequencyHigh
            // 
            this.txtCutOffFrequencyHigh.Location = new System.Drawing.Point(376, 42);
            this.txtCutOffFrequencyHigh.Name = "txtCutOffFrequencyHigh";
            this.txtCutOffFrequencyHigh.Size = new System.Drawing.Size(104, 20);
            this.txtCutOffFrequencyHigh.TabIndex = 16;
            this.txtCutOffFrequencyHigh.Text = "20";
            // 
            // btnDesignFilter
            // 
            this.btnDesignFilter.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnDesignFilter.Location = new System.Drawing.Point(853, 16);
            this.btnDesignFilter.Name = "btnDesignFilter";
            this.btnDesignFilter.Size = new System.Drawing.Size(125, 68);
            this.btnDesignFilter.TabIndex = 15;
            this.btnDesignFilter.Text = "Design Filter";
            this.btnDesignFilter.UseVisualStyleBackColor = true;
            this.btnDesignFilter.Click += new System.EventHandler(this.btnDesignFilter_Click);
            // 
            // cmbDisplay
            // 
            this.cmbDisplay.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDisplay.FormattingEnabled = true;
            this.cmbDisplay.Items.AddRange(new object[] {
            "Both",
            "Raw Only",
            "Win Only"});
            this.cmbDisplay.Location = new System.Drawing.Point(747, 66);
            this.cmbDisplay.Name = "cmbDisplay";
            this.cmbDisplay.Size = new System.Drawing.Size(92, 21);
            this.cmbDisplay.TabIndex = 14;
            this.cmbDisplay.SelectedIndexChanged += new System.EventHandler(this.cmbDisplay_SelectedIndexChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(662, 69);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(44, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Display:";
            // 
            // radViewStep
            // 
            this.radViewStep.AutoSize = true;
            this.radViewStep.Location = new System.Drawing.Point(747, 41);
            this.radViewStep.Name = "radViewStep";
            this.radViewStep.Size = new System.Drawing.Size(51, 17);
            this.radViewStep.TabIndex = 12;
            this.radViewStep.Text = "Step";
            this.radViewStep.UseVisualStyleBackColor = true;
            // 
            // radViewImpulse
            // 
            this.radViewImpulse.AutoSize = true;
            this.radViewImpulse.Checked = true;
            this.radViewImpulse.Location = new System.Drawing.Point(747, 16);
            this.radViewImpulse.Name = "radViewImpulse";
            this.radViewImpulse.Size = new System.Drawing.Size(68, 17);
            this.radViewImpulse.TabIndex = 11;
            this.radViewImpulse.TabStop = true;
            this.radViewImpulse.Text = "Impulse";
            this.radViewImpulse.UseVisualStyleBackColor = true;
            this.radViewImpulse.CheckedChanged += new System.EventHandler(this.radViewImpulse_CheckedChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(662, 18);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(79, 13);
            this.label6.TabIndex = 10;
            this.label6.Text = "Response Plot:";
            // 
            // cmbWindow
            // 
            this.cmbWindow.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbWindow.FormattingEnabled = true;
            this.cmbWindow.Items.AddRange(new object[] {
            "Rectangular",
            "Triangular",
            "Welch",
            "Sine",
            "Hann",
            "Hamming",
            "Blackman",
            "Nuttall",
            "BlackmanNuttall",
            "BlackmanHarris",
            "FlatTop"});
            this.cmbWindow.Location = new System.Drawing.Point(376, 68);
            this.cmbWindow.Name = "cmbWindow";
            this.cmbWindow.Size = new System.Drawing.Size(104, 21);
            this.cmbWindow.TabIndex = 9;
            this.cmbWindow.SelectedIndexChanged += new System.EventHandler(this.cmbWindow_SelectedIndexChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(252, 71);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(93, 13);
            this.label5.TabIndex = 8;
            this.label5.Text = "Window Function:";
            // 
            // txtCutOffFrequency
            // 
            this.txtCutOffFrequency.Enabled = false;
            this.txtCutOffFrequency.Location = new System.Drawing.Point(376, 16);
            this.txtCutOffFrequency.Name = "txtCutOffFrequency";
            this.txtCutOffFrequency.Size = new System.Drawing.Size(104, 20);
            this.txtCutOffFrequency.TabIndex = 7;
            this.txtCutOffFrequency.Text = "10";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(252, 19);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(105, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "Low Frequency (Hz):";
            // 
            // txtShiftSamples
            // 
            this.txtShiftSamples.Location = new System.Drawing.Point(140, 68);
            this.txtShiftSamples.Name = "txtShiftSamples";
            this.txtShiftSamples.Size = new System.Drawing.Size(104, 20);
            this.txtShiftSamples.TabIndex = 5;
            this.txtShiftSamples.Text = "32";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(6, 71);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(74, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Shift Samples:";
            // 
            // txtFilterLength
            // 
            this.txtFilterLength.Location = new System.Drawing.Point(140, 42);
            this.txtFilterLength.Name = "txtFilterLength";
            this.txtFilterLength.Size = new System.Drawing.Size(104, 20);
            this.txtFilterLength.TabIndex = 3;
            this.txtFilterLength.Text = "64";
            // 
            // txtSamplingFrequency
            // 
            this.txtSamplingFrequency.Location = new System.Drawing.Point(140, 16);
            this.txtSamplingFrequency.Name = "txtSamplingFrequency";
            this.txtSamplingFrequency.Size = new System.Drawing.Size(104, 20);
            this.txtSamplingFrequency.TabIndex = 2;
            this.txtSamplingFrequency.Text = "100";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(6, 45);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(68, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Filter Length:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(6, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(128, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Sampling Frequency (Hz):";
            // 
            // mnuMain
            // 
            this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.infoToolStripMenuItem});
            this.mnuMain.Location = new System.Drawing.Point(0, 0);
            this.mnuMain.Name = "mnuMain";
            this.mnuMain.Size = new System.Drawing.Size(1008, 24);
            this.mnuMain.TabIndex = 4;
            this.mnuMain.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exportCoefficientsToolStripMenuItem,
            this.exportTimeDomainDataToolStripMenuItem,
            this.exportFrequencyDomainDataToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // exportCoefficientsToolStripMenuItem
            // 
            this.exportCoefficientsToolStripMenuItem.Name = "exportCoefficientsToolStripMenuItem";
            this.exportCoefficientsToolStripMenuItem.Size = new System.Drawing.Size(238, 22);
            this.exportCoefficientsToolStripMenuItem.Text = "Export Filter Coefficients";
            this.exportCoefficientsToolStripMenuItem.Click += new System.EventHandler(this.exportCoefficientsToolStripMenuItem_Click);
            // 
            // exportTimeDomainDataToolStripMenuItem
            // 
            this.exportTimeDomainDataToolStripMenuItem.Name = "exportTimeDomainDataToolStripMenuItem";
            this.exportTimeDomainDataToolStripMenuItem.Size = new System.Drawing.Size(238, 22);
            this.exportTimeDomainDataToolStripMenuItem.Text = "Export Time Domain Data";
            this.exportTimeDomainDataToolStripMenuItem.Click += new System.EventHandler(this.exportTimeDomainDataToolStripMenuItem_Click);
            // 
            // exportFrequencyDomainDataToolStripMenuItem
            // 
            this.exportFrequencyDomainDataToolStripMenuItem.Name = "exportFrequencyDomainDataToolStripMenuItem";
            this.exportFrequencyDomainDataToolStripMenuItem.Size = new System.Drawing.Size(238, 22);
            this.exportFrequencyDomainDataToolStripMenuItem.Text = "Export Frequency Domain Data";
            this.exportFrequencyDomainDataToolStripMenuItem.Click += new System.EventHandler(this.exportFrequencyDomainDataToolStripMenuItem_Click);
            // 
            // infoToolStripMenuItem
            // 
            this.infoToolStripMenuItem.Name = "infoToolStripMenuItem";
            this.infoToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
            this.infoToolStripMenuItem.Text = "&Info";
            this.infoToolStripMenuItem.Click += new System.EventHandler(this.infoToolStripMenuItem_Click);
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1008, 751);
            this.Controls.Add(this.groupBox5);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.mnuMain);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.mnuMain;
            this.MaximizeBox = false;
            this.Name = "frmMain";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "FIR Filter Designer [Philip Salmony @ philsal.co.uk]";
            this.Load += new System.EventHandler(this.frmMain_Load);
            this.groupBox1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chrtFilterTimeDomain)).EndInit();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chrtWindowTimeDomain)).EndInit();
            this.groupBox3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chrtFilterFrequencyDomain)).EndInit();
            this.groupBox4.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chrtWindowFrequencyDomain)).EndInit();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.mnuMain.ResumeLayout(false);
            this.mnuMain.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.DataVisualization.Charting.Chart chrtFilterTimeDomain;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.DataVisualization.Charting.Chart chrtWindowTimeDomain;
        private System.Windows.Forms.DataVisualization.Charting.Chart chrtFilterFrequencyDomain;
        private System.Windows.Forms.DataVisualization.Charting.Chart chrtWindowFrequencyDomain;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.TextBox txtShiftSamples;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtFilterLength;
        private System.Windows.Forms.TextBox txtSamplingFrequency;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtCutOffFrequency;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbWindow;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.RadioButton radViewStep;
        private System.Windows.Forms.RadioButton radViewImpulse;
        private System.Windows.Forms.ComboBox cmbDisplay;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btnDesignFilter;
        private System.Windows.Forms.MenuStrip mnuMain;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportCoefficientsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportTimeDomainDataToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportFrequencyDomainDataToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem infoToolStripMenuItem;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox txtCutOffFrequencyHigh;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.RadioButton radBS;
        private System.Windows.Forms.RadioButton radBP;
        private System.Windows.Forms.RadioButton radHP;
        private System.Windows.Forms.RadioButton radLP;
    }
}

