namespace ObjImportGUI
{
    partial class Form3
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
            this.bank1 = new System.Windows.Forms.Label();
            this.bank2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // bank1
            // 
            this.bank1.AutoSize = true;
            this.bank1.Location = new System.Drawing.Point(51, 112);
            this.bank1.Name = "bank1";
            this.bank1.Size = new System.Drawing.Size(59, 13);
            this.bank1.TabIndex = 15;
            this.bank1.Text = "Bank 0x0C";
            // 
            // bank2
            // 
            this.bank2.AutoSize = true;
            this.bank2.Location = new System.Drawing.Point(51, 152);
            this.bank2.Name = "bank2";
            this.bank2.Size = new System.Drawing.Size(60, 13);
            this.bank2.TabIndex = 16;
            this.bank2.Text = "Bank 0x0D";
            // 
            // Form3
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(662, 266);
            this.Controls.Add(this.bank2);
            this.Controls.Add(this.bank1);
            this.Name = "Form3";
            this.Text = "Bank Selector";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label bank1;
        private System.Windows.Forms.Label bank2;
    }
}