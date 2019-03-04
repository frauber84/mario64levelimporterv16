using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ObjImportGUI
{
    class Collision
    {
        public int ID;
        public string TextureName;
        public int Type;
        public int CParam1;
        public int CParam2;

        public Collision(int id, int type, int cparam1, int cparam2)
        {
            ID = id;
            Type = type;
            CParam1 = cparam1;
            CParam2 = cparam2;
        }

        public void SetTextureName(string textureName)
        {
            TextureName = textureName;
        }

        public string GetTextureName()
        {
            return TextureName;
        }

        public void SetType(int value)
        {
            Type = value;
        }

        public void SetCParam1(int value)
        {
            CParam1 = value;
        }

        public void SetCParam2(int value)
        {
            CParam2 = value;
        }


    }
}
