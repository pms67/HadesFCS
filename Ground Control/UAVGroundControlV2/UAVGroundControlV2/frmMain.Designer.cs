namespace UAVGroundControlV2
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmMain));
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tbCommands = new System.Windows.Forms.TabPage();
            this.tbWaypoints = new System.Windows.Forms.TabPage();
            this.tbRadioLog = new System.Windows.Forms.TabPage();
            this.radioReceivedList = new System.Windows.Forms.DataGridView();
            this.clTime = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clSequence = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clIDA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clIDB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clPayloadLength = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clPayload = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.clChecksum = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.tbRadioSetup = new System.Windows.Forms.TabPage();
            this.cmdCOMPortOpenClose = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.cmbCOMPorts = new System.Windows.Forms.ComboBox();
            this.tbLogging = new System.Windows.Forms.TabPage();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.txtToolStrip = new System.Windows.Forms.ToolStripStatusLabel();
            this.tabControl1.SuspendLayout();
            this.tbRadioLog.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.radioReceivedList)).BeginInit();
            this.tbRadioSetup.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tbCommands);
            this.tabControl1.Controls.Add(this.tbWaypoints);
            this.tabControl1.Controls.Add(this.tbRadioLog);
            this.tabControl1.Controls.Add(this.tbRadioSetup);
            this.tabControl1.Controls.Add(this.tbLogging);
            this.tabControl1.Location = new System.Drawing.Point(12, 455);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(984, 237);
            this.tabControl1.TabIndex = 1;
            // 
            // tbCommands
            // 
            this.tbCommands.Location = new System.Drawing.Point(4, 22);
            this.tbCommands.Name = "tbCommands";
            this.tbCommands.Padding = new System.Windows.Forms.Padding(3);
            this.tbCommands.Size = new System.Drawing.Size(976, 211);
            this.tbCommands.TabIndex = 1;
            this.tbCommands.Text = "Commands";
            this.tbCommands.UseVisualStyleBackColor = true;
            // 
            // tbWaypoints
            // 
            this.tbWaypoints.Location = new System.Drawing.Point(4, 22);
            this.tbWaypoints.Name = "tbWaypoints";
            this.tbWaypoints.Size = new System.Drawing.Size(976, 211);
            this.tbWaypoints.TabIndex = 4;
            this.tbWaypoints.Text = "Waypoints";
            this.tbWaypoints.UseVisualStyleBackColor = true;
            // 
            // tbRadioLog
            // 
            this.tbRadioLog.Controls.Add(this.radioReceivedList);
            this.tbRadioLog.Location = new System.Drawing.Point(4, 22);
            this.tbRadioLog.Name = "tbRadioLog";
            this.tbRadioLog.Padding = new System.Windows.Forms.Padding(3);
            this.tbRadioLog.Size = new System.Drawing.Size(976, 211);
            this.tbRadioLog.TabIndex = 0;
            this.tbRadioLog.Text = "Radio Log";
            this.tbRadioLog.UseVisualStyleBackColor = true;
            // 
            // radioReceivedList
            // 
            this.radioReceivedList.AllowUserToAddRows = false;
            this.radioReceivedList.AllowUserToDeleteRows = false;
            this.radioReceivedList.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.ColumnHeader;
            this.radioReceivedList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.radioReceivedList.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.clTime,
            this.clSequence,
            this.clIDA,
            this.clIDB,
            this.clPayloadLength,
            this.clPayload,
            this.clChecksum});
            this.radioReceivedList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.radioReceivedList.Location = new System.Drawing.Point(3, 3);
            this.radioReceivedList.Name = "radioReceivedList";
            this.radioReceivedList.ReadOnly = true;
            this.radioReceivedList.Size = new System.Drawing.Size(970, 205);
            this.radioReceivedList.TabIndex = 2;
            // 
            // clTime
            // 
            this.clTime.HeaderText = "Time";
            this.clTime.Name = "clTime";
            this.clTime.ReadOnly = true;
            this.clTime.Width = 55;
            // 
            // clSequence
            // 
            this.clSequence.HeaderText = "Sequence";
            this.clSequence.Name = "clSequence";
            this.clSequence.ReadOnly = true;
            this.clSequence.Width = 81;
            // 
            // clIDA
            // 
            this.clIDA.HeaderText = "ID A";
            this.clIDA.Name = "clIDA";
            this.clIDA.ReadOnly = true;
            this.clIDA.Width = 43;
            // 
            // clIDB
            // 
            this.clIDB.HeaderText = "ID B";
            this.clIDB.Name = "clIDB";
            this.clIDB.ReadOnly = true;
            this.clIDB.Width = 43;
            // 
            // clPayloadLength
            // 
            this.clPayloadLength.HeaderText = "Payload Length";
            this.clPayloadLength.Name = "clPayloadLength";
            this.clPayloadLength.ReadOnly = true;
            this.clPayloadLength.Width = 97;
            // 
            // clPayload
            // 
            this.clPayload.HeaderText = "Payload";
            this.clPayload.Name = "clPayload";
            this.clPayload.ReadOnly = true;
            this.clPayload.Width = 70;
            // 
            // clChecksum
            // 
            this.clChecksum.HeaderText = "Checksum?";
            this.clChecksum.Name = "clChecksum";
            this.clChecksum.ReadOnly = true;
            this.clChecksum.Width = 69;
            // 
            // tbRadioSetup
            // 
            this.tbRadioSetup.Controls.Add(this.cmdCOMPortOpenClose);
            this.tbRadioSetup.Controls.Add(this.label1);
            this.tbRadioSetup.Controls.Add(this.cmbCOMPorts);
            this.tbRadioSetup.Location = new System.Drawing.Point(4, 22);
            this.tbRadioSetup.Name = "tbRadioSetup";
            this.tbRadioSetup.Size = new System.Drawing.Size(976, 211);
            this.tbRadioSetup.TabIndex = 2;
            this.tbRadioSetup.Text = "Radio Setup";
            this.tbRadioSetup.UseVisualStyleBackColor = true;
            // 
            // cmdCOMPortOpenClose
            // 
            this.cmdCOMPortOpenClose.Location = new System.Drawing.Point(163, 8);
            this.cmdCOMPortOpenClose.Name = "cmdCOMPortOpenClose";
            this.cmdCOMPortOpenClose.Size = new System.Drawing.Size(41, 21);
            this.cmdCOMPortOpenClose.TabIndex = 2;
            this.cmdCOMPortOpenClose.Text = "Open";
            this.cmdCOMPortOpenClose.UseVisualStyleBackColor = true;
            this.cmdCOMPortOpenClose.Click += new System.EventHandler(this.cmdCOMPortOpenClose_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 11);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(56, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "COM Port:";
            // 
            // cmbCOMPorts
            // 
            this.cmbCOMPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbCOMPorts.FormattingEnabled = true;
            this.cmbCOMPorts.Location = new System.Drawing.Point(65, 8);
            this.cmbCOMPorts.Name = "cmbCOMPorts";
            this.cmbCOMPorts.Size = new System.Drawing.Size(92, 21);
            this.cmbCOMPorts.TabIndex = 0;
            // 
            // tbLogging
            // 
            this.tbLogging.Location = new System.Drawing.Point(4, 22);
            this.tbLogging.Name = "tbLogging";
            this.tbLogging.Size = new System.Drawing.Size(976, 211);
            this.tbLogging.TabIndex = 3;
            this.tbLogging.Text = "Logging";
            this.tbLogging.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(484, 437);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "General";
            // 
            // groupBox2
            // 
            this.groupBox2.Location = new System.Drawing.Point(508, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(484, 437);
            this.groupBox2.TabIndex = 3;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Map";
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.txtToolStrip});
            this.statusStrip.Location = new System.Drawing.Point(0, 707);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(1008, 22);
            this.statusStrip.TabIndex = 4;
            // 
            // txtToolStrip
            // 
            this.txtToolStrip.Name = "txtToolStrip";
            this.txtToolStrip.Size = new System.Drawing.Size(47, 17);
            this.txtToolStrip.Text = "Started.";
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1008, 729);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.tabControl1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "frmMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "UAV Ground Control v2.0";
            this.Load += new System.EventHandler(this.frmMain_Load);
            this.tabControl1.ResumeLayout(false);
            this.tbRadioLog.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.radioReceivedList)).EndInit();
            this.tbRadioSetup.ResumeLayout(false);
            this.tbRadioSetup.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tbRadioLog;
        private System.Windows.Forms.DataGridView radioReceivedList;
        private System.Windows.Forms.DataGridViewTextBoxColumn clTime;
        private System.Windows.Forms.DataGridViewTextBoxColumn clSequence;
        private System.Windows.Forms.DataGridViewTextBoxColumn clIDA;
        private System.Windows.Forms.DataGridViewTextBoxColumn clIDB;
        private System.Windows.Forms.DataGridViewTextBoxColumn clPayloadLength;
        private System.Windows.Forms.DataGridViewTextBoxColumn clPayload;
        private System.Windows.Forms.DataGridViewCheckBoxColumn clChecksum;
        private System.Windows.Forms.TabPage tbCommands;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TabPage tbWaypoints;
        private System.Windows.Forms.TabPage tbRadioSetup;
        private System.Windows.Forms.TabPage tbLogging;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cmbCOMPorts;
        private System.Windows.Forms.Button cmdCOMPortOpenClose;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel txtToolStrip;
    }
}

