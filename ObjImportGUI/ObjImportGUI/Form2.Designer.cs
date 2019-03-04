namespace ObjImportGUI
{
    partial class Form2
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
            this.label1 = new System.Windows.Forms.Label();
            this.RevertLevelCombo = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.RevertButton = new System.Windows.Forms.Button();
            this.CancelRevertButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(22, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(343, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "Reverting a level will restore the original Mario 64 level in your ROM file.";
            // 
            // RevertLevelCombo
            // 
            this.RevertLevelCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.RevertLevelCombo.FormattingEnabled = true;
            this.RevertLevelCombo.Items.AddRange(new object[] {
            "Haunted House",
            "Cool Cool Mountain",
            "Inside Castle",
            "Hazy Maze Cave",
            "Shifting Sand Land",
            "Bob-Omb\'s Battlefield",
            "Snow Man\'s land",
            "Wet Dry World",
            "Jolly Roger Bay",
            "Tiny Huge Island",
            "Tick Tock Clock",
            "Rainbow Ride",
            "Castle Grounds",
            "Bowser First Course",
            "Vanish Cap",
            "Bowser\'s Fire Sea",
            "Secret Aquarium",
            "Bowser Third Course",
            "Lethal Lava Land",
            "Dire Dire Docks",
            "Whomp\'s Fortress",
            "Picture at the end",
            "Castle Courtyard",
            "Peach\'s Secret Slide",
            "Metal Cap",
            "Wing Cap",
            "Bowser First Battle",
            "Rainbow Clouds",
            "Bowser Second Battle",
            "Bowser Third Battle",
            "Tall Tall Mountain",
            "First Screen (\"Super Mario 64\")"});
            this.RevertLevelCombo.Location = new System.Drawing.Point(74, 64);
            this.RevertLevelCombo.Name = "RevertLevelCombo";
            this.RevertLevelCombo.Size = new System.Drawing.Size(216, 21);
            this.RevertLevelCombo.TabIndex = 12;
            this.RevertLevelCombo.SelectedIndexChanged += new System.EventHandler(this.RevertLevelCombo_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(22, 67);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(36, 13);
            this.label2.TabIndex = 14;
            this.label2.Text = "Level:";
            // 
            // RevertButton
            // 
            this.RevertButton.Location = new System.Drawing.Point(309, 61);
            this.RevertButton.Name = "RevertButton";
            this.RevertButton.Size = new System.Drawing.Size(73, 25);
            this.RevertButton.TabIndex = 15;
            this.RevertButton.Text = "Revert";
            this.RevertButton.UseVisualStyleBackColor = true;
            this.RevertButton.Click += new System.EventHandler(this.RevertButton_Click);
            // 
            // CancelRevertButton
            // 
            this.CancelRevertButton.Location = new System.Drawing.Point(388, 61);
            this.CancelRevertButton.Name = "CancelRevertButton";
            this.CancelRevertButton.Size = new System.Drawing.Size(73, 25);
            this.CancelRevertButton.TabIndex = 16;
            this.CancelRevertButton.Text = "Cancel";
            this.CancelRevertButton.UseVisualStyleBackColor = true;
            this.CancelRevertButton.Click += new System.EventHandler(this.CancelRevertButton_Click);
            // 
            // Form2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(477, 114);
            this.Controls.Add(this.CancelRevertButton);
            this.Controls.Add(this.RevertButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.RevertLevelCombo);
            this.MaximumSize = new System.Drawing.Size(485, 148);
            this.MinimumSize = new System.Drawing.Size(485, 148);
            this.Name = "Form2";
            this.Text = "Choose level to revert";
            this.Load += new System.EventHandler(this.Form2_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox RevertLevelCombo;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button RevertButton;
        private System.Windows.Forms.Button CancelRevertButton;
    }
}