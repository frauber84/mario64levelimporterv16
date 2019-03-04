using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace ObjImportGUI
{
    public partial class Form2 : Form
    {

        public static string ROMFile = "No file loaded!";
        public static int Level = 0;

        public Form2(string RomFile, int level)
        {
            InitializeComponent();
            RevertLevelCombo.SelectedIndex = 0;
            this.StartPosition = FormStartPosition.CenterParent;
            this.MaximizeBox = false;
            ROMFile = RomFile;
            Level = level;
            string debug = "ROMFile = " + ROMFile.ToString() + "  Level = " + Level.ToString();

        }

        private void CancelRevertButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form2_Load(object sender, EventArgs e)
        {

        }

        private void RevertLevelCombo_SelectedIndexChanged(object sender, EventArgs e)
        {

            

        }


        private void RevertButton_Click(object sender, EventArgs e)
        {

            if (RevertLevelCombo.SelectedIndex == -1) { MessageBox.Show("Invalid level!", "Error"); return; }

            FileInfo rom = new FileInfo(ROMFile);

            FileInfo executableFileInfo = new FileInfo(Application.ExecutablePath);
            string ApplicationPath = executableFileInfo.DirectoryName + "\\obj_import.exe";
            if (File.Exists(ApplicationPath) == false) { MessageBox.Show("File obj_import.exe not found! (Must be in the same directory as this program).", "Error"); return; }

            string SettingsPath = executableFileInfo.DirectoryName + "\\settings.txt";
            StreamWriter Settings = new StreamWriter(SettingsPath);

            
            string ApplicationArguments = "\"" + "noobjfile" + "\" \"" + ROMFile + "\" ";

            Settings.WriteLine("Level " + RevertLevelCombo.SelectedIndex.ToString());
            Settings.WriteLine("Restore 1");
            Settings.Close();

            ApplicationArguments = ApplicationArguments + " nocustombg";
            ApplicationArguments = ApplicationArguments + " \"" + SettingsPath + " \"";

            Process ProcessObj = new Process();
            
            ProcessObj.StartInfo.FileName = ApplicationPath;
            ProcessObj.StartInfo.Arguments = ApplicationArguments;
            ProcessObj.StartInfo.UseShellExecute = false;
            ProcessObj.Start();
            ProcessObj.WaitForExit();

            if (ProcessObj.ExitCode == 0x09) { MessageBox.Show("Level sucessfully restored!"); }
            else if (ProcessObj.ExitCode == 0x0102) { MessageBox.Show("Not a proper ROM!", "Error"); }
            else { 
                string message;
                message = "Whoops, something went wrong! (" + Convert.ToString(ProcessObj.ExitCode) + ")";
                MessageBox.Show(message, "Error"); 
            }

        }
    }

        
}
