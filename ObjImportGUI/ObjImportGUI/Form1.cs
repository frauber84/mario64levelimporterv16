using System;
using System.Collections.Generic;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Xml;
using System.IO;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace ObjImportGUI
{

    public partial class MainForm : Form
    {


        public static string RomFileCopy = "No file loaded!";
        public static bool DontOpenBG = false;
        public static bool DontOpenMusic = false;
        public static bool MusicHack = false;
        public static int[] CollisionList = new int[500];
        public static int[] ColParam1 = new int[500];
        public static int[] ColParam2 = new int[500];
        public int CollisionChange = 0;
        public int FoundMtl = 0;
        public static int CurrentTab;
        public static int PreviousTab;
        public static int MusicStatus = 0;
        public static int CurrentMusicCopy;
        List<NInst> CurrentNInstList = new List<NInst>();
        List<Sequence> CurrentSequenceList = new List<Sequence>();
        List<Collision> CurrentCollisionList = new List<Collision>();

        int[] CTypeList = {
            0x00,
            0x01,
            0x05,
            0x0A,
            0x0B,
            0x0D,
            0x0E,
            0x12,
            0x13,
            0x14,
            0x15,
            0x1A,
            0x1B,
            0x1C,
            0x1D,
            0x1E,
            0x21,
            0x22,
            0x23,
            0x24,
            0x25,
            0x26,
            0x27,
            0x28,
            0x29,
            0x2A,
            0x2C,
            0x2D,
            0x2E,
            0x2F,
            0x30,
            0x32,
            0x33,
            0x34,
            0x35,
            0x36,
            0x37,
            0x38,
            0x65,
            0x66,
            0x6E,
            0x6F,
            0x70,
            0x72,
            0x75,
            0x76,
            0x79,
            0x7A,
            0X7B,
            0xFD,
            0xFF,
            80, /* non solid */
                          };



        private void AddBlankInstruments(int InstCount)
        {
            for (int i = 0; i < InstCount; i++)
            {
                InstList.Items.Add("Sound" + i.ToString() + " - ?? ");
            }
        }

        private void AddInstrument(String inst)
        {
            InstList.Items.Add(inst);
        }




        public string TrimString(string str)
        {
            
            string newstr;

            try
            {
                string pattern = @"^[ \t]+|[ \t]+$";
                Regex reg = new Regex(pattern, RegexOptions.IgnoreCase);
                newstr = reg.Replace(str, "");
                return newstr;
            }
            catch (Exception ex)
            {
                string error = ex.ToString();
                return str; // do not trim line
            }
            
        }

        public void WritePatches()
        {

            FileStream fs = new FileStream(RomFile.Text, FileMode.Open, FileAccess.ReadWrite);
            BinaryReader br = new BinaryReader(fs);
            BinaryWriter bw = new BinaryWriter(fs);

            bool DoPatch = true;

            // update Music Hack state
            bw.Seek(0xD213A, SeekOrigin.Begin);
            UInt16 seqCheck = SwapUInt16(br.ReadUInt16());
            if (seqCheck == 0x8057)
            {
                MusicHackOn.Checked = true;
                MusicHack = true;
            }
            else
            {
                MusicHackOff.Checked = true;
                MusicHack = false;
            }

            // check collision hack
            bw.Seek(0x1000DA, SeekOrigin.Begin);
            UInt16 colCheck = SwapUInt16(br.ReadUInt16());
            if (colCheck == 0x8075) DoPatch = false;

            if (DoPatch == true)
            {
                MessageBox.Show("Your ROM will now be patched for collision improvements (this only happens once)");

                /* Collision Hack (increase max trangles from 2300 to 6500) */
                bw.Seek(0x100100, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x3C028067));
                bw.Write(SwapInt32(0x34420000));
                bw.Seek(0x1000C8, SeekOrigin.Begin); // max number of trianles
                bw.Write(SwapInt32(0x240E1964));
                bw.Seek(0x1000D8, SeekOrigin.Begin); // PTR for aray of triangles?
                bw.Write(SwapInt32(0x3C028075)); // redo all those static memory allocations!!
                bw.Write(SwapInt32(0x00000000));

                MessageBox.Show("Done!");
            }
            br.Close();
            bw.Close();
            fs.Close();

        }
        public void ReadMusicData()
        {
            FileStream fs = new FileStream(RomFile.Text, FileMode.Open, FileAccess.ReadWrite);
            BinaryReader br = new BinaryReader(fs);
            BinaryWriter bw = new BinaryWriter(fs);

            bw.Seek(0x7B085F, SeekOrigin.Begin);
            uint ch = br.ReadByte();

            if (ch == 0x00)
            {
                MessageBox.Show("Your ROM will now be patched for additional music settings and sequences (this process only happens once).");

                bw.Seek(0x7B0863, SeekOrigin.Begin);
                int SequenceCount = br.ReadByte();

                List<Sequence> SequenceList = new List<Sequence>();
                for (int i = 0; i < SequenceCount; i++)
                {
                    UInt32 StartOffset = SwapInt32(br.ReadUInt32());
                    UInt32 Lenght = SwapInt32(br.ReadUInt32());
                    SequenceList.Add(new Sequence(StartOffset, Lenght));
                }

                foreach (Sequence seq in SequenceList) seq.ReadData(fs, br, bw, 0x7B0860);

                bw.Seek(0x2F001b0, SeekOrigin.Begin);

                foreach (Sequence seq in SequenceList) seq.WriteData(fs, br, bw);

                //write table
                bw.Seek(0x2F00000, SeekOrigin.Begin);
                bw.Write(SwapInt16(0x0003));
                bw.Write(SwapInt16(0x0035));

                foreach (Sequence seq in SequenceList)
                {
                    bw.Write(SwapInt32(seq.NewStartOffset));
                    bw.Write(SwapInt32(seq.Lenght));
                }

                // (Adding extra sequences)
                for (int i = 0; i < (0x35 - SequenceCount); i++)
                {
                    bw.Write(SwapInt32(0x000001b0));
                    bw.Write(SwapInt32(0x00003490));
                }

                bw.Seek(0x7B085F, SeekOrigin.Begin);
                bw.Write((byte)0x11); // mark this operation

                // patch BlockDmaCopy that copy sequence list
                bw.Seek(0xD4714, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x3C0402F0));  // LUI A0, 0x02F0
                bw.Seek(0xD471C, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x24840000));  // ADDIU A0, A0, 0x0000
                bw.Seek(0xD4768, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x3C0402F0));  // LUI A0, 0x02F0
                bw.Seek(0xD4770, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x24840000));  // ADDIU A0, A0, 0x0000
                bw.Seek(0xD4784, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x3C0502F0));  // LUI A1, 0x02F0
                bw.Seek(0xD4788, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x24A50000));  // ADDIU A1, A1, 0x0000

                // Deal with Instrument Sets
                bw.Seek(0x7CC620, SeekOrigin.Begin);
                List<NInst> NInstList = new List<NInst>();
                for (int i = 0; i < SequenceCount; i++)
                {
                    UInt16 StartOffset = SwapUInt16(br.ReadUInt16());
                    NInstList.Add(new NInst(StartOffset));
                }

                foreach (NInst inst in NInstList) inst.ReadNInst(fs, br, bw, 0x7CC620);

                // (Adding extra sequences)
                for (int i = 0; i < (0x35 - SequenceCount); i++)
                {
                    NInstList.Add(new NInst(1, 0x11));
                }


                bw.Seek(0x7f006a, SeekOrigin.Begin);
                foreach (NInst inst in NInstList) inst.WriteNInst(fs, br, bw);

                //write table
                bw.Seek(0x7f0000, SeekOrigin.Begin);
                foreach (NInst inst in NInstList)
                {
                    bw.Write(SwapUInt16(inst.NewStartOffset));
                }

                bw.Seek(0xD48C6, SeekOrigin.Begin);
                bw.Write(SwapInt16(0x007f));
                bw.Seek(0xD48CC, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x34840000));

                bw.Seek(0xD48DA, SeekOrigin.Begin); // BlockDMA lenght for ninsts
                bw.Write(SwapInt16(0x0200));

                // Names
                bw.Seek(0x7f1000, SeekOrigin.Begin);
                bw.Write("00 - No music");
                bw.Write("01 - End level");
                bw.Write("02 - SMB (Title Screen)");
                bw.Write("03 - Bob-omb's Battlefield");
                bw.Write("04 - Inside Castle walls");
                bw.Write("05 - Dire Dire Docks");
                bw.Write("06 - Lethal Laval land");
                bw.Write("07 - Bowser battle");
                bw.Write("08 - Snow");
                bw.Write("09 - Slide");
                bw.Write("10 - Haunted House");
                bw.Write("11 - Piranha plant lullaby");
                bw.Write("12 - Hazy Maze");
                bw.Write("13 - Star select");
                bw.Write("14 - Wing cap");
                bw.Write("15 - Metal cap");
                bw.Write("16 - Bowser Message");
                bw.Write("17 - Bowser course");
                bw.Write("18 - Star catch");
                bw.Write("19 - Ghost Merry-go-round");
                bw.Write("20 - Start and End Race with Koopa the Quick");
                bw.Write("21 - Star appears");
                bw.Write("22 - Boss fight");
                bw.Write("23 - Take a Key");
                bw.Write("24 - Looping stairs");
                bw.Write("25 - Bowser Fight");
                bw.Write("26 - Credits song");
                bw.Write("27 - 1-up");
                bw.Write("28 - Toad");
                bw.Write("29 - Peach message");
                bw.Write("30 - Intro Castle sequence");
                bw.Write("31 - End fanfare");
                bw.Write("32 - End music");
                bw.Write("33 - Menu");
                bw.Write("34 - Lakitu");
                bw.Write("35 - Custom Music 1");
                bw.Write("36 - Custom Music 2");
                bw.Write("37 - Custom Music 3");
                bw.Write("38 - Custom Music 4");
                bw.Write("39 - Custom Music 5");
                bw.Write("40 - Custom Music 6");
                bw.Write("41 - Custom Music 7");
                bw.Write("42 - Custom Music 8");
                bw.Write("43 - Custom Music 9");
                bw.Write("44 - Custom Music 10");
                bw.Write("45 - Custom Music 11");
                bw.Write("46 - Custom Music 12");
                bw.Write("47 - Custom Music 13");
                bw.Write("48 - Custom Music 14");
                bw.Write("49 - Custom Music 15");
                bw.Write("50 - Custom Music 16");
                bw.Write("51 - Custom Music 17");
                bw.Write("52 - Custom Music 18");
                bw.Write("53 - Custom Music 19");

                bw.Seek(0x7f1000, SeekOrigin.Begin);

                int selectedMusic = Music.SelectedIndex;
                int selectedCurrentMusic = CurrentMusic.SelectedIndex;

                // populate music boxes
                Music.Items.Clear();
                CurrentMusic.Items.Clear();

                for (int i = 0; i <= 0x35; i++)
                {
                    string currentsong = br.ReadString();
                    Music.Items.Add(currentsong);
                    CurrentMusic.Items.Add(currentsong);
                }

                bw.Seek(0x7f1000, SeekOrigin.Begin);

                // populate music boxes
                Music.Items.Clear();
                CurrentMusic.Items.Clear();

                for (int i = 0; i <= 0x35; i++)
                {
                    string currentsong = br.ReadString();
                    Music.Items.Add(currentsong);
                    CurrentMusic.Items.Add(currentsong);
                }

                CurrentNInstList.Clear();

                bw.Seek(0x7f0000, SeekOrigin.Begin);
                for (int i = 0; i < 0x35; i++)
                {
                    UInt16 StartOffset = SwapUInt16(br.ReadUInt16());
                    CurrentNInstList.Add(new NInst(StartOffset, (UInt32)i, 0));
                }
                foreach (NInst inst in CurrentNInstList) inst.ReadNInst(fs, br, bw, 0x7F0000);

                CurrentSequenceList.Clear();
                bw.Seek(0x2F00003, SeekOrigin.Begin);
                SequenceCount = br.ReadByte();
                for (int i = 0; i < SequenceCount; i++)
                {
                    UInt32 StartOffset = SwapInt32(br.ReadUInt32());
                    UInt32 Lenght = SwapInt32(br.ReadUInt32());
                    CurrentSequenceList.Add(new Sequence(StartOffset, Lenght));
                }

                Music.SelectedIndex = selectedMusic;
                CurrentMusic.SelectedIndex = selectedCurrentMusic;

                MessageBox.Show("Done!");

            }
            else if (ch == 0x11)
            {

                int selectedMusic = Music.SelectedIndex;
                int selectedCurrentMusic = CurrentMusic.SelectedIndex;

                bw.Seek(0x7f1000, SeekOrigin.Begin);

                // populate music boxes
                Music.Items.Clear();
                CurrentMusic.Items.Clear();

                for (int i = 0; i <= 0x35; i++)
                {
                    string currentsong = br.ReadString();
                    Music.Items.Add(currentsong);
                    CurrentMusic.Items.Add(currentsong);
                }

                CurrentNInstList.Clear();

                bw.Seek(0x7f0000, SeekOrigin.Begin);
                for (int i = 0; i < 0x35; i++)
                {
                    UInt16 StartOffset = SwapUInt16(br.ReadUInt16());
                    CurrentNInstList.Add(new NInst(StartOffset, (UInt32)i, 0));
                }
                foreach (NInst inst in CurrentNInstList) inst.ReadNInst(fs, br, bw, 0x7F0000);

                // load sequence data
                CurrentSequenceList.Clear();
                bw.Seek(0x2F00003, SeekOrigin.Begin);
                int SequenceCount = br.ReadByte();
                for (int i = 0; i < SequenceCount; i++)
                {
                    UInt32 StartOffset = SwapInt32(br.ReadUInt32());
                    UInt32 Lenght = SwapInt32(br.ReadUInt32());
                    CurrentSequenceList.Add(new Sequence(StartOffset, Lenght));
                }

                Music.SelectedIndex = selectedMusic;
                CurrentMusic.SelectedIndex = selectedCurrentMusic;

            }


            br.Close();
            bw.Close();
            fs.Close();
        }

        public bool CheckROM()
        {

            if ( ! (File.Exists(RomFile.Text)) ) return false;

            FileInfo rom = new FileInfo(RomFile.Text);
            uint ch;

            if (rom.Length < 0x2FFFFF0)
            {
                return false;
            }
            
            FileStream fs = new FileStream(RomFile.Text, FileMode.Open, FileAccess.ReadWrite);
            BinaryReader br = new BinaryReader(fs);
            BinaryWriter bw = new BinaryWriter(fs);

            bw.Seek(0x12015C7, SeekOrigin.Begin);
            ch = br.ReadByte();
            br.Close();
            bw.Close();
            fs.Close();

            if (ch != 0x14) return false;
            
            return true;
        }

        public MainForm()
        {
            InitializeComponent();
            this.StartPosition = FormStartPosition.CenterScreen;
        }

        private static string CheckBoxState(CheckBox chbox)
        {
            if (chbox.Checked == true) return "1";
            else return "0";
        }
        public static float SwapFloat32(float value)
        {
            Byte[] bytes = BitConverter.GetBytes(value);
            Array.Reverse(bytes);
            return BitConverter.ToSingle(bytes, 0);
        }

        public static Int16 SwapInt16(Int16 value)
        {
            Byte[] bytes = BitConverter.GetBytes(value);
            Array.Reverse(bytes);
            return BitConverter.ToInt16(bytes, 0);
        }

        public static UInt16 SwapUInt16(UInt16 value)
        {
            Byte[] bytes = BitConverter.GetBytes(value);
            Array.Reverse(bytes);
            return BitConverter.ToUInt16(bytes, 0);
        }

        public static UInt32 SwapInt32(UInt32 value)
        {
            Byte[] bytes = BitConverter.GetBytes(value);
            Array.Reverse(bytes);
            return BitConverter.ToUInt32(bytes, 0);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            TextButton.Enabled = false;
            TextButton.Visible = false;
            SwapXZ.Enabled = false;
            SwapXZ.Visible = false;
            Renderer.SelectedIndex = 0;
            AngleY.Enabled = false;
            Brightness.SelectedIndex = 0; 
            CameraOn.Checked = true;
            PauseWarpLevel.SelectedIndex = 2;
            CameraList.SelectedIndex = 0;
            label49.Visible = false;
            Camera.SelectedIndex = 0;
            SlideTime.Enabled = false;
            SlideTime.Value = 21;
            label28.Visible = false;
            WaterType.Enabled = false;
            WaterX.Enabled = false;
            WaterX2.Enabled = false;
            WaterZ.Enabled = false;
            WaterZ2.Enabled = false;
            WaterX.Enabled = false;
            StarX.Enabled = false;
            StarY.Enabled = false;
            StarZ.Enabled = false;
            BoxList.Enabled = false;
            AddWaterBox.Enabled = false;
            RemoveWaterBox.Enabled = false;
            WaterHeight.Enabled = false;
            CParam1.Enabled = false;
            CParam2.Enabled = false;
            this.MaximizeBox = false;
            OffsetX.Enabled = false;
            OffsetY.Enabled = false;
            OffsetZ.Enabled = false;
            BGFile.BackColor = System.Drawing.SystemColors.Control;
            BGFile.Enabled = false;
            MarioX.Enabled = false;
            MarioY.Enabled = false;
            MarioZ.Enabled = false;
            
            FogR.Enabled = false;
            FogG.Enabled = false;
            FogB.Enabled = false;
            FogPreset.Enabled = false;
            

            //System.Windows.Forms.ToolTip ToolTip1 = new System.Windows.Forms.ToolTip();
            //ToolTip1.SetToolTip(this.FlipTextures, "Try enabling this if your texture seems flipped");

            RomFile.Text = "No file loaded!";
            objFile.Text = "No file loaded!";
            BGFile.Text = "No file loaded!";
            TextureSizeLabel.Text = "";

            label37.Text = "";
            LevelList.SelectedIndex = 12;
            StarList.SelectedIndex = 0;
            TrajectoryList.SelectedIndex = 0;
            ActSelector.SelectedIndex = 0;
            StartLives.Value = 4;
            TrajectoryFile.Text = "No file loaded!";
            MtlFile.Text = "No material library associated with this .obj file";
            BoxFile.Text = "No file loaded!";
            ActOn.Checked = true;
            TextureTooBig.Visible = false;

            DeathHeight.Minimum = -8192;
            DeathHeight.Maximum = 8192;
            DeathHeight.Value = -8192;
            DeathHeight.Enabled = false;

            WaterX.Value = -8192;
            WaterZ.Value = +8192;
            WaterX2.Value = +8192;
            WaterZ2.Value = -8192;
            WaterHeight.Value = 1000;
            WaterType.SelectedIndex = 0;
            Music.SelectedIndex = 0;
            CurrentMusic.SelectedIndex = 1;
            Terrain.SelectedIndex = 0;
            BankC.SelectedIndex = 0;
            FogPreset.SelectedIndex = 0;
            BankD.SelectedIndex = 0;
            BankE.SelectedIndex = 0;
            LevelListComboBox.SelectedIndex = 0; // bob-omb
            Weather.SelectedIndex = 0; // none
            BackGround.SelectedIndex = 4;
            ScalingValue.Text = "500";
            FogR.Hexadecimal = true;
            FogG.Hexadecimal = true;
            FogB.Hexadecimal = true;

            //combo box
        }

        private void GoButton_Click(object sender, EventArgs e)
        {

        }

        private void NoMtlCheckBox_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void LoadObjButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Load Wavefront .OBJ file";
            dlg.Filter = "(.obj files)|*.obj";
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                objFile.Text = dlg.FileName;

            }
        }

        private void LevelListComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            

        }

        private void label1_Click(object sender, EventArgs e)
        {
            MessageBox.Show("found 3 alligator(s). Calling Jesse Dockens ...");
            

        }

        private void LoadROM_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Load Mario 64 ROM";
            dlg.Filter = "Mario 64 ROM|*.z64; *.v64; *.rom; *.bin";
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                RomFile.Text = dlg.FileName;
                RomFileCopy = RomFile.Text;
            }


        }

        private void ScalingValue_TextChanged(object sender, EventArgs e)
        {

            try
            {
                float myNumber = float.Parse(ScalingValue.Text);
            }
            catch
            {
                MessageBox.Show("Invalid Scaling", "Error");
                ScalingValue.Text = "";
            }
            
        }

        private void Weather_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label3_Click(object sender, EventArgs e)
        {
            MessageBox.Show("eggs eggs eggs");
        }

        private void BackGround_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {
            MessageBox.Show("It's a secret to everybody...");
        }

        private void label2_Click(object sender, EventArgs e)
        {
            MessageBox.Show("All work and no play makes jack a dull boy...\nAll work and no play makes jack a dull boy...\nAll work and no play makes jack a dull boy...\nAll work and no play makes jack a dull boy...\n");
        }

        private void DeathAtBottom_CheckedChanged(object sender, EventArgs e)
        {
            if (DeathAtBottom.Checked == false)
            {
                DeathHeight.Enabled = false;
            }
            else DeathHeight.Enabled = true;
        }

        private void label5_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Don't bug me!");
        }

        private void DeathHeight_ValueChanged(object sender, EventArgs e)
        {
            

        }

        private void BankC_SelectedIndexChanged(object sender, EventArgs e)
        {

            if (BankC.SelectedIndex == 0) // haunted house, castle courtyard
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Bookend",
                "Haunted Chair",
                "Small Key",
                "Mad Piano",
                "Boo",
                "Haunted Cage"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 1)  // ccm/snow
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Spindrift",
                "Penguin",
                "Mr. Blizzard"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 2) // hazy, tall tall mountain
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Monty Mole",
                "Ukiki Monkey",
                "Fwoosh"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 3) // ssl
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Klepto",
                "Eyerock",
                "Pokey"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 4) // bob-omb
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "King Bob-Omb",
                "Water Bubble"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 5) // wet-dry, tick tock, whomp
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Yellow Sphere",
                "Hoot the Owl",
                "Yoshi Egg",
                "Thwomp",
                "Bullet Bill",
                "Heave-Ho"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 6) // jolly, aquarium
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Clam",
                "Shark",
                "Unagi"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 7) // TH island, rainbow ride, bowser1Course, dire dire docks
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Bubba",
                "Wiggler",
                "Lakitu",
                "Spinny Ball",
                "Spinny"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 8) // castle grounds
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Birds",
                "Peach",
                "Yoshi",
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 9) // vanish cap
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Beta Trampoline",
                "Cap Switches"
                });
                ObjectsC.SelectedIndex = 0;
            }
            else if (BankC.SelectedIndex == 10) // bowser fire sea, lethal lava land
            {
                ObjectsC.Items.Clear();
                ObjectsC.Items.AddRange(new object[] {
                "Small Bully",
                "Big Bully",
                "Beta Blargg"
                });
                ObjectsC.SelectedIndex = 0;
            }

        }

        private void BankD_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (BankD.SelectedIndex == 0) // haunted house, hazy maze
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Mr. I",
                "Bat (Swoop)",
                "Snufit",
                "Dorrie",
                "Scuttlebug",                
                });
                ObjectsD.SelectedIndex = 0;
            }
            else if (BankD.SelectedIndex == 1) // ccm, snowman
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Moneybags"
                });
                ObjectsD.SelectedIndex = 0;
            }
            else if (BankD.SelectedIndex == 2) // inside castle, castle grounds
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Lakitu",
                "Toads",
                "MIPS the rabbit",
                "Boo"
                });
                ObjectsD.SelectedIndex = 0;
            }
            else if (BankD.SelectedIndex == 3) // bob-omb, tiny huge
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Koopa Flag",
                "Wooden Log",
                "Koopa-the-Quick",
                "Koopa",
                "Piranha Plant",
                "Whomp",
                "Metalic Ball",
                "Chain Chomp"
                });
                ObjectsD.SelectedIndex = 0;
            }
            else if (BankD.SelectedIndex == 4) // wet-dry, jolly
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Skeeter",
                "Bowser Bomb",
                "Fish",
                "Chirp Chirp",
                "Manta Ring",
                "Treasure Chest",
                });
                ObjectsD.SelectedIndex = 0;
            }
            else if (BankD.SelectedIndex == 5) // bowser fight
            {
                ObjectsD.Items.Clear();
                ObjectsD.Items.AddRange(new object[] {
                "Bowser Flames",
                "Yellow Sphere (Bowser 1)",
                "Bowser",
                "Bowser 2",
                "Bowser Bomb",
                "Bowser Clouds",
                });
                ObjectsD.SelectedIndex = 0;
            }


        }

        private void ObjectsC_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void ObjectsD_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label9_Click(object sender, EventArgs e)
        {

        }

        private void BankE_SelectedIndexChanged(object sender, EventArgs e)
        {

            if (BankE.SelectedIndex == 0) // haunted house, hazy maze
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Haunted Staircase",
                "Floor Part",
                "Tumbling Bridge",
                "Bookshelf",
                "Tumbling Bridge",                
                "Merry-Go-Round",
                "Coffin"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 1) // ccm
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Wooden Cablecar",
                "Snowman's Bottom/Head/Base",
                "Cabin",
                "Ice Column",
                "Mountain Part"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 2)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Star Door",
                "Castle Floor Trap",
                "Pendulum",
                "Clock Big/Small Arms",
                "Brick Pillar"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 3)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Red Mesh Wall",
                "Giant Boulder",
                "Falling Rock",
                "Small Rock",
                "Metal Platform",
                "Elevator Platform"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 4)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Sand Pits",
                "Pyramid Top",
                "Tox Box",
                "Grindel",
                "Spindel",
                "Pyramid Wall",
                "Pyramid Elevator Cage"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 5)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Star Cage Bars",
                "Wooden Bridge",
                "Grills Door",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 6)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Ice Square",
                "Small Triangle",
                "Snow Wave",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 7)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Wooden Platform",
                "Arrow Lift Platform",
                "Water Level Diamond",
                "Square Wooden Platform",
                "Wooden Platform",
                "Hexagonal Platform"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 8)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Pillar (Stalagtite)",
                "Pillar Base",
                "Jolly Rock",
                "Floating Bridge",
                "Exploding Box",
                "Sunken Ship",
                "Ship Parts"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 9)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Tiny Huge Island Top",
                "Tiny Island Top",
                "Grassy Island",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 10)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Clock Cube",
                "Clock Prism",
                "Clock Pendulum",
                "Clock Metal Platform",
                "Square Prism Platform",
                "Hexagonal Gear Platform",
                "Triangular Clock Gear",
                "Clock Box",
                "Clock Platform",
                "Clock Big Arm",
                "Clockwork Gear Platform",
                "Small Gear",
                "Big Gear"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 11)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Flying Carpet",
                "Rainbow Ring",
                "Rainbow Path Parts",
                "Wing",
                "Many non-solid platforms"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 12)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Castle Flag",
                "Castle Tower (non-solid)",
                "Castle Moat Grills",
                "Castle Grills"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 13)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Quartz Cristal",
                "Quartzy Path",
                "Wooden Bridge & Fences",
                "Magic Staircase",
                "Many non-solid platforms"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 14)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Wood Platform"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 15)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Platform and Cage Track",
                "Cage Track",
                "Wire Meshes",
                "Flat Star",
                "Warp Cone",
                "Mesh Cage Elevator",
                "Cage Platform",
                "Sinking Platforms",
                "Many non-solid platforms"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 16)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Fences",
                "Platforms Path",
                "Track For Platform",
                "Pole & Spike",
                "Rainbow Staircase",
                "Many non-solid platforms",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 17)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Bascule Bridge",
                "Wooden Bridge Path",
                "Long Sinking Platform",
                "Bowser Puzzle Pieces",
                "Giant Log",
                "Many non-solid platforms",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 18)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Yellow Pole",
                "Bowser Picture Door",
                "Bowser Submarine",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 19)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Rocky Staircase",
                "Floating Brick Platform",
                "Whomp's Tower",
                "Bullet Bill Cannon",
                "Platform an Wooden Board Bridge",
                "Grass & Flowers",
                "Clock-Like Rotating Bridge",
                "Concrete Bridge",
                "Giant Pole",
                "Whomp's Tower Platforms",
                "Breakable Wall Corners",
                "Giant Wooden Board",
                "Tower Door",
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 20)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Castle Courtyard Tower"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 21)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Cloud"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 22)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Bowser's Lava Platform"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 23)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Bowser Platforms",
                "Bowser Bomb Stand"
                });
                ObjectsE.SelectedIndex = 0;
            }
            else if (BankE.SelectedIndex == 24)
            {
                ObjectsE.Items.Clear();
                ObjectsE.Items.AddRange(new object[] {
                "Little Cage",
                "Giant Wooden Log (Pitoune)",
                "Mushroom Platforms",
                "Cannon Hole",
                "Mountain Bridge Parts",
                "Vine Bridge",
                "Blue Smiley",
                "Yellow Smiley",
                "Star Smiley",
                "Moon Smiley",
                "Tall Tall Slide Exit Podium"
                });
                ObjectsE.SelectedIndex = 0;
            }

        }

        private void ObjectsE_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void Terrain_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void Music_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void Fog_CheckedChanged(object sender, EventArgs e)
        {
            if (Fog.Checked == true)
            {
                FogR.Enabled = true;
                FogG.Enabled = true;
                FogB.Enabled = true;
                FogPreset.Enabled = true;
            }
            else if (Fog.Checked == false)
            {
                FogR.Enabled = false;
                FogG.Enabled = false;
                FogB.Enabled = false;
                FogPreset.Enabled = false;
            }

        }

        private void FogPreset_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void FogR_ValueChanged(object sender, EventArgs e)
        {

        }

        private void FogG_ValueChanged(object sender, EventArgs e)
        {

        }

        private void FogB_ValueChanged(object sender, EventArgs e)
        {

        }

        private void CustomBG_CheckedChanged(object sender, EventArgs e)
        {
            if (CustomBG.Checked == true && DontOpenBG == false)
            {

                BackGround.Enabled = false;
                OpenFileDialog dlg = new OpenFileDialog();
                dlg.Title = "Load background image file";
                dlg.Filter = "Image files|*.bmp; *.png; *.jpg; *.gif";
                if (dlg.ShowDialog() == DialogResult.OK) BGFile.Text = dlg.FileName;
                BGFile.BackColor = System.Drawing.SystemColors.Window;
                BGFile.Enabled = true;
                if (BGFile.Text == "No file loaded!") CustomBG.Checked = false;
            }
            else if (CustomBG.Checked == false)
            {
                BackGround.Enabled = true;
                BGFile.Enabled = false;
                BGFile.BackColor = System.Drawing.SystemColors.Control;

            }

            if (DontOpenBG == true) DontOpenBG = false;


        }

        private void OffsetPosition_CheckedChanged(object sender, EventArgs e)
        {
            if (OffsetPosition.Checked == true)
            {
                OffsetX.Enabled = true;
                OffsetY.Enabled = true;
                OffsetZ.Enabled = true;
            }
            else if (OffsetPosition.Checked == false)
            {
                OffsetX.Enabled = false;
                OffsetY.Enabled = false;
                OffsetZ.Enabled = false;
            }
        }

        private void WaterType_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void WaterX_ValueChanged(object sender, EventArgs e)
        {

        }

        private void Water_CheckedChanged(object sender, EventArgs e)
        {

            if (Water.Checked == false)
            {
                WaterType.Enabled = false;
                WaterX.Enabled = false;
                WaterX2.Enabled = false;
                WaterZ.Enabled = false;
                WaterZ2.Enabled = false;
                WaterX.Enabled = false;
                BoxList.Enabled = false;
                AddWaterBox.Enabled = false;
                RemoveWaterBox.Enabled = false;
                WaterHeight.Enabled = false;
            }
            else if (Water.Checked == true)
            {
                WaterType.Enabled = true;
                WaterX.Enabled = true;
                WaterX2.Enabled = true;
                WaterZ.Enabled = true;
                WaterZ2.Enabled = true;
                WaterX.Enabled = true;
                BoxList.Enabled = true;
                AddWaterBox.Enabled = true;
                RemoveWaterBox.Enabled = true;
                WaterHeight.Enabled = true;
            }
        }


        private void aboutToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            
        }

        private void FlipTextures_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void LoadSettings_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Choose settings file";
            dlg.Filter = "(.xml setting files)|*.xml";
            if (dlg.ShowDialog() == DialogResult.OK)
            {

                StreamReader streamReader = new StreamReader(dlg.FileName);
                string xmlfile = streamReader.ReadToEnd();
                streamReader.Close();

                XmlDocument doc = new XmlDocument();
                doc.LoadXml(xmlfile);

                BoxList.Enabled = true;
                BoxList.Nodes.Clear();
                XmlNodeList objfile = doc.GetElementsByTagName("objfile");
                XmlNodeList romfile = doc.GetElementsByTagName("romfile");
                XmlNodeList scaling = doc.GetElementsByTagName("scaling");
                XmlNodeList levelid = doc.GetElementsByTagName("levelid");
                XmlNodeList camera = doc.GetElementsByTagName("camera");
                XmlNodeList bankc = doc.GetElementsByTagName("bankc");
                XmlNodeList bankd = doc.GetElementsByTagName("bankd");
                XmlNodeList banke = doc.GetElementsByTagName("banke");
                XmlNodeList weather = doc.GetElementsByTagName("weather");
                XmlNodeList background = doc.GetElementsByTagName("background");
                XmlNodeList terrain = doc.GetElementsByTagName("terrain");
                XmlNodeList music = doc.GetElementsByTagName("music");
                XmlNodeList deathplane = doc.GetElementsByTagName("deathplane");
                XmlNodeList deathplaneheight = doc.GetElementsByTagName("deathplaneheight");
                XmlNodeList nomtl = doc.GetElementsByTagName("nomtl");
                XmlNodeList fog = doc.GetElementsByTagName("fog");
                XmlNodeList fogpreset = doc.GetElementsByTagName("fogpreset");
                XmlNodeList fogr = doc.GetElementsByTagName("fogr");
                XmlNodeList fogg = doc.GetElementsByTagName("fogg");
                XmlNodeList fogb = doc.GetElementsByTagName("fogb");
                XmlNodeList custombg = doc.GetElementsByTagName("custombg");
                XmlNodeList bgfile = doc.GetElementsByTagName("bgfile");
                XmlNodeList offsetposition = doc.GetElementsByTagName("offsetposition");
                XmlNodeList offsetx = doc.GetElementsByTagName("offsetx");
                XmlNodeList offsety = doc.GetElementsByTagName("offsety");
                XmlNodeList offsetz = doc.GetElementsByTagName("offsetz");
                XmlNodeList waterbox = doc.GetElementsByTagName("waterbox");
                XmlNodeList water = doc.GetElementsByTagName("water");
                XmlNodeList watertype = doc.GetElementsByTagName("water");
                XmlNodeList waterx = doc.GetElementsByTagName("waterx");
                XmlNodeList waterz = doc.GetElementsByTagName("waterz");
                XmlNodeList waterheight = doc.GetElementsByTagName("waterheight");
                XmlNodeList fliptextures = doc.GetElementsByTagName("fliptextures");
                XmlNodeList swapxz = doc.GetElementsByTagName("swapxz");
                XmlNodeList rotatey = doc.GetElementsByTagName("rotatey");
                XmlNodeList angley = doc.GetElementsByTagName("angley");
                XmlNodeList bright = doc.GetElementsByTagName("bright");

                if (objfile.Count != 0) objFile.Text = objfile[0].InnerText;
                if (romfile.Count != 0) RomFile.Text = romfile[0].InnerText;
                if (romfile.Count != 0) RomFileCopy = romfile[0].InnerText;
                if (scaling.Count != 0) ScalingValue.Text = scaling[0].InnerText;
                if (levelid.Count != 0) LevelListComboBox.SelectedIndex = Convert.ToInt32(levelid[0].InnerText);
                if (camera.Count != 0) Camera.SelectedIndex = Convert.ToInt32(camera[0].InnerText);
                if (bankc.Count != 0) BankC.SelectedIndex = Convert.ToInt32(bankc[0].InnerText);
                if (bankd.Count != 0) BankD.SelectedIndex = Convert.ToInt32(bankd[0].InnerText);
                if (banke.Count != 0) BankE.SelectedIndex = Convert.ToInt32(banke[0].InnerText);
                if (weather.Count != 0) Weather.SelectedIndex = Convert.ToInt32(weather[0].InnerText);
                if (background.Count != 0) BackGround.SelectedIndex = Convert.ToInt32(background[0].InnerText);
                if (terrain.Count != 0) Terrain.SelectedIndex = Convert.ToInt32(terrain[0].InnerText);
                if (music.Count != 0) Music.SelectedIndex = Convert.ToInt32(music[0].InnerText);
                if (bright.Count != 0) Brightness.SelectedIndex = Convert.ToInt32(bright[0].InnerText);

                if (deathplane.Count != 0)
                {
                    if (deathplane[0].InnerText == "0") DeathAtBottom.Checked = false;
                    else if (deathplane[0].InnerText == "1") DeathAtBottom.Checked = true;
                }
                if (deathplaneheight.Count != 0) DeathHeight.Value = Convert.ToInt32(deathplaneheight[0].InnerText);

                if (nomtl.Count != 0)
                {
                    if (nomtl[0].InnerText == "0") NoMtlCheckBox.Checked = false;
                    else if (nomtl[0].InnerText == "1") NoMtlCheckBox.Checked = true;
                }

                if (fog.Count != 0)
                {
                    if (fog[0].InnerText == "0") Fog.Checked = false;
                    else if (fog[0].InnerText == "1") Fog.Checked = true;
                }

                if (swapxz.Count != 0)
                {
                    if (swapxz[0].InnerText == "0") SwapXZ.Checked = false;
                    else if (swapxz[0].InnerText == "1") SwapXZ.Checked = true;
                }

                if (rotatey.Count != 0)
                {
                    if (rotatey[0].InnerText == "0") RotateY.Checked = false;
                    else if (rotatey[0].InnerText == "1") RotateY.Checked = true;
                }

                if (angley.Count != 0) AngleY.Value = Convert.ToInt32(angley[0].InnerText);

                if (fogpreset.Count != 0) FogPreset.SelectedIndex = Convert.ToInt32(fogpreset[0].InnerText);
                if (fogr.Count != 0) FogR.Value = Convert.ToInt32(fogr[0].InnerText);
                if (fogg.Count != 0) FogG.Value = Convert.ToInt32(fogg[0].InnerText);
                if (fogb.Count != 0) FogB.Value = Convert.ToInt32(fogb[0].InnerText);

                if (custombg.Count != 0)
                {
                    if (custombg[0].InnerText == "0") CustomBG.Checked = false;
                    else if (custombg[0].InnerText == "1")
                    {
                        DontOpenBG = true;
                        CustomBG.Checked = true;
                    }
                }

                int i = 0;

                if (waterbox.Count != 0)
                {

                    BoxList.Nodes.Clear();

                    for (i = 0; i < waterbox.Count; i++)
                    {
                        BoxList.Nodes.Add(waterbox[i].InnerText);
                    }
                }

                if (bgfile.Count != 0) BGFile.Text = bgfile[0].InnerText;

                if (CustomBG.Checked == false) BackGround.Enabled = true;

                if (offsetposition.Count != 0)
                {
                    if (offsetposition[0].InnerText == "0") OffsetPosition.Checked = false;
                    else if (offsetposition[0].InnerText == "1") OffsetPosition.Checked = true;
                }

                if (offsetx.Count != 0) OffsetX.Value = Convert.ToInt32(offsetx[0].InnerText);
                if (offsety.Count != 0) OffsetY.Value = Convert.ToInt32(offsety[0].InnerText);
                if (offsetz.Count != 0) OffsetZ.Value = Convert.ToInt32(offsetz[0].InnerText);

                if (water.Count != 0)
                {
                    if (water[0].InnerText == "0") 
                    {
                        Water.Checked = false;
                        BoxList.Enabled = false;
                        
                    }
                    else if (water[0].InnerText == "1")
                    {
                        Water.Checked = true;
                        BoxList.Enabled = true;
                    }
                }

                if (watertype.Count != 0) WaterType.SelectedIndex = Convert.ToInt32(watertype[0].InnerText);
                if (waterx.Count != 0) WaterX.Value = Convert.ToInt32(waterx[0].InnerText);
                if (waterz.Count != 0) WaterZ.Value = Convert.ToInt32(waterz[0].InnerText);
                if (waterheight.Count != 0) WaterHeight.Value = Convert.ToInt32(waterheight[0].InnerText);

                if (fliptextures.Count != 0)
                {
                    if (fliptextures[0].InnerText == "0") FlipTextures.Checked = false;
                    else if (fliptextures[0].InnerText == "1") FlipTextures.Checked = true;
                }

            }

        }

        private void SaveSettings_Click(object sender, EventArgs e)
        {

            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save settings";
            dlg.Filter = "(.xml setting files)|*.xml";
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                XmlTextWriter textWriter = new XmlTextWriter(dlg.FileName, null);
                textWriter.WriteStartDocument();
                textWriter.WriteComment("Mario 64 Level Importer v16 Settings File");

                textWriter.WriteStartElement("level", "");

                textWriter.WriteStartElement("objfile", "");
                textWriter.WriteString(objFile.Text);
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("romfile", "");
                textWriter.WriteString(RomFile.Text);
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("scaling", "");
                textWriter.WriteString(ScalingValue.Text);
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("levelid", "");
                textWriter.WriteString(Convert.ToString(LevelListComboBox.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("camera", "");
                textWriter.WriteString(Convert.ToString(Camera.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("bankc", "");
                textWriter.WriteString(Convert.ToString(BankC.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("bankd", "");
                textWriter.WriteString(Convert.ToString(BankD.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("banke", "");
                textWriter.WriteString(Convert.ToString(BankE.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("weather", "");
                textWriter.WriteString(Convert.ToString(Weather.SelectedIndex));
                textWriter.WriteEndElement();

                textWriter.WriteStartElement("bright", "");
                textWriter.WriteString(Convert.ToString(Brightness.SelectedIndex));
                textWriter.WriteEndElement();

                textWriter.WriteStartElement("background", "");
                textWriter.WriteString(Convert.ToString(BackGround.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("terrain", "");
                textWriter.WriteString(Convert.ToString(Terrain.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("music", "");
                textWriter.WriteString(Convert.ToString(Music.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("deathplane", "");
                if (DeathAtBottom.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("deathplaneheight", "");
                textWriter.WriteString(Convert.ToString(DeathHeight.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("nomtl", "");
                if (NoMtlCheckBox.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("fog", "");
                if (Fog.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("fogpreset", "");
                textWriter.WriteString(Convert.ToString(FogPreset.SelectedIndex));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("fogr", "");
                textWriter.WriteString(Convert.ToString(FogR.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("fogg", "");
                textWriter.WriteString(Convert.ToString(FogG.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("fogb", "");
                textWriter.WriteString(Convert.ToString(FogB.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("custombg", "");
                if (CustomBG.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("bgfile", "");
                textWriter.WriteString(BGFile.Text);
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("offsetposition", "");
                if (OffsetPosition.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("offsetx", "");
                textWriter.WriteString(Convert.ToString(OffsetX.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("offsety", "");
                textWriter.WriteString(Convert.ToString(OffsetY.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("offsetz", "");
                textWriter.WriteString(Convert.ToString(OffsetZ.Value));
                textWriter.WriteEndElement();
                textWriter.WriteStartElement("water", "");
                if (Water.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();

                foreach (TreeNode tn in BoxList.Nodes)
                {
                    textWriter.WriteStartElement("waterbox", "");
                    textWriter.WriteString(tn.Text);
                    textWriter.WriteEndElement();
                }

                textWriter.WriteStartElement("fliptextures", "");
                if (FlipTextures.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();

                textWriter.WriteStartElement("swapxz", "");
                if (SwapXZ.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();

                textWriter.WriteStartElement("rotatey", "");
                if (RotateY.Checked == true) textWriter.WriteString("1");
                else textWriter.WriteString("0");
                textWriter.WriteEndElement();

                textWriter.WriteStartElement("angley", "");
                textWriter.WriteString(Convert.ToString(AngleY.Value));
                textWriter.WriteEndElement();



                textWriter.WriteEndElement(); // level
                textWriter.WriteEndDocument();
                textWriter.Close();
                MessageBox.Show("Settings saved!");

            }


        }

        private void ImportLevel_Click(object sender, EventArgs e)
        {
            try
            {
                float myNumber = float.Parse(ScalingValue.Text);
            }
            catch
            {
                MessageBox.Show("Invalid Scaling!", "Error");
                return;
            }

            if (objFile.Text == "No file loaded!") { MessageBox.Show("Load an Wavefront .obj file first!", "Error"); return; }
            if (RomFile.Text == "No file loaded!") { MessageBox.Show("Load a ROM file first!", "Error"); return; }
            if (LevelListComboBox.SelectedIndex == 31) { MessageBox.Show("Intro screen replacement isn't 100% functional yet.", "Warning"); }
            if (LevelListComboBox.SelectedIndex == 12 || LevelListComboBox.SelectedIndex == 2 || LevelListComboBox.SelectedIndex == 22) { MessageBox.Show("Warning: the level you are replacing has hardcoded data\n and may not work properly. "); }
            if (LevelListComboBox.SelectedIndex == -1) { MessageBox.Show("Invalid level!", "Error"); return; }
            if (Weather.SelectedIndex == -1) { MessageBox.Show("Invalid weather setting!", "Error"); return; }
            if (BackGround.SelectedIndex == -1) { MessageBox.Show("Invalid background texture setting!", "Error"); return; }
            if (BankC.SelectedIndex == -1) { MessageBox.Show("Invalid Object Bank 0xC setting!", "Error"); return; }
            if (BankD.SelectedIndex == -1) { MessageBox.Show("Invalid Object Bank 0xD setting!", "Error"); return; }
            if (FogPreset.SelectedIndex == -1) { MessageBox.Show("Invalid Fog Preset setting!", "Error"); return; }
            if (BankE.SelectedIndex == -1) { MessageBox.Show("Invalid Object Bank 0xE setting!", "Error"); return; }
            if (Terrain.SelectedIndex == -1) { MessageBox.Show("Invalid terrain type setting!", "Error"); return; }
            if (WaterType.SelectedIndex == -1) { MessageBox.Show("Invalid water type setting!", "Error"); return; }
            if (Music.SelectedIndex == -1) { MessageBox.Show("Invalid level music setting!", "Error"); return; }
            if (Weather.SelectedIndex == 4 && LevelListComboBox.SelectedIndex != 31) { MessageBox.Show("Lava will only work on collision type 1 (refer to readme.txt \nfor help on assigning collision types to materials)", "Warning"); }
            if (Weather.SelectedIndex == 3 && DeathAtBottom.Checked == true && LevelListComboBox.SelectedIndex != 31) MessageBox.Show("The beta flower effect may not work properly if you have a \"death at bottom\" floor", "Warning");

            if (Water.Checked == true && BoxList.Nodes.Count == 0) { MessageBox.Show("Error: Water is enabled but boxes aren't set"); return; }
            if (Water.Checked == false && BoxList.Nodes.Count != 0)
            {
                DialogResult dlgResult = MessageBox.Show("There are unused water box settings.\n\nContinue?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dlgResult == DialogResult.No)
                {
                    return;
                }
            }
            
            float Scaling = float.Parse(ScalingValue.Text);
            if (Scaling == 0) { MessageBox.Show("Invalid Scaling!", "Error"); return; }

            FileInfo executableFileInfo = new FileInfo(Application.ExecutablePath);
            string ApplicationPath = executableFileInfo.DirectoryName + "\\obj_import.exe";
            if (File.Exists(ApplicationPath) == false) { MessageBox.Show("File obj_import.exe not found! (Must be in the same directory as this program).", "Error"); return; }

            string SettingsPath = executableFileInfo.DirectoryName + "\\settings.txt";
            StreamWriter Settings = new StreamWriter(SettingsPath);

            string ApplicationArguments = "\"" + objFile.Text + "\" \"" + RomFile.Text + "\" ";

            Settings.WriteLine("ObjFile " + objFile.Text);
            Settings.WriteLine("Scaling " + ScalingValue.Text);
            Settings.WriteLine("Level " + Convert.ToString(LevelListComboBox.SelectedIndex));
            Settings.WriteLine("Weather " + Convert.ToString(Weather.SelectedIndex));
            Settings.WriteLine("Background " +  Convert.ToString(BackGround.SelectedIndex));
            Settings.WriteLine("DeathAtBottom " + CheckBoxState(DeathAtBottom));
            Settings.WriteLine("Restore 0");
            Settings.WriteLine("Camera " + (Camera.SelectedIndex + 1).ToString());
            Settings.WriteLine("BankC " + BankC.SelectedIndex.ToString());
            Settings.WriteLine("BankD " + BankD.SelectedIndex.ToString());
            Settings.WriteLine("BankE " + BankE.SelectedIndex.ToString());
            Settings.WriteLine("Music " + Music.SelectedIndex.ToString());
            Settings.WriteLine("Bright " + Brightness.SelectedIndex.ToString());
            Settings.WriteLine("Terrain " + Terrain.SelectedIndex.ToString());
            Settings.WriteLine("NoMtl " + CheckBoxState(NoMtlCheckBox));
            Settings.WriteLine("DeathHeight " + DeathHeight.Value.ToString());
            Settings.WriteLine("Fog " + CheckBoxState(Fog));
            Settings.WriteLine("FogPreset " + FogPreset.SelectedIndex.ToString());
            Settings.WriteLine("FogR " + FogR.Value.ToString());
            Settings.WriteLine("FogG " + FogG.Value.ToString());
            Settings.WriteLine("FogB " + FogB.Value.ToString());
            Settings.WriteLine("CustomBG " + CheckBoxState(CustomBG));
            Settings.WriteLine("BGFile " + BGFile.Text);
            Settings.WriteLine("OffsetPosition " + CheckBoxState(OffsetPosition));
            Settings.WriteLine("OffsetX " + OffsetX.Value.ToString());
            Settings.WriteLine("OffsetY " + OffsetY.Value.ToString());
            Settings.WriteLine("OffsetZ " + OffsetZ.Value.ToString());
            Settings.WriteLine("Water " + CheckBoxState(Water));
            foreach (TreeNode tn in BoxList.Nodes)
            {
                Settings.WriteLine("WaterBox " + tn.Text);
            }
            Settings.WriteLine("SetMarioPos " + CheckBoxState(SetMarioPos));
            Settings.WriteLine("MarioX " + MarioX.Value.ToString());
            Settings.WriteLine("MarioY " + MarioY.Value.ToString());
            Settings.WriteLine("MarioZ " + MarioZ.Value.ToString());
            Settings.WriteLine("FlipTextures " + CheckBoxState(FlipTextures));
            //Settings.WriteLine("SwapXZ " + CheckBoxState(SwapXZ));
            //To DO: add rotateY or angely
            Settings.Close();

            if (BGFile.Text == "No file loaded!") ApplicationArguments = ApplicationArguments + " nocustombg";
            else ApplicationArguments = ApplicationArguments + " \"" + BGFile.Text + " \"";

            ApplicationArguments = ApplicationArguments + " \"" + SettingsPath + " \"";

            // save MTL if needed
            if (MtlFile.Text != "No material library associated with this .obj file" && CollisionChange == 1)
            {

                DialogResult dlgResult = MessageBox.Show("\nThere are unsaved collision settings. Save them before importing?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dlgResult == DialogResult.Yes)
                {
                    SaveMtl_Click(sender, e);
                }
            }



            //MessageBox.Show(ApplicationArguments);
            Process ProcessObj = new Process();

            ProcessObj.StartInfo.FileName = ApplicationPath;
            ProcessObj.StartInfo.Arguments = ApplicationArguments;
            ProcessObj.StartInfo.UseShellExecute = false;
            ProcessObj.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(objFile.Text);
            ProcessObj.Start();
            ProcessObj.WaitForExit();

            if (ProcessObj.ExitCode == -1) MessageBox.Show("Are you sure you're using obj_import v16 ?", "Error");
            else if (ProcessObj.ExitCode >= 0x1000)
            {
                String message = "Level successfully imported (replacing " + LevelListComboBox.SelectedItem + ")\n";

                if ((ProcessObj.ExitCode & 0x1) == 1) message = message + "\nWarning: Some vertexes were out of bounds. Try using a smaller scalling.";
                if ((ProcessObj.ExitCode & 0x10) == 0x10) message = message + "\nWarning: material file couldn't be found. Textures from the game were assigned instead.";
                if ((ProcessObj.ExitCode & 0x40) == 0x40) message = message + "\nWarning: some texture coordinates were out of range. Try dividing the \nproblematic faces (refer to obj_import.txt for more information).";
                if ((ProcessObj.ExitCode & 0x80) == 0x80) message = message + "\nWarning: ImageMagick couldn't be initialized for some reason, so textures \ncouldn't be converted. Refer to obj_import.txt for more information.";
                if ((ProcessObj.ExitCode & 0x200) == 0x200) message = message + "\nWarning: some textures in your level were bigger than the N64 texture \ncache size (4k) [32x64 at most] and have been resized to fit.\nRecommended texture size is 32x32.";
                if ((ProcessObj.ExitCode & 0x400) == 0x400) message = message + "\nWarning: your .obj file doesn't contain texture coordinates, textures won't be\n mapped correctly. Re-check your .obj exporter settings.";
                if ((ProcessObj.ExitCode & 0x800) == 0x800) message = message + "\nWarning: some textures from your material file couldn't be found.";
                if ((ProcessObj.ExitCode & 0x100) == 0x100) message = message + "\n\nTip: if your level doesn't open in TT64 due to a #texture error, \ntry importing again without textures, editing the objects in \nToad's Tool 64 and importing the level again with textures \n(object lists aren't overwritten during the import process).";

                MessageBox.Show(message);

                DialogResult dlgResult = MessageBox.Show("\nOpen conversion log?", "Debug log", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dlgResult == DialogResult.Yes)
                {

                    FileInfo TextFileInfo = new FileInfo(Application.ExecutablePath);
                    string TextFilePath = executableFileInfo.DirectoryName + "\\log.txt";

                    if (File.Exists(TextFilePath) == false) { MessageBox.Show("Debug log not found!", "Error"); return; }

                    Process TextFile = new Process();
                    TextFile.StartInfo.FileName = TextFilePath;
                    TextFile.Start();
                }


            }
            else if (ProcessObj.ExitCode == 0x01) MessageBox.Show("Error opening ROM file. Check if it isn't already being used by another program", "Error");
            else if (ProcessObj.ExitCode == 0x0102) MessageBox.Show("Invalid ROM! This program requires a special ROM: \n\n1 - Extend a clean SM64 1.0 ROM with the Mario 64 ROM Extender\n2 - Expand the ROM using rom_expand.exe\n3 - Apply the obj_import.ppf patch with PPF-Studio in the expanded ROM.", "Error");
            else if (ProcessObj.ExitCode == 0x02) MessageBox.Show("Error opening .obj file. Check if it isn't already being used by another program", "Error");
            else if (ProcessObj.ExitCode == 0x04) MessageBox.Show("No faces or vertices to process in .obj file.\n\nIf you are sure your .obj file is valid, please try \nrewriting it on a different 3D modeller.", "Error");
            else if (ProcessObj.ExitCode == 0x05) MessageBox.Show("Maximum level size exceeded. Try reducing the number of textures or avoiding the use of custom backgrounds", "Error");
            else if (ProcessObj.ExitCode == 0x06) MessageBox.Show("Your .obj file use quads or polygons with more than three verts. \nPlease triangulate the .obj file before importing.", "Error");
            else if ((ProcessObj.ExitCode & 0xFF) == 0x6)
            {
                String message = "Too many faces in .obj file. Found " + Convert.ToString((ProcessObj.ExitCode & 0xFFFF00) >> 8) + " faces when maximum is 5000";
                MessageBox.Show(message, "Error");
            }



        }

        private void RevertLevel_Click(object sender, EventArgs e)
        {
            if (RomFile.Text == "No file loaded!") { MessageBox.Show("Load a proper ROM file first!", "Error"); return; }

            Form2 form2 = new Form2(RomFile.Text, LevelListComboBox.SelectedIndex);
            form2.ShowDialog();


        }

        private void About_Click(object sender, EventArgs e)
        {
            
            MessageBox.Show("       Mario 64 Wavefront .obj Importer v16\n             by messiaen (aka frauber)\n\n             youtube.com/user/frauber\n\nThanks to VL-Tone, Cellar Dweller, Nagra and \nYoshielectron for all the wonderful SM64 docs", "About");

        }

        private void label22_Click(object sender, EventArgs e)
        {
                        
        }


        private void WaterZ_ValueChanged(object sender, EventArgs e)
        {

        }

        private void WaterHeight_ValueChanged(object sender, EventArgs e)
        {

        }

        private void AddWaterBox_Click(object sender, EventArgs e)
        {
            string WaterString = "Type = " + WaterType.SelectedIndex.ToString() + ", (" + WaterX.Value.ToString() + ", " + WaterZ.Value.ToString() + "), (" + WaterX2.Value.ToString() + ", " + WaterZ2.Value.ToString()+ "), Height = " + WaterHeight.Value.ToString() + " (" + WaterType.SelectedItem.ToString() + ")";

            foreach (TreeNode tn in BoxList.Nodes)
            {
                if (tn.Text == WaterString)
                {
                    MessageBox.Show("Duplicate entry!");
                    goto exit;
                }
            }
            BoxList.Nodes.Add(WaterString);            

        exit: ;
            
        }

        private void BoxList_AfterSelect(object sender, TreeViewEventArgs e)
        {
            string CurrentBox  = BoxList.SelectedNode.ToString().Substring(16);

            CurrentBox = CurrentBox.Replace("(", "");
            CurrentBox = CurrentBox.Replace(",", "");
            CurrentBox = CurrentBox.Replace(")", "");
            CurrentBox = CurrentBox.Replace("Height = ", "");

            object[] targets = new object[6];
            targets[0] = new Int32();
            targets[1] = new Int32();
            targets[2] = new Int32();
            targets[3] = new Int32();
            targets[4] = new Int32();
            targets[5] = new Int32();

            
            Scanner scanner = new Scanner();
            scanner.Scan(CurrentBox,
             "{0} {1} {2} {3} {4} {5}", targets);

            WaterType.SelectedIndex = Convert.ToInt32(targets[0]);
            WaterX.Value = Convert.ToInt32(targets[1]);
            WaterZ.Value = Convert.ToInt32(targets[2]);
            WaterX2.Value = Convert.ToInt32(targets[3]);
            WaterZ2.Value = Convert.ToInt32(targets[4]);
            WaterHeight.Value = Convert.ToInt32(targets[5]);
            
        }

        private void RemoveWaterBox_Click(object sender, EventArgs e)
        {
            if (BoxList.Nodes.Count != 0 && BoxList.SelectedNode != null) BoxList.Nodes.Remove(BoxList.SelectedNode);
        }

        private void m64File_TextChanged(object sender, EventArgs e)
        {

        }

        private void LoadTrajectory_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Load Trajectory file";
            dlg.Filter = "(.txt files)|*.txt";
            if (dlg.ShowDialog() == DialogResult.OK) TrajectoryFile.Text = dlg.FileName;
        }

        private void SetItemBox_CheckedChanged(object sender, EventArgs e)
        {

            if (SetItemBox.Checked == true)
            {
                OpenFileDialog dlg = new OpenFileDialog();
                dlg.Title = "Load Item Box Content file";
                dlg.Filter = "(.txt files)|*.txt";
                if (dlg.ShowDialog() == DialogResult.OK) BoxFile.Text = dlg.FileName;
            }

            if (BoxFile.Text == "No file loaded!") SetItemBox.Checked = false;
        }

        private void RestoreCRC_CheckedChanged(object sender, EventArgs e)
        {
            if (RestoreCRC.Checked == true)
            {
                DialogResult dlgResult = MessageBox.Show("Restoring the bootcode checksum check will prevent any further\nadjustments on ROM settings. Only use this option if you want to\nget rid of bootcode related error messages when releasing your hack.\n\nContinue?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dlgResult == DialogResult.No)
                {
                    RestoreCRC.Checked = false;
                }
            }

        }

        private void ApplySettings_Click(object sender, EventArgs e)
        {

            if (RomFile.Text == "No file loaded!") { MessageBox.Show("Load a proper ROM file first!", "Error"); return; }

            if (ResetActSelectors.Checked == false && ActSelectorCheck.Checked == false && StarPosition.Checked == false && SkipLakitu.Checked == false && SkipPeach.Checked == false && StartLivesCheck.Checked == false && StartLevel.Checked == false && RestoreCRC.Checked == false && KillCamera.Checked == false && SetPauseWarp.Checked == false)
            {
                MessageBox.Show("\nPlease select the settings you wish to change in the ROM.", "Nothing to do!");
                return;
            }


            int[] LevelNumber = { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
                                  0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                  0x1C, 0x1D, 0x1E, 0x1F, 0x21, 0x22, 0x24    };


            FileInfo rom = new FileInfo(MainForm.RomFileCopy);

            if (rom.Length < 0x2FFFFF0)
            {
                MessageBox.Show("Invalid ROM File!", "Error!");
                return;
            }

            FileStream fs = new FileStream(MainForm.RomFileCopy, FileMode.Open, FileAccess.ReadWrite);
            BinaryWriter bw = new BinaryWriter(fs);
            BinaryReader br = new BinaryReader(fs);

            int[] ActSelectorHack =
                {
                    0x3C, 0x01, 0x80, 0x33, 0x84, 0x21, 0xDD, 0xF8, 0x3C, 0x08, 0x80, 0x40, 0x35, 0x08, 0x2E, 0xC0, 
                    0x01, 0x01, 0x40, 0x21, 0x91, 0x08, 0x00, 0x00, 0x24, 0x01, 0x00, 0x01, 0x10, 0x28, 0x00, 0x03, 
                    0x24, 0x00, 0x00, 0x00, 0x08, 0x09, 0x2F, 0xE4, 0x00, 0x00, 0x10, 0x25, 0x08, 0x09, 0x2F, 0xD6, 
                     0x24, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 
                };


            int[] DefaultActTable =
                {
                    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x11, 
                };

            if (ActSelectorCheck.Checked == true)
            {
                bw.Seek(0x1202f00, SeekOrigin.Begin);
                int ActHack = br.ReadByte();

                if (ActHack != 0x3c)
                {
                    bw.Seek(0x1202ec0, SeekOrigin.Begin);
                    foreach (byte item in DefaultActTable)
                    {
                        bw.Write(item);
                    }
                }                

                bw.Seek(0x6F38, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x2B010001));
                
                bw.Seek(0x6F50, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x08100BC0));
                bw.Write(0x00000000);

                bw.Seek(0x1202f00, SeekOrigin.Begin);
                foreach (byte item in ActSelectorHack)
                {
                    bw.Write(item);
                }
                
                bw.Seek(0x1202ec0 + LevelNumber[ActSelector.SelectedIndex], SeekOrigin.Begin);
                if (ActOn.Checked == true) bw.Write((byte)1);
                else bw.Write((byte)0);

            }

            if (ResetActSelectors.Checked == true)
            {
                bw.Seek(0x1202ec0, SeekOrigin.Begin);
                foreach (byte item in DefaultActTable)
                {
                    bw.Write(item);
                }
            }

            if (SetPauseWarp.Checked == true)
            {
                bw.Seek(0x666A, SeekOrigin.Begin);
                bw.Write(SwapInt16((Int16)LevelNumber[PauseWarpLevel.SelectedIndex]));
                bw.Seek(0x666E, SeekOrigin.Begin);
                bw.Write(SwapInt16((Int16)PauseWarpArea.Value));
                bw.Seek(0x6672, SeekOrigin.Begin);
                bw.Write(SwapInt16((Int16)PauseWarpID.Value));
            }

            if (StartLevel.Checked == true && LevelList.SelectedIndex != -1)
            {
                // restore possible problems with earlier versions
                bw.Seek(0x6D68, SeekOrigin.Begin);
                bw.Write(SwapInt32(0xAFA5002C));
                bw.Write(SwapInt32(0xAFB00018));
                bw.Write(SwapInt32(0x3C018034));
                bw.Write(SwapInt32(0xA020B248));
                bw.Write(SwapInt32(0x3C018034));

                bw.Seek(0x6D68, SeekOrigin.Begin);
                bw.Write((byte)0x24); bw.Write((byte)0x05); bw.Write((byte)0x00);
                bw.Write(Convert.ToByte(LevelNumber[LevelList.SelectedIndex]));
                bw.Seek(0x6D78, SeekOrigin.Begin);
                bw.Write(SwapInt32(0xAFA5002C));
            }
            else if (StartLevel.Checked == true && LevelList.SelectedIndex == -1)
            {
                MessageBox.Show("Invalid LevelList.SelectedIndex value, skipping setting", "Error");
            }


            if (StartLivesCheck.Checked == true)
            {
                bw.Seek(0x1001B, SeekOrigin.Begin);
                bw.Write(Convert.ToByte(StartLives.Value));
            }

            if (SkipPeach.Checked == true)
            {
                bw.Seek(0x6BD4, SeekOrigin.Begin);
                bw.Write(SwapInt16(0x2400));
            }

            if (SkipLakitu.Checked == true)
            {
                bw.Seek(0x6D90, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x24100000));

            }

            if (RestoreCRC.Checked == true)
            {
                bw.Seek(0x66C, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x14E80006));
                bw.Seek(0x678, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x16080003));
            }

            if (KillCamera.Checked == true)
            {

                bw.Seek(0x41ADA, SeekOrigin.Begin);
                byte TestPatch = br.ReadByte();

                if (TestPatch != 0x0B)
                {
                    // captain hook
                    bw.Seek(0x41AD8, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x0C100BD4));
                    bw.Write(SwapInt32(0x8FA40040));

                    // hack
                    bw.Seek(0x1202f50, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x27BDFFE8));
                    bw.Write(SwapInt32(0xAFBF0014));
                    bw.Write(SwapInt32(0x3C018033));
                    bw.Write(SwapInt32(0x8421DDF8));
                    bw.Write(SwapInt32(0x3C088040));
                    bw.Write(SwapInt32(0x35082E50));
                    bw.Write(SwapInt32(0x01014021));
                    bw.Write(SwapInt32(0x91080000));
                    bw.Write(SwapInt32(0x24010001));
                    bw.Write(SwapInt32(0x10280003));
                    bw.Write(SwapInt32(0x24000000));
                    bw.Write(SwapInt32(0x03E00008));
                    bw.Write(SwapInt32(0x27BD0018));
                    bw.Write(SwapInt32(0x0C0A3BAC));
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x8FBF0014));
                    bw.Write(SwapInt32(0x03E00008));
                    bw.Write(SwapInt32(0x27BD0018));
       
                    // table
                    bw.Seek(0x1202e50, SeekOrigin.Begin);
                    for (int i = 0; i < 0x2C; i++ )
                    {
                        bw.Write((Byte)1);
                    }

                }

                bw.Seek(0x1202e50 + LevelNumber[CameraList.SelectedIndex], SeekOrigin.Begin);
                if (CameraOn.Checked == true) bw.Write((byte)1);
                else bw.Write((byte)0);

            }

            bw.Close();
            br.Close();
            fs.Close();

            if (RestoreCRC.Checked == true)
            {
                FileInfo executableFileInfo = new FileInfo(Application.ExecutablePath);
                string ApplicationPath = executableFileInfo.DirectoryName + "\\chksum64.exe";

                if (File.Exists(ApplicationPath) == false)
                {
                    MessageBox.Show("File chksum64.exe not found! (Must be in the same directory as this program).\nSkipping checksum fix (don't forgot to correct this later).", "Error");
                }
                else
                {
                    string ApplicationArguments = "\"" + MainForm.RomFileCopy + "\"";

                    Process ChksumFix = new Process();

                    ChksumFix.StartInfo.FileName = ApplicationPath;
                    ChksumFix.StartInfo.Arguments = ApplicationArguments;
                    ChksumFix.StartInfo.UseShellExecute = false;
                    ChksumFix.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(MainForm.RomFileCopy);
                    ChksumFix.Start();
                    ChksumFix.WaitForExit();
                }

            }

            MessageBox.Show("Settings saved to ROM");
        }

        private void ActOff_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void ResetActSelectors_CheckedChanged(object sender, EventArgs e)
        {
            if (ResetActSelectors.Checked == true) ActSelectorCheck.Checked = false;
        }

        private void ActSelectorCheck_CheckedChanged(object sender, EventArgs e)
        {
            if (ActSelectorCheck.Checked == true) ResetActSelectors.Checked = false;
        }

        private void ApplyMiscSettings_Click(object sender, EventArgs e)
        {
            if (RomFile.Text == "No file loaded!") { MessageBox.Show("Load a proper ROM file first!", "Error"); return; }

            if (SetTrajectory.Checked == false && SetItemBox.Checked == false && StarPosition.Checked == false && SetSlideTime.Checked == false)
            {
                MessageBox.Show("\nPlease select which settings to change in ROM.", "Nothing   !");
                return;
            }


            FileInfo rom = new FileInfo(MainForm.RomFileCopy);

            if (rom.Length < 0x2FFFFF0)
            {
                MessageBox.Show("Invalid ROM File!", "Error!");
                return;
            }

            FileStream fs = new FileStream(MainForm.RomFileCopy, FileMode.Open, FileAccess.ReadWrite);
            BinaryWriter bw = new BinaryWriter(fs);

            if (StarPosition.Checked == true && StarList.SelectedIndex != -1)
            {

                uint[] StarWrapperFunction =
                {
                    0x27, 0xBD, 0xFF, 0xE8, 0xAF, 0xBF, 0x00, 0x14, 0xC4, 0x8C, 0x00, 0x00, 0xC4, 0x8E, 0x00, 0x04, 
                    0x0C, 0x0B, 0xCA, 0xE2, 0x8C, 0x86, 0x00, 0x08, 0x8F, 0xBF, 0x00, 0x14, 0x27, 0xBD, 0x00, 0x18, 
                    0x03, 0xE0, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 
                } ;

                bw.Seek(0x1202700, SeekOrigin.Begin);

                foreach (byte item in StarWrapperFunction)
                {
                    bw.Write(item);
                }

                if (StarList.SelectedIndex == 0) // ktq #1
                {
                    bw.Seek(0xED868, SeekOrigin.Begin);
                    bw.Write(SwapInt16(Convert.ToInt16(StarX.Value)));
                    bw.Write(SwapInt16(Convert.ToInt16(StarY.Value)));
                    bw.Write(SwapInt16(Convert.ToInt16(StarZ.Value)));
                }
                else if (StarList.SelectedIndex == 1) // ktq #2
                {
                    bw.Seek(0xED878, SeekOrigin.Begin);
                    bw.Write(SwapInt16(Convert.ToInt16(StarX.Value)));
                    bw.Write(SwapInt16(Convert.ToInt16(StarY.Value)));
                    bw.Write(SwapInt16(Convert.ToInt16(StarZ.Value)));
                }
                else if (StarList.SelectedIndex == 2) // king bob-omb
                {
                    bw.Seek(0x1204F00, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0x62AD4, SeekOrigin.Begin);


                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F00));  // ORI A0, A0, 0x4F40
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x00000000));

                }
                else if (StarList.SelectedIndex == 3) // whomp
                {
                    bw.Seek(0x1204F10, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0x82900, SeekOrigin.Begin);

                    bw.Write(SwapInt32(0x3C018040));
                    bw.Write(SwapInt32(0xC42C4F10));
                    bw.Write(SwapInt32(0xC42E4F14));
                    bw.Write(SwapInt32(0x8C264F18));

                    bw.Seek(0x82914, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x00000000));
                }
                else if (StarList.SelectedIndex == 4)
                {
                    bw.Seek(0x1204F20, SeekOrigin.Begin);  // 0x80404F20 (s16 x,y,z triplet)
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xC9A1C, SeekOrigin.Begin); // 0x8030EA1c
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F20));  // ORI A0, A0, 0x4F20
                    bw.Write(SwapInt32(0x00000000)); // NOPs
                    bw.Write(SwapInt32(0x00000000));
                    bw.Write(SwapInt32(0x00000000));
                }
                else if (StarList.SelectedIndex == 5)
                {
                    bw.Seek(0x1204F30, SeekOrigin.Begin);  // 0x80404F30 (s16 x,y,z triplet)
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xA6970, SeekOrigin.Begin); // 0x802EB970
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F30));  // ORI A0, A0, 0x4F30
                }
                else if (StarList.SelectedIndex == 6)
                {
                    bw.Seek(0x1204F40, SeekOrigin.Begin);  // 0x80404F40
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xA6950, SeekOrigin.Begin); // 0x802EB950
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F40));  // ORI A0, A0, 0x4F40
                }
                else if (StarList.SelectedIndex == 7)
                {
                    bw.Seek(0x1204F50, SeekOrigin.Begin);  // 0x80404F50
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xC802C, SeekOrigin.Begin); //  0x8030D02C
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F50));  // ORI A0, A0, 0x4F50
                }
                else if (StarList.SelectedIndex == 8)
                {
                    bw.Seek(0x1204F60, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0x605E4, SeekOrigin.Begin); //  0x802a55e4
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F60));
                }
                else if (StarList.SelectedIndex == 9)
                {
                    bw.Seek(0x1204F70, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xBCFE0, SeekOrigin.Begin); //  80301FE0
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F70));

                }
                else if (StarList.SelectedIndex == 10) // peach slide
                {
                    bw.Seek(0x1204F80, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));


                    bw.Seek(0xB7D0, SeekOrigin.Begin); //  80301FE0
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F80));

                }
                else if (StarList.SelectedIndex == 11) // BIG PENGUIN RACE
                {
                    bw.Seek(0x1204F90, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));


                    bw.Seek(0x605E4, SeekOrigin.Begin); //  80301FE0
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844F90));

                }
                else if (StarList.SelectedIndex == 12) // treasure chests
                {
                    bw.Seek(0x1204FA0, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));

                    bw.Seek(0xB32B0, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FA0));

                }
                else if (StarList.SelectedIndex == 13)
                {
                    bw.Seek(0x1204FAC, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0x7FBB0, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FAC));
                }
                else if (StarList.SelectedIndex == 14)
                {
                    bw.Seek(0x1204FB8, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0xCC47C, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FB8));
                }
                else if (StarList.SelectedIndex == 15)
                {
                    bw.Seek(0x1204FC4, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0x7FC24, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FC4));
                }
                else if (StarList.SelectedIndex == 16)
                {
                    bw.Seek(0x1204FD0, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0x61450, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FD0));
                }
                else if (StarList.SelectedIndex == 17)
                {
                    bw.Seek(0x1204FD8, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0x7FBEC, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FD8));
                }
                else if (StarList.SelectedIndex == 18)
                {
                    bw.Seek(0x1204FE4, SeekOrigin.Begin);
                    bw.Write(SwapFloat32(Convert.ToSingle(StarX.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarY.Value)));
                    bw.Write(SwapFloat32(Convert.ToSingle(StarZ.Value)));
                    bw.Seek(0xA6CBC, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040));  // LUI A0, 0x8040
                    bw.Write(SwapInt32(0x0C1009C0));  // JAL 0x80402700
                    bw.Write(SwapInt32(0x34844FE4));
                }











            }
            else if (StarPosition.Checked == true && StarList.SelectedIndex == -1)
            {
                MessageBox.Show("Invalid StarPosition.SelectedIndex value, skipping setting", "Error");
            }

            if (SetSlideTime.Checked == true)
            {
                bw.Seek(0xB7A6, SeekOrigin.Begin);
                bw.Write(SwapInt16( (Int16)(SlideTime.Value*30) ));
            }

            if (SetTrajectory.Checked == true && TrajectoryList.SelectedIndex != -1)
            {

                if (TrajectoryFile.Text == "No file loaded!")
                {
                    MessageBox.Show("Load a trajectory file first! Skipping setting.", "Error");
                    goto OutOfTrajectory;
                }

                if (TrajectoryList.SelectedIndex == 0)
                {
                    bw.Seek(0xED864, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x80405000));
                    bw.Seek(0x1205000, SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 1)
                {
                    bw.Seek(0xED874, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x80405500));
                    bw.Seek(0x1205500, SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 2) // racing penguin
                {
                    bw.Seek(0xCCA6E, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x8040));
                    bw.Seek(0xCCA76, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x5A00));
                    bw.Seek(0x1205a00, SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 3) // snowman's bottom
                {
                    bw.Seek(0xABC9E, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x8040));
                    bw.Seek(0xABCA6, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x5F00));
                    bw.Seek(0x1205F00, SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex > 3 && TrajectoryList.SelectedIndex <= 12)
                {

                    bw.Seek(0xED9DC + ((TrajectoryList.SelectedIndex-4) *4), SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x8040));
                    bw.Write(SwapUInt16((ushort)(0x5000 + (0x500 * TrajectoryList.SelectedIndex))));
                    
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 13)
                {
                    bw.Seek(0xA9AB4, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040)); // LUI A0, 0x8040
                    bw.Seek(0xA9ABC, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x3484)); // ORI A0, A0, 0x8040xxxx
                    bw.Write(SwapUInt16((ushort)(0x5000 +(0x500 * TrajectoryList.SelectedIndex)))); 
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 14)
                {
                    bw.Seek(0xA9AD4, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040)); // LUI A0, 0x8040
                    bw.Seek(0xA9ADC, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x3484)); // ORI A0, A0, 0x8040xxxx
                    bw.Write(SwapUInt16((ushort)(0x5000 +(0x500 * TrajectoryList.SelectedIndex)))); 
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 15)
                {
                    bw.Seek(0xA9AF4, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C048040)); // LUI A0, 0x8040
                    bw.Seek(0xA9AFC, SeekOrigin.Begin);
                    bw.Write(SwapUInt16(0x3484)); // ORI A0, A0, 0x8040xxxx
                    bw.Write(SwapUInt16((ushort)(0x5000 + (0x500 * TrajectoryList.SelectedIndex))));
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 16)
                {
                    bw.Seek(0xA9B1C, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C098040)); // LUI T1, 0x8040
                    bw.Write(SwapUInt16(0x3529)); // ORI T1, T1, 0x8040xxxx
                    bw.Write(SwapUInt16((ushort)(0x5000 + (0x500 * TrajectoryList.SelectedIndex))));
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }
                else if (TrajectoryList.SelectedIndex == 16)
                {
                    bw.Seek(0xA9B38, SeekOrigin.Begin);
                    bw.Write(SwapInt32(0x3C0B8040)); // LU3 T3, 0x8040
                    bw.Write(SwapUInt16(0x356B)); // ORI T3, T3, 0x8040xxxx
                    bw.Write(SwapUInt16((ushort)(0x5000 + (0x500 * TrajectoryList.SelectedIndex))));
                    bw.Seek(0x1205000 + (0x500 * TrajectoryList.SelectedIndex), SeekOrigin.Begin);
                }

                /* Write trajectory */
                StreamReader streamReader = new StreamReader(TrajectoryFile.Text);
                string line = streamReader.ReadLine();
                Int16 entry = 0;

                while (line != null)
                {
                    var parts = line.Split(',');
                    int x = Convert.ToInt16(parts[0]);
                    int y = Convert.ToInt16(parts[1]);
                    int z = Convert.ToInt16(parts[2]);

                    bw.Write(SwapInt16(entry));
                    bw.Write(SwapInt16(Convert.ToInt16(x)));
                    bw.Write(SwapInt16(Convert.ToInt16(y)));
                    bw.Write(SwapInt16(Convert.ToInt16(z)));

                    entry++;
                    line = streamReader.ReadLine();
                }

                streamReader.Close();
                bw.Write(SwapInt32(0xFFFFFFFF));
                bw.Write(SwapInt32(0xFFFFFFFF));

            }
            else if (SetTrajectory.Checked == true && TrajectoryList.SelectedIndex == -1)
            {
                MessageBox.Show("Invalid TrajectoryList.SelectedIndex value, skipping setting", "Error");
            }
        OutOfTrajectory: ;

            if (SetItemBox.Checked == true)
            {
                /* To Do: rewrite this! */ 
                
                StreamReader streamReader = new StreamReader(BoxFile.Text);
                string boxfile = streamReader.ReadToEnd();
                streamReader.Close();
                char[] delimiterChars = { ',', '\n' };
                string[] itembox = boxfile.Split(delimiterChars);

                bw.Seek(0x1204000, SeekOrigin.Begin);

                byte entry = 0;
                int control = 0;
                foreach (string s in itembox)
                {
                    try
                    {
                        int number = Int32.Parse(s, System.Globalization.NumberStyles.HexNumber);
                        if (control == 0) bw.Write(entry);

                        if (control == 1)
                        {
                            UInt16 BParam = (UInt16)SwapInt16(Convert.ToInt16(number));
                            bw.Write(BParam);

                        }
                        if (control == 2)
                        {
                            byte ModelID = Convert.ToByte(number);
                            bw.Write(ModelID);
                        }
                        if (control == 3)
                        {
                            UInt32 Behavior = SwapInt32(Convert.ToUInt32(number));
                            bw.Write(Behavior);
                        }

                        control++;

                        if (control == 4)
                        {
                            entry++;
                            if (entry == 0x63) entry = 0x64;
                            control = 0;
                        }

                    }
                    catch
                    {
                        String message = "Invalid value: " + s + "\n\nOnly 32-bit hexadecimal values are accepted \nAll values within an entry must be separated by commas (,)";
                        MessageBox.Show(message, "Error");
                    }
                }

                bw.Write(SwapInt32(0x63000000)); // finish
                bw.Write(SwapInt32(0x00000000));

                //write new pointer for itembox list
                bw.Seek(0x7C8E2, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x8040));
                bw.Seek(0x7C8E6, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x4000));

            }

            bw.Close();
            fs.Close();

            MessageBox.Show("Settings saved to ROM");


        }

        private void tabPage7_Click(object sender, EventArgs e)
        {
            
        }

        private void TextureList_SelectedIndexChanged(object sender, EventArgs e)
        {
            CollisionType.SelectedIndex = CollisionList[TextureList.SelectedIndex];
            FileInfo mtlInfo = new FileInfo(MtlFile.Text);
            string TexturePath = mtlInfo.DirectoryName + "\\" + TextureList.SelectedItem.ToString();

            if (File.Exists(TexturePath) == false)
            {
                TextureSizeLabel.Text = "Texture not found!";
                TexturePic.Image = null;
                return;
            }
            Image texture = Image.FromFile(TexturePath);

            if (texture.Size.Width * texture.Size.Height > 2048)
            {
                TextureTooBig.Visible = true;

                int width = texture.Size.Width;
                int height = texture.Size.Height;
                while ((width * height * 2) > 4096)   // divide by 2 while result fits doesn't fit texture cache
                {
                    width = width / 2;
                    height = height / 2;
                }

                if ( (width % 2) == 1) width -= 1;   // make it an even number
                if ( (height % 2) == 1) height -= 1;

                if (width == 1) width = 2;  // minimum size
                if (height == 1) width = 2;

                Bitmap resized = new Bitmap(texture, width*2, height*2);
                TexturePic.ClientSize = new Size(width*2,height*2);
                TexturePic.Image = (Image)resized;
                TextureSizeLabel.Text = width.ToString() + "x" + height.ToString() + " (originally " + texture.Size.Width + "x" +texture.Size.Height + ")";

            }
            else
            {
                Bitmap resized = new Bitmap(texture, texture.Size.Width * 2, texture.Size.Height * 2);
                TexturePic.ClientSize = new Size(texture.Size.Width*2, texture.Size.Height*2);
                TextureTooBig.Visible = false;
                TexturePic.Image = (Image)resized;
                TextureSizeLabel.Text = texture.Size.Width + "x" + texture.Size.Height;
            }

        }

        private void CollisionType_SelectedIndexChanged(object sender, EventArgs e)
        {
            CollisionChange = 1;

            SaveMtl.ForeColor = Color.Black;
           
            CollisionList[TextureList.SelectedIndex] = CollisionType.SelectedIndex;
            CParam1.Value = ColParam1[TextureList.SelectedIndex];
            CParam2.Value = ColParam2[TextureList.SelectedIndex];

            if (CTypeList[CollisionType.SelectedIndex] == 0x04 || CTypeList[CollisionType.SelectedIndex] == 0x0E || CTypeList[CollisionType.SelectedIndex] == 0x24 || CTypeList[CollisionType.SelectedIndex] == 0x25 || CTypeList[CollisionType.SelectedIndex] == 0x27 || CTypeList[CollisionType.SelectedIndex] == 0x2C || CTypeList[CollisionType.SelectedIndex] == 0x2D)
            {
                CParam1.Enabled = true;
                CParam2.Enabled = true;
                DebugCollision.Visible = true;
                if (CTypeList[CollisionType.SelectedIndex] == 0x2C || CTypeList[CollisionType.SelectedIndex] == 0x2D) DebugCollision.Text = "(Wind direction)";
                else if (CTypeList[CollisionType.SelectedIndex] == 0x0E) DebugCollision.Text = "(Water torrent direction/force)";
                else if (CTypeList[CollisionType.SelectedIndex] == 0x24 || CTypeList[CollisionType.SelectedIndex] == 0x25 || CTypeList[CollisionType.SelectedIndex] == 0x27) DebugCollision.Text = "(Moving sand direction/force)";
            }
            else
            {
                CParam1.Enabled = false;
                CParam2.Enabled = false;
                DebugCollision.Visible = false;
            }
            //DebugCollision.Text = "Entry: " + CollisionType.SelectedIndex.ToString() + "  Value:" + CTypeList[CollisionType.SelectedIndex].ToString("x");
            //DebugCollision.Visible = true;

        }

        private void MtlFile_TextChanged(object sender, EventArgs e)
        {
            if (MtlFile.Text == "No material library associated with this .obj file")
            {
                CollisionType.Enabled = false;
                TextureList.Enabled = false;
            }
            else if (File.Exists(MtlFile.Text))
            {
                for (int i =0; i < 499; i++)
                {
                    CollisionList[i] = 0;
                    ColParam1[i] = 0;
                    ColParam2[i] = 0;
                }


                CollisionChange = 0;
                TextureList.Items.Clear();
                StreamReader sr = new StreamReader(MtlFile.Text);
                string line = sr.ReadLine();

                while (line != null)
                {
                    line = TrimString(line);

                    if (line.StartsWith("#"))
                    {
                        // ignore
                    }
                    else if (line.StartsWith("map_Kd") || line.StartsWith("map_Ka") || line.StartsWith("map_kd") || line.StartsWith("map_ka"))
                    {
                        string TextureFile = line.Substring(7);
                        TextureList.Items.Add(TextureFile);
                        TextureList.SelectedIndex = 0;
                        CollisionList[TextureList.Items.Count] = 0;
                    }
                    else if (line.StartsWith("mapkd") || line.StartsWith("mapKd") || line.StartsWith("mapka") || line.StartsWith("mapKa"))
                    {
                        string TextureFile = line.Substring(6);
                        TextureList.Items.Add(TextureFile);
                        TextureList.SelectedIndex = 0;
                        CollisionList[TextureList.Items.Count] = 0;
                    }
                    else if (line.StartsWith("collisionType"))
                    {
                        // To DO: Adjust this 

                        int ColType = 0;
                        // int ClParam1 = 0; (No idea what's this)
                        // int ClParam2 = 0;

                        try
                        {
                            object[] targets = new object[3];
                            targets[0] = new Int32();
                            targets[1] = new Int32();
                            targets[2] = new Int32();

                            Scanner scanner = new Scanner();
                            scanner.Scan(line.Substring(13),
                             "{0} {1} {2}", targets);

                            ColType = Convert.ToInt32(targets[0]);
                            ColParam1[TextureList.Items.Count - 1] = Convert.ToInt32(targets[1]);
                            ColParam2[TextureList.Items.Count - 1] = Convert.ToInt32(targets[2]);
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show(ex.ToString());
                        }
                        finally
                        {

                            object[] targets = new object[1];
                            targets[0] = new Int32();

                            Scanner scanner = new Scanner();
                            scanner.Scan(line.Substring(13),
                             "{0}", targets);

                            ColType = Convert.ToInt32(targets[0]);

                        }


                        int index = 0;
                        foreach (int type in CTypeList)
                        {
                            if (type == ColType) CollisionList[TextureList.Items.Count -1] = index;

                            index++;

                        }

                    }
                    line = sr.ReadLine();

                }
                sr.Close();

                if (TextureList.Items.Count != 0)
                {
                    TextureList.Enabled = true;
                    CollisionType.Enabled = true;
                    TextureList.SelectedIndex = 0;
                    CollisionType.SelectedIndex = CollisionList[TextureList.SelectedIndex];
                    CParam1.Value = ColParam1[TextureList.SelectedIndex];
                    CParam2.Value = ColParam2[TextureList.SelectedIndex];
                }
            }
            CollisionChange = 0;
            SaveMtl.ForeColor = Color.Gray;
        }

        private void TextureSizeLabel_Click(object sender, EventArgs e)
        {

        }

        private void SaveMtl_Click(object sender, EventArgs e)
        {

            if (CollisionChange == 0)
            {
                MessageBox.Show("Nothing to do!", "Error");
                return;
            }
            StreamReader sr = new StreamReader(MtlFile.Text);
            StringWriter mtl = new StringWriter();
            
            string line = sr.ReadLine();

            while (line != null)
            {
                line = TrimString(line);
                if (line.StartsWith("map_Kd") || line.StartsWith("map_Ka") || line.StartsWith("map_kd") || line.StartsWith("map_ka") || line.StartsWith("mapKd") || line.StartsWith("mapkd"))
                {
                    string TextureFile = "bogus";

                    if (line.StartsWith("mapKd") || line.StartsWith("mapkd"))
                    {
                        TextureFile = line.Substring(6);
                    }
                    else TextureFile = line.Substring(7);

                    mtl.WriteLine(line);

                    if (TextureList.Items.Count != 0)
                    {
                        for (int k = 0; k < TextureList.Items.Count; k++)
                        {
                            if (TextureFile == TextureList.Items[k].ToString())
                            {
                                line = sr.ReadLine();
                                line = TrimString(line);

                                if (line != null && line.StartsWith("collisionType"))
                                {
                                    mtl.WriteLine("collisionType " + CTypeList[CollisionList[k]].ToString() + " " + ColParam1[k].ToString() + " " + ColParam2[k].ToString());
                                }
                                else
                                {
                                    mtl.WriteLine("collisionType " + CTypeList[CollisionList[k]].ToString() + " " + ColParam1[k].ToString() + " " + ColParam2[k].ToString());
                                    mtl.WriteLine(line);

                                }

                            }
                        }
                    }
                    
                }
                else mtl.WriteLine(line);

                line = sr.ReadLine();

            }
            sr.Close();
            mtl.Close();

            StreamWriter sr2 = new StreamWriter(MtlFile.Text);

            sr2.Write(mtl.ToString()); // Write to mtl file
            sr2.Close();
            MessageBox.Show("Material settings saved!");
            CollisionChange = 0;
            SaveMtl.ForeColor = Color.Gray;
            
        }

        private void ActSelector_SelectedIndexChanged(object sender, EventArgs e)
        {

            int[] LevelNumber = { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
                                  0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                  0x1C, 0x1D, 0x1E, 0x1F, 0x21, 0x22, 0x24    };

            int a = ActSelector.SelectedIndex;

            if (ActOn.Checked == true && (a == 2 || a == 12 || a == 14 || a == 16 || a == 21 || a == 23 || a == 27))
            {
                label37.Text = "Warning: enabling act selectors might not work on this level = " + ActSelector.SelectedIndex.ToString() + " (#" + LevelNumber[ActSelector.SelectedIndex].ToString() + ")";
            }

        }

        private void ActOn_CheckedChanged(object sender, EventArgs e)
        {

            int[] LevelNumber = { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
                                  0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                  0x1C, 0x1D, 0x1E, 0x1F, 0x21, 0x22, 0x24    };

            int a = ActSelector.SelectedIndex;

            if (ActOn.Checked == true && ( a == 2 || a == 12 || a == 14 || a == 16 || a ==21 || a == 23 || a == 27 ))
            {
                label37.Text = "Warning: enabling act selectors might not work on this level = " + ActSelector.SelectedIndex.ToString() + " (#" + LevelNumber[ActSelector.SelectedIndex].ToString() + ")";
            }
            
            
        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {

            CurrentTab = tabControl1.SelectedIndex;

            if (CurrentTab == 4 && objFile.Text == "No file loaded!")
            {
                MessageBox.Show("Load an Wavefront .obj file first!");
                tabControl1.SelectedIndex = PreviousTab;
                
            }

            if ( ( CurrentTab == 1 || CurrentTab == 5 || CurrentTab == 6 || CurrentTab == 7) && (RomFile.Text == "No file loaded!") )
            {
                MessageBox.Show("Load a ROM file first!");
                tabControl1.SelectedIndex = PreviousTab;
            }
            
            PreviousTab = tabControl1.SelectedIndex;
        }

        private void TrajectoryList_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void DebugCollision_Click(object sender, EventArgs e)
        {
        
        }

        private void objFile_TextChanged(object sender, EventArgs e)
        {

            if (objFile.Text != "No file loaded!")
            {
                CurrentCollisionList.Clear(); // clean collision parameters
                StreamReader sr = new StreamReader(objFile.Text);
                string line = sr.ReadLine();

                FoundMtl = 0;
                while (line != null)
                {
                    line = TrimString(line);
                    if (line.StartsWith("#"))
                    {
                        // ignore
                    }
                    else if (line.StartsWith("mtllib"))
                    {
                        FoundMtl = 1;
                        string mtllib = line.Substring(7);
                        FileInfo mtlInfo = new FileInfo(objFile.Text);

                        if (mtllib.StartsWith("\\") || mtllib.StartsWith("C:") || mtllib.StartsWith("c:") || mtllib.StartsWith("D:") || mtllib.StartsWith("d:") || mtllib.StartsWith("E:") || mtllib.StartsWith("e:") || mtllib.StartsWith("F:") || mtllib.StartsWith("f:") || mtllib.StartsWith("G:") || mtllib.StartsWith("g:") || mtllib.StartsWith("H:") || mtllib.StartsWith("h:") || mtllib.StartsWith("I:") || mtllib.StartsWith("i:"))
                        {
                            MtlFile.Text = mtllib;
                        }
                        else
                        {
                            string MtlPath = mtlInfo.DirectoryName + "\\" + mtllib;
                            MtlFile.Text = MtlPath;
                        }


                    }
                    line = sr.ReadLine();

                    if (FoundMtl == 0) MtlFile.Text = "No material library associated with this .obj file";

                }
                sr.Close();

            }
        }

        private void tabPage1_Click(object sender, EventArgs e)
        {

        }

        private void SetMarioPos_CheckedChanged(object sender, EventArgs e)
        {
            if (SetMarioPos.Checked == true)
            {
                MarioX.Enabled = true;
                MarioY.Enabled = true;
                MarioZ.Enabled = true;
                label28.Visible = true;
                
            }
            else if (SetMarioPos.Checked == false)
            {
                MarioX.Enabled = false;
                MarioY.Enabled = false;
                MarioZ.Enabled = false;
                label28.Visible = false;
            }
        }

        private void tabPage4_Click(object sender, EventArgs e)
        {

        }

        private void CParam1_ValueChanged(object sender, EventArgs e)
        {
            ColParam1[TextureList.SelectedIndex] = Convert.ToInt32(CParam1.Value);
            CollisionChange = 1;
        }

        private void CParam2_ValueChanged(object sender, EventArgs e)
        {
            ColParam2[TextureList.SelectedIndex] = Convert.ToInt32(CParam2.Value);
            CollisionChange = 1;
        }

        private void StarPosition_CheckedChanged(object sender, EventArgs e)
        {
            if (StarPosition.Checked == true)
            {
                StarX.Enabled = true;
                StarY.Enabled = true;
                StarZ.Enabled = true;
            }
            else if (StarPosition.Checked == false)
            {
                StarX.Enabled = false;
                StarY.Enabled = false;
                StarZ.Enabled = false;
            }
        }

        private void SlideTime_ValueChanged(object sender, EventArgs e)
        {
            if (SlideTime.Value == 1) label44.Text = "second";
            else label44.Text = "seconds";
        }

        private void SetSlideTime_CheckedChanged(object sender, EventArgs e)
        {
            if (SetSlideTime.Checked == true) SlideTime.Enabled = true;
            else if (SetSlideTime.Checked == false) SlideTime.Enabled = false;
        }

        private void RomFile_TextChanged(object sender, EventArgs e)
        {

            if (RomFile.Text != "No file loaded!")
            {
                if (CheckROM() == false)
                {
                    MessageBox.Show("                                    Invalid ROM File!\n\nRefer to obj_import.txt for details on preparing your ROM. ", "Error");
                    RomFile.Text = "No file loaded!";
                }
                else
                {
                    ReadMusicData();
                    WritePatches();
                }

            }



        }

        private void InsertSequence_Click(object sender, EventArgs e)
        {
            object[] a = new object[CurrentMusic.Items.Count];
            CurrentMusic.Items.CopyTo(a, 0);

            CurrentMusicCopy = CurrentMusic.SelectedIndex;

            SequenceForm SeqForm = new SequenceForm(a, RomFile.Text);
            SeqForm.ShowDialog();

            if (MusicStatus == 1)
            {
                ReadMusicData(); // update Music Stuff
                MusicStatus = 0;
            }

        }

        private void InstSet_SelectedIndexChanged(object sender, EventArgs e)
        {

            foreach (NInst inst in CurrentNInstList)
            {
                if (CurrentMusic.SelectedIndex == inst.SequenceID)
                {
                    inst.NInsts[0] = (byte)InstSet.SelectedIndex;
                }
            }

            

            InstList.Items.Clear();
            if (InstSet.SelectedIndex == 0)
            {
                AddBlankInstruments(6);
            }
            else if (InstSet.SelectedIndex == 1)
            {
                AddBlankInstruments(9);
            }
            else if (InstSet.SelectedIndex == 2)
            {
                AddBlankInstruments(3);
            }
            else if (InstSet.SelectedIndex == 3)
            {
                AddBlankInstruments(0x0a);
            }
            else if (InstSet.SelectedIndex == 4)
            {
                AddBlankInstruments(0x10);
            }
            else if (InstSet.SelectedIndex == 5)
            {
                AddBlankInstruments(0x10);
            }
            else if (InstSet.SelectedIndex == 6)
            {
                AddBlankInstruments(0x10);
            }
            else if (InstSet.SelectedIndex == 7)
            {
                AddBlankInstruments(0x0f);
            }
            else if (InstSet.SelectedIndex == 8)
            {
                AddBlankInstruments(0x1b);
            }
            else if (InstSet.SelectedIndex == 9)
            {
                AddBlankInstruments(7);
            }
            else if (InstSet.SelectedIndex == 10)
            {
                AddBlankInstruments(0x18);
            }
            else if (InstSet.SelectedIndex == 11)
            {
                AddInstrument("00 - Acoustic Guitar");
                AddInstrument("01 - Accordeon");
                AddInstrument("02 - Ac??");
                AddInstrument("03 - Ac??");
                AddInstrument("04 - Ac??");
                AddInstrument("05 - Ac??");
                AddInstrument("06 - Low Accordeon");
                AddInstrument("07 - 07?");
                AddInstrument("08 - ??");
                AddInstrument("09 - ??");
                AddInstrument("10 - Crash Cymbal");
                AddInstrument("11 - Cymbal 2(?)");
            }
            else if (InstSet.SelectedIndex == 12)
            {
                AddInstrument("00 - ");
                AddInstrument("01 - ");
                AddInstrument("02 - ");
                AddInstrument("03 - ");
                AddInstrument("04 - ");
                AddInstrument("05 - ");
                AddInstrument("06 - Low instrument?");
                AddInstrument("07 - Recorder");
                AddInstrument("08 - ");
                AddInstrument("09 - ");
                AddInstrument("10 - ");
                AddInstrument("11 - Clash Cymbal");
                AddInstrument("12 - Snare Drum");
                AddInstrument("13 - Triangle");
                AddInstrument("14 - Triangle");
                AddInstrument("15 - Sine wave-like sound");
            }
            else if (InstSet.SelectedIndex == 13)
            {
                AddInstrument("00 - Banjo");
                AddInstrument("01 - Test me!");
                AddInstrument("02 - Fiddle");
                AddInstrument("03 - Test me!");
                AddInstrument("04 - Whistle with vibrato");
                AddInstrument("05 - Test me!");
                AddInstrument("06 - Acoustic Bass");
                AddInstrument("07 - Acoustic Bass");
                AddInstrument("08 - Acoustic Bass");
                AddInstrument("09 - Acoustic Bass");
                AddInstrument("10 - Test me!");
                AddInstrument("11 - Test me!");
                AddInstrument("12 - Test me!");
            }
            else if (InstSet.SelectedIndex == 14)
            {
                AddInstrument("00 - Strings");
                AddInstrument("01 - ?");
                AddInstrument("02 - ?");
                AddInstrument("03 - Pizzicato String");
                AddInstrument("04 - Cello (or low woodwind?)");
                AddInstrument("05 - Eletric Piano");
                AddInstrument("06 - ?");
            }
            else if (InstSet.SelectedIndex == 15)
            {
                AddInstrument("00 - Percussion loop");
                AddInstrument("01 - Voice 'Uhs'");
                AddInstrument("02 - Sitar drone notes");
                AddInstrument("03 - Sitar");
            }
            else if (InstSet.SelectedIndex == 16)
            {
                AddInstrument("00 - Low drone-like haunted sound");
                AddInstrument("01 - ");
                AddInstrument("02 - Low Cowbell");
                AddInstrument("03 - ");
                AddInstrument("04 - High Cowbell");
                AddInstrument("05 - ");
                AddInstrument("06 - ");
            }
            else if (InstSet.SelectedIndex == 17)
            {
                AddInstrument("00 - Snare Drum");
                AddInstrument("01 - Fingered Bass");
                AddInstrument("02 - Fingered Bass");
                AddInstrument("03 - Organ");
                AddInstrument("04 - Steel Drum");
                AddInstrument("05 - Trumpet");
                AddInstrument("06 - Slap Bass");
                AddInstrument("07 - Synth");
                AddInstrument("08 - Clavinet");
                AddInstrument("09 - Clavinet");
                AddInstrument("10 - Drum Sample (hi-hat)");
                AddInstrument("11 - Drum Sample");
                AddInstrument("12 - Drum Sample");
                AddInstrument("13 - Drum Sample");
            }
            else if (InstSet.SelectedIndex == 18)
            {
                AddBlankInstruments(0x0c);
            }
            else if (InstSet.SelectedIndex == 19)
            {
                AddBlankInstruments(0x10);
            }
            else if (InstSet.SelectedIndex == 20)
            {
                AddBlankInstruments(5);
            }
            else if (InstSet.SelectedIndex == 21)
            {
                AddBlankInstruments(0x0a);
            }
            else if (InstSet.SelectedIndex == 22)
            {
                AddBlankInstruments(2);
            }
            else if (InstSet.SelectedIndex == 23)
            {
                AddInstrument("00 - Acoustic bass");
                AddInstrument("01 - Banjo");
                AddInstrument("02 - Strings");
                AddInstrument("03 - Strings");
                AddInstrument("04 - Electric piano");
                AddInstrument("05 - Electric piano");
                AddInstrument("06 - Electric piano");
                AddInstrument("07 - Electric piano");
                AddInstrument("08 - Electric piano");
                AddInstrument("09 - Electric piano");
                AddInstrument("10 - Clash cymbal");
                AddInstrument("11 - Cymbal");
                AddInstrument("12 - Cowbell");
            }
            else if (InstSet.SelectedIndex == 24) // metal cap
            {
                AddInstrument("00 - Organ");
                AddInstrument("01 - Synth bass");
                AddInstrument("02 - Saw wave");
                AddInstrument("03 - Orchestra hit");
                AddInstrument("04 - Orchestra hit");
                AddInstrument("05 - Orchestra hit");
                AddInstrument("06 - Orchestra hit");
                AddInstrument("07 - Orchestra hit");
                AddInstrument("08 - Orchestra hit");
                AddInstrument("09 - Orchestra hit");
                AddInstrument("10 - Cymbal");
                AddInstrument("11 - Cymbal");
                AddInstrument("12 - Cowbell");
                AddInstrument("13 - Orchestral percussion");
                AddInstrument("14 - TR-808 Tom");
            }
            else if (InstSet.SelectedIndex == 25)
            {
                AddBlankInstruments(0x0d);
            }
            else if (InstSet.SelectedIndex == 26)
            {
                AddBlankInstruments(0x0d);
            }
            else if (InstSet.SelectedIndex == 27)
            {
                AddBlankInstruments(0x0c);
            }
            else if (InstSet.SelectedIndex == 28)
            {
                AddBlankInstruments(7);
            }
            else if (InstSet.SelectedIndex == 29)
            {
                AddBlankInstruments(6);
            }
            else if (InstSet.SelectedIndex == 30)
            {
                AddBlankInstruments(4);
            }
            else if (InstSet.SelectedIndex == 31)
            {
                AddBlankInstruments(0x0d);
            }
            else if (InstSet.SelectedIndex == 32)
            {
                AddBlankInstruments(9);
            }
            else if (InstSet.SelectedIndex == 33)
            {
                AddBlankInstruments(4);
            }
            else if (InstSet.SelectedIndex == 34)
            {
                AddBlankInstruments(0x0c);
            }
            else if (InstSet.SelectedIndex == 35)
            {
                AddBlankInstruments(0x08);
            }
            else if (InstSet.SelectedIndex == 36)
            {
                AddBlankInstruments(0x0a);
            }
            else if (InstSet.SelectedIndex == 37)
            {
                AddInstrument("00: Accordion");
                AddInstrument("01: Slap bass");
                AddInstrument("02: Steel guitar?");
                AddInstrument("03: Electric piano");
                AddInstrument("04: Pizzicato string");
                AddInstrument("05: French horn");
                AddInstrument("06: Pan flute");
                AddInstrument("07: Strings");
                AddInstrument("08: Electric piano");
                AddInstrument("09: Electric piano");
                AddInstrument("0A: Cymbal");
                AddInstrument("0B: Cymbal");
                AddInstrument("0C: Cowbell");
                AddInstrument("0D: Cowbell");
                AddInstrument("0E: Accordion");
                AddInstrument("0F: Pizzicato string");
            }

            InstList.SelectedIndex = 0;

            
        }

        private void tabPage2_Click(object sender, EventArgs e)
        {

        }

        private void CurrentMusic_SelectedIndexChanged(object sender, EventArgs e)
        {
             MusicName.Text = CurrentMusic.Text;

            if (CurrentMusic.SelectedIndex == 0 || CurrentMusic.SelectedIndex == 0x0A) 
            {
                InstSet.Enabled = false;
                InstList.Enabled = false;
                RenameSequence.Enabled = false;
                MusicName.Enabled = false;
                SequenceNumber.Text = ("Number: " + CurrentMusic.SelectedIndex.ToString() + " (Hex = 0x" + CurrentMusic.SelectedIndex.ToString("X") + ")");

                int counter = 0;
                foreach (Sequence seq in CurrentSequenceList)
                {
                    if (CurrentMusic.SelectedIndex == counter)
                    {
                        SequencePointerLabel.Text = "Pointer: ";
                        SequencePointerLabel.Text += seq.StartOffset.ToString("X6");
                        SequenceSizeLabel.Text = "Size: ";
                        SequenceSizeLabel.Text += seq.Lenght.ToString("D");
                        SequenceSizeLabel.Text += " bytes";
                    }
                    counter++;
                }

                foreach (NInst inst in CurrentNInstList)
                {
                    if (CurrentMusic.SelectedIndex == inst.SequenceID)
                    {
                        InstSet.SelectedIndex = inst.NInsts[0];
                        InstDataLabel.Text = "InstData: ";
                        InstDataLabel.Text += inst.NInstCount.ToString("X2");
                        InstDataLabel.Text += " ";

                        int count = 0;
                        bool doOnce = true;
                        foreach (byte b in inst.NInsts)
                        {
                            InstDataLabel.Text += inst.NInsts[count].ToString("X2");
                            InstDataLabel.Text += " ";
                            count += 1;
                            if (count > 3)
                            {
                                if (doOnce) InstDataLabel.Text += "\n";
                                doOnce = false;
                            }
                        }
                    }
                }

            }
            else
            {
                MusicName.Enabled = true;
                RenameSequence.Enabled = true;
                InstSet.Enabled = true;
                InstList.Enabled = true;
                SequenceNumber.Text = ("Number: " + CurrentMusic.SelectedIndex.ToString() + " (Hex = 0x" + CurrentMusic.SelectedIndex.ToString("X") + ")");

                int counter = 0;
                foreach (Sequence seq in CurrentSequenceList)
                {
                    if (CurrentMusic.SelectedIndex == counter)
                    {
                        SequencePointerLabel.Text = "Pointer: ";
                        SequencePointerLabel.Text += seq.StartOffset.ToString("X6");
                        SequenceSizeLabel.Text = "Size: ";
                        SequenceSizeLabel.Text += seq.Lenght.ToString("D");
                        SequenceSizeLabel.Text += " bytes";
                    }
                    counter++;
                }


                foreach (NInst inst in CurrentNInstList)
                {
                    if (CurrentMusic.SelectedIndex == inst.SequenceID)
                    {
                        InstSet.SelectedIndex = inst.NInsts[0];
                        InstDataLabel.Text = "InstData: ";
                        InstDataLabel.Text += inst.NInstCount.ToString("X2");
                        InstDataLabel.Text += " ";

                        int count = 0;
                        bool doOnce = true;
                        foreach (byte b in inst.NInsts)
                        {
                            InstDataLabel.Text += inst.NInsts[count].ToString("X2");
                            InstDataLabel.Text += " ";
                            count += 1;
                            if (count > 3)
                            {
                                if (doOnce) InstDataLabel.Text += "\n";
                                doOnce = false;
                            }
                        }
                    }
                }

           
            }

            

        }

        private void SaveMusic_Click(object sender, EventArgs e)
        {
            
            FileStream fs = new FileStream(RomFile.Text, FileMode.Open, FileAccess.ReadWrite);
            BinaryReader br = new BinaryReader(fs);
            BinaryWriter bw = new BinaryWriter(fs);

            /* Music Hack (get rid of sequence size limitations) */

            if (MusicHackOn.Checked == true)
            {
                bw.Seek(0xD213A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x8057));
                bw.Seek(0xD215A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x8057));
                bw.Seek(0xD459A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x8057));
                bw.Seek(0xEE2B0, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x0000BD00));
                MusicHack = true;
            }
            else if (MusicHackOff.Checked == true)
            {
                bw.Seek(0xD213A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x801D));
                bw.Seek(0xD215A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x801D));
                bw.Seek(0xD459A, SeekOrigin.Begin);
                bw.Write(SwapUInt16(0x801D));
                bw.Seek(0xEE2B0, SeekOrigin.Begin);
                bw.Write(SwapInt32(0x00006D00));
                MusicHack = false;
            }

            int selectedMusic = Music.SelectedIndex;
            int selectedCurrentMusic = CurrentMusic.SelectedIndex;
                
            bw.Seek(0x7f1000, SeekOrigin.Begin);
            
            foreach (object o in CurrentMusic.Items)
            {
                bw.Write(o.ToString());

            }
             
            bw.Seek(0x7f006a, SeekOrigin.Begin);
                
            foreach (NInst inst in CurrentNInstList) inst.WriteNInst(fs, br, bw);

            //write table
            bw.Seek(0x7f0000, SeekOrigin.Begin);
            foreach (NInst inst in CurrentNInstList)
            {
                bw.Write(SwapUInt16(inst.NewStartOffset));
            }
                
            br.Close();
            bw.Close();
            fs.Close();

            Music.SelectedIndex = selectedMusic;
            CurrentMusic.SelectedIndex = selectedCurrentMusic;

            MessageBox.Show("Music Settings sucessfully saved!", "Status");

            ReadMusicData();
            label49.Visible = false;
        }

        private void tabPage8_Click(object sender, EventArgs e)
        {

        }

        private void RenameSequence_Click(object sender, EventArgs e)
        {
            int counter = 0;
            label49.Visible = true;

            int selectedMusic = Music.SelectedIndex;
            int selectedCurrentMusic = CurrentMusic.SelectedIndex;
            Music.Items.Clear();

            foreach (object o in CurrentMusic.Items)
            {

                if (counter == CurrentMusic.SelectedIndex)
                {
                    Music.Items.Add(MusicName.Text);
                }
                else
                {
                    Music.Items.Add(o.ToString());
                } 

                counter += 1;
                
            }

            CurrentMusic.Items.Clear();
            // copy objects from combobox
            object[] a = new object[Music.Items.Count];
            Music.Items.CopyTo(a, 0);
            CurrentMusic.Items.AddRange(a);

            CurrentMusicCopy = CurrentMusic.SelectedIndex;

            Music.SelectedIndex = selectedMusic;
            CurrentMusic.SelectedIndex = selectedCurrentMusic;
        }

        private void MusicName_TextChanged(object sender, EventArgs e)
        {

        }

        private void ExtractSequence_Click(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save Mario 64 Sequence File (.m64)";
            dlg.Filter = "(.m64 Mario 64 Sequence File)|*.m64";
            if (dlg.ShowDialog() == DialogResult.OK)
            {

                FileStream fs = new FileStream(RomFile.Text, FileMode.Open, FileAccess.ReadWrite);
                FileStream newFile = new FileStream(dlg.FileName, FileMode.Create);
                BinaryWriter bwOut = new BinaryWriter(newFile);
                BinaryWriter bw = new BinaryWriter(fs);

                UInt32 Offset = 0;
                UInt32 Lenght = 0;
                int counter = 0;
                foreach (Sequence seq in CurrentSequenceList)
                {
                    if (CurrentMusic.SelectedIndex == counter)
                    {
                        Offset = seq.StartOffset;
                        Lenght = seq.Lenght;
                    }
                    counter++;
                }

                byte[] buffer;
                bw.Seek(0x2F00000 + (int)Offset, SeekOrigin.Begin);
                try
                {
                    buffer = new byte[Lenght];
                    int count;
                    int sum = 0;

                    while ((count = fs.Read(buffer, sum, (int)Lenght - sum)) > 0)
                        sum += count;
                }
                finally
                {
                    bw.Close();
                    fs.Close();
                }

                bwOut.Write(buffer);
                newFile.Close();
                bwOut.Close();

                MessageBox.Show(("Sequence \"" + CurrentMusic.Text + "\" sucessfully extracted!"));
            }


        }

        private void KillCamera_CheckedChanged(object sender, EventArgs e)
        {
            if (KillCamera.Checked == true)
            {
                MessageBox.Show("Toogling this option will free Lakitu's camera from some level-dependent\n hardcoded camera positions and angles.", "Help");
            }
        }

        private void Camera_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void SetPauseWarp_CheckedChanged(object sender, EventArgs e)
        {
            //MessageBox.Show("To Do: enable/ disbable menu stuff. Explain about warp (0x26F0 ? is that really it)?");
        }

        private void CameraList_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void StartLevel_CheckedChanged(object sender, EventArgs e)
        {
            
        }

        private void MusicHackOn_CheckedChanged(object sender, EventArgs e)
        {
            
        }

        private void SwapXZ_CheckedChanged(object sender, EventArgs e)
       {
           if (SwapXZ.Checked == true)
           {
               MessageBox.Show("Enabling this option will correct the orientation of models \nmade in SketchUp, which uses a left-hand coordinate system\nas opposed to the right-hand system used in the N64.\n\nRead the documentation at the Level Importer site for\nmore information about it.", "Unasked-for help message");
           }


        }

        private void RotateY_CheckedChanged(object sender, EventArgs e)
        {
            if (RotateY.Checked == true) AngleY.Enabled = true;
            else AngleY.Enabled = false;
        }

        private void PreviewLevel_Click(object sender, EventArgs e)
        {

        }

        private void PauseWarpLevel_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void LevelList_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label53_Click(object sender, EventArgs e)
        {

        }

        private void Preview_Click(object sender, EventArgs e)
        {

            FileInfo executableFileInfo = new FileInfo(Application.ExecutablePath);
            string ApplicationPath = executableFileInfo.DirectoryName + "\\preview.exe";
            if (File.Exists(ApplicationPath) == false) { MessageBox.Show("File preview.exe not found! (Must be in the same directory as this program).", "Error"); return; }
            if (objFile.Text == "No file loaded!") { MessageBox.Show("Load an Wavefront .obj file first!", "Error"); return; }

            string SettingsPath = executableFileInfo.DirectoryName + "\\previn";
            StreamWriter Settings = new StreamWriter(SettingsPath);
            String set = "<?xml version=\"1.0\"?><config><startUpModel file=\"" + objFile.Text + "\" /><messageText caption=\"Mario 64 Level Importer - 3D Preview (Irrlicht Engine)\"></messageText><scaling value = \"" + float.Parse(ScalingValue.Text) + "\"></scaling><rotation yvalue = \"" + AngleY.Value + "\"></rotation>" + "<offset xvalue=\"" + OffsetX.Value + "\" yvalue=\"" + OffsetY.Value + "\" zvalue=\"" + OffsetZ.Value + "\"></config>";
            Settings.WriteLine(set); 
            Settings.Close();            

            Process ProcessObj = new Process();

            ProcessObj.StartInfo.FileName = ApplicationPath;
            //ProcessObj.StartInfo.Arguments = ApplicationArguments;
            ProcessObj.StartInfo.UseShellExecute = false;

            System.Reflection.Assembly a = System.Reflection.Assembly.GetEntryAssembly();
            string baseDir = System.IO.Path.GetDirectoryName(a.Location);
            ProcessObj.StartInfo.WorkingDirectory = baseDir;
            ProcessObj.Start();
            ProcessObj.WaitForExit();

            if (ProcessObj.ExitCode == 1) //TO DO: finish this
            {
                string pout = baseDir + "\\prevout";
                //MessageBox.Show(pout);
                StreamReader sr = new StreamReader(pout);            
                string line = sr.ReadLine();

                while (line != null)
                {
                    if (line.StartsWith("o"))
                    {
                        try
                        {
                            object[] targets = new object[3];
                            targets[0] = new Int32();
                            targets[1] = new Int32();
                            targets[2] = new Int32();

                            Scanner scanner = new Scanner();
                            scanner.Scan(line.Substring(2), "{0} {1} {2}", targets);

                            OffsetX.Value = Convert.ToInt32(targets[0]);
                            OffsetY.Value = Convert.ToInt32(targets[1]);
                            OffsetZ.Value = Convert.ToInt32(targets[2]);

                            if (OffsetX.Value != 0 || OffsetY.Value != 0 || OffsetZ.Value != 0) OffsetPosition.Checked = true;
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show(ex.ToString());
                        }
                    }
                    else if (line.StartsWith("s"))
                    {
                        ScalingValue.Text = line.Substring(2);
                    }
                    else if (line.StartsWith("y"))
                    {
                        try
                        {
                            object[] targets = new object[2];
                            targets[0] = new Int32();
                            targets[1] = new Int32();
                            Scanner scanner = new Scanner();
                            scanner.Scan(line.Substring(2), "{0}", targets);
                            AngleY.Value = Convert.ToInt32(targets[0]);

                            if (AngleY.Value != 0) RotateY.Checked = true;
                            
                        }
                        catch (Exception ex)
                        {
                             MessageBox.Show(ex.ToString());
                        }
                    }
                    line = sr.ReadLine();
                }

                sr.Close();
            
            }

        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void Polygon_CheckedChanged(object sender, EventArgs e)
        {
            if (Polygon.Checked == true)
            {
                MessageBox.Show("Toggling this option will enable you to import models up to 9000 triangles.\nHowever, you may experience random errors as well as memory corruption. \nUse this option at your own risk!", "Warning");

            }
        }

        private void Text_Click(object sender, EventArgs e)
        {
         //   if (RomFile.Text == "No file loaded!") { MessageBox.Show("Load a proper ROM file first!", "Error"); return; }

            Text TextForm = new Text(RomFile.Text);
            TextForm.ShowDialog();

        }


    }



}
