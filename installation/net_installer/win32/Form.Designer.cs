/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
namespace Titanium
{
    partial class form
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
            this.backgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.label = new System.Windows.Forms.Label();
            this.button = new System.Windows.Forms.Button();
            this.progress = new System.Windows.Forms.ProgressBar();
            this.SuspendLayout();
            // 
            // backgroundWorker
            // 
            this.backgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker_DoWork_1);
            // 
            // label
            // 
            this.label.AutoSize = true;
            this.label.Location = new System.Drawing.Point(20, 18);
            this.label.Name = "label";
            this.label.Size = new System.Drawing.Size(227, 13);
            this.label.TabIndex = 0;
            this.label.Text = "Configuring your system. One moment please...";
            // 
            // button
            // 
            this.button.Location = new System.Drawing.Point(133, 77);
            this.button.Name = "button";
            this.button.Size = new System.Drawing.Size(75, 27);
            this.button.TabIndex = 1;
            this.button.Text = "Cancel";
            this.button.UseVisualStyleBackColor = true;
            this.button.Click += new System.EventHandler(this.button_Click);
            // 
            // progress
            // 
            this.progress.ForeColor = System.Drawing.Color.Fuchsia;
            this.progress.Location = new System.Drawing.Point(23, 44);
            this.progress.Name = "progress";
            this.progress.Size = new System.Drawing.Size(296, 20);
            this.progress.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
            this.progress.TabIndex = 2;
            // 
            // form
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(348, 115);
            this.ControlBox = false;
            this.Controls.Add(this.progress);
            this.Controls.Add(this.button);
            this.Controls.Add(this.label);
            this.Name = "form";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Installer App";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.form_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.ComponentModel.BackgroundWorker backgroundWorker;
        private System.Windows.Forms.Label label;
        private System.Windows.Forms.Button button;
        private System.Windows.Forms.ProgressBar progress;
        private string tempdir;
        private string installdir;
        private string[] urls;
        private string unzipper;


        private delegate void UpdateTextDelegate(string message);
        private void UpdateText(string message)
        {
            this.label.Text = message;
        }
        private UpdateTextDelegate textDelegate;
        
        private delegate void UpdateProgressDelegate(int value, int max);
        private void UpdateProgress(int value, int max)
        {
            this.progress.Style = value < 0 ? System.Windows.Forms.ProgressBarStyle.Continuous : System.Windows.Forms.ProgressBarStyle.Blocks;
            this.progress.Value = value;
            this.progress.Maximum = max;
        }
        private UpdateProgressDelegate progressDelegate;
    }
}

