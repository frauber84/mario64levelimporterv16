// Eventually:
// - Check this Bug: Camera Zoom in/out = Stops music! (also when talking to Toad?)
// - Text settings (credits) (?)
// - Hardcoded Music Settings

// v16
// - To DO: texture brightness
// - To DO: XML load for new functions
// - overtide 6500 polygon limitation
// - 3D preview
// - Swap X/Z coordinates
// - y rotation
// - fixed pause warp  bug

// v15

// - Corrected problem when setting bob omb star position
// - To DO: make Settings acurately represent what's in the ROM
// - BUBLLY TREE AND OTHERS
// - NO INVISIBLE WALLS!
// - NO MUSIC LIMITATIONS
// -  Kill camera on specific levels
// - Corrected bug on camera presets (zero vs. one indexed)
// - Exit Menu Warp
// - Fixed trajectory function
// - vastly improved documentation
// - fixed annoyance when using the Peach Slide Timer function  false nothing is selected warning)
 
// new shared -x22? Spiky Tree (0x18)
// Bubly tree
// Snow Tree (0x19)
// Metal Door (0x1F)
// Hazy Maze Door (0x20)
// Castle Door 1 Star
// Castle Door 2 Star
// Castle Door 3 Star
// Castle Door with lock
// Castle Door


// v14.1
// - 14.1: fixed killcamera + more instrument sets
// - better ROM checking
// - 6500 faces
// - custom music: replace, extract
// - MANY new stars
// - MANY new trajectories
// - peach slide timer
// - fixed problem with Fog colors
// - fixed restore option
// - Set Camera Presets!
// - Kill Camera option


// v13
// - redesigned GUI, better interface
// - fixed mapKd issue which prevented some .obj exporter plug-ins to work
// - multiple water boxes (still some stuff to do)
// - hardcoded start position
// - fixed false "vertex out of range" warnings
// - act selector hack
// - trajectories for racing penguin and snowman's bottom
// - collision mapping now has an interface
// - support for extra collision parameters
// - fixed "hardcore" fog settings
// - background parts (bg00.bmp files) are now correctly deleted after conversion


// v12
// - flip textures function
// - improved background importer (thanks VL-Tone for the help. Not 100% done yet)
// - fixed a bug when restoring levels
// - Water box/toxic etc

// v11
// - offseting models
// - restore crc
// - new star positions
// - enabling fog doesn't break the level on tt64 anymore
// - fixed weather problem when alpha faces weren't present
// - trajectory settings
// - fixed alpha problems when fog was enabled
// - custom BG!
// - minor GUI redesign

// 0.9:
// - collisionType 80 (not solid) 
// - alpha
// - fog presets


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "obj_import.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShellApi.h>


void StripToPath(char* szFilename )
{
    int i = 0, pos = 0;
    while ( szFilename[i] != '\0' )
    {
        if ( szFilename[i] == '\\' )
        pos = i;
        i++;
    }
    szFilename[pos + 1] = '\0';
}

int Read32(FILE *fp)
{
    int s32;
    unsigned int ch[4];
    unsigned int x;

    ch[0] = fgetc(fp) << 24;
    ch[1] = fgetc(fp) << 16;
    ch[2] = fgetc(fp) << 8;
    ch[3] = fgetc(fp);

    x = ch[0] | ch[1] | ch[2] | ch[3];

    s32 = (x >> 24) | (x >> 8 & 0x0000FF00) | (x << 8 & 0x00FF0000) | (x << 24);
    return s32;
}
void Write16(int u16, FILE *fp)
{
     fputc(u16 >> 8, fp);
     fputc(u16, fp);
}

void Write32(int u32, FILE *fp)
{
        fputc(u32 >> 24, fp);
        fputc(u32 >> 16, fp);
        fputc(u32 >> 8, fp);
        fputc(u32, fp);

}

void ParseObj(char *FileName, FILE* vtx)
{

    char line[2048];
    char CurrentMTL[100];
    char mtllib[100];
    int i;

    FILE *fp = fopen(FileName, "r");
    if (!fp) exit(ERROR_OBJ);

    strcpy(CurrentMTL, "grass");
    mtlList[0].MtlPointer = 0x09005800;
    InitMTL();

    fList[0].CollisionType = 666;

    while (fgets(line, 2048, fp))
    {
          
        char *ptr = (void*)0;

        if (ptr = strstr(line,"mtllib ") )
        {
            strcpy(mtllib, ReadString(ptr+7));
            sprintf(DebugMessage, "\n[Material]\nMaterial Library = %s", mtllib);
            DebugPrint(DebugMessage);
            if (NOMTL == 0) ParseMTL(mtllib, vtx);
        }
        else if (ptr = strstr(line,"usemtl ") )
        {
            strcpy(CurrentMTL, ReadString(ptr+7));

            /* check if it's a new MTL or not */
            for (i = 0; i <= MAX_MTL; i++)
            {
                if (  strcmp(CurrentMTL, mtlList[i].MtlName) == 0 ) goto WayOut;
            }

            /* new material */
            mtlCount++;
            sprintf(DebugMessage, "\nMaterial %d = %s @ 0x%08x", mtlCount, CurrentMTL, mtlList[mtlCount].MtlPointer);
            DebugPrint(DebugMessage);
            strcpy(mtlList[mtlCount].MtlName, CurrentMTL);
            WayOut: ;

        }
		else if (ptr = strstr(line,"v ") )
	    {
		    vCount++;
		    
		    if (SWAPXZ) sscanf(ptr+2, "%f %f %f", &vList[vCount].z, &vList[vCount].y, &vList[vCount].x);
	  	    else sscanf(ptr+2, "%f %f %f", &vList[vCount].x, &vList[vCount].y, &vList[vCount].z);
	  	    
	  	    if (ROTATEY)
	  	    {
                 if (SWAPXZ == 0)
                 {
                     float tempX = (vList[vCount].z * cos(ANGLEY*PI/180) )  -  ( vList[vCount].x * sin(ANGLEY*PI/180) )  ;
                     float tempZ = (vList[vCount].z * sin(ANGLEY*PI/180) )  +  ( vList[vCount].x * cos (ANGLEY*PI/180) ) ;
                     
	  	             vList[vCount].x = tempX;
         	  	     vList[vCount].z = tempZ;
                     
                 }
                 else
                 {
                     printf("\nT Do: handle Y rotation when SwapXZ ==1");
                 }
                 
            }

	       if ( ( (vList[vCount].x * SCALING) + OFFSETX) > 8192 || ( (vList[vCount].x * SCALING) + OFFSETX) < -8192)
	       {
               OUT_OF_RANGE = 1;
               sprintf(DebugMessage, "\n[Warning]\nVertex %d out of range (between +8192 and -8192) = %f %f %f", vCount, (vList[vCount].x*SCALING)+OFFSETX, (vList[vCount].y*SCALING)+OFFSETY, (vList[vCount].z*SCALING)+OFFSETZ);
               DebugPrint(DebugMessage);
           }
	       else if ( ( (vList[vCount].z * SCALING) + OFFSETZ) > 8192 || ( (vList[vCount].z * SCALING) + OFFSETZ) < -8192)
           {
               OUT_OF_RANGE = 1;
               sprintf(DebugMessage, "\n[Warning]\nVertex %d out of range (between +8192 and -8192) = %f %f %f", vCount, (vList[vCount].x*SCALING)+OFFSETX, (vList[vCount].y*SCALING)+OFFSETY, (vList[vCount].z*SCALING)+OFFSETZ);
               DebugPrint(DebugMessage);
           }


		}
		else if (ptr = strstr(line,"vn ") )
	    {
		    vnCount++;
		    
		    if (SWAPXZ) sscanf(ptr+3, "%f %f %f", &vnList[vnCount].z, &vnList[vnCount].y, &vnList[vnCount].x);
		    else sscanf(ptr+3, "%f %f %f", &vnList[vnCount].x, &vnList[vnCount].y, &vnList[vnCount].z);
		    
		    signed char	nx = vnList[vnCount].x * 127;
            signed char	ny = vnList[vnCount].y * 127;
            signed char	nz = vnList[vnCount].z * 127;
		    
	        sprintf(DebugMessage, "\nvn[%d] = %f %f %f (%x %x %x)", vnCount, vnList[vnCount].x, vnList[vnCount].y, vnList[vnCount].z, nx, ny, nz);
	        DebugPrint(DebugMessage);
	        
	        vnList[vnCount].x = nx;
	        vnList[vnCount].y = ny;
	        vnList[vnCount].z = nz;
		}
		else if (ptr = strstr(line,"vt ") )
	    {
		    vtCount++;
		    sscanf(ptr+3, "%f %f", &vtList[vtCount].s, &vtList[vtCount].t);
		    
		    if (vtList[vtCount].s > 32 || vtList[vtCount].s < -32 || vtList[vtCount].t > 32 || vtList[vtCount].t < -32)
		    {
               sprintf(DebugMessage, "\n[Warning]\nvt %d out of range (between +32 and -32) --> s = %f t = %f", vtCount, vtList[vtCount].s, vtList[vtCount].t);
               DebugPrint(DebugMessage);
               VT_OUT_OF_RANGE = 1;
               
               if (vtList[vtCount].s > 32) vtList[vtCount].s = 31.9;
               else if (vtList[vtCount].s < -32) vtList[vtCount].s = -31.9;
               
               if (vtList[vtCount].t > 32) vtList[vtCount].s = 31.9;
               else if (vtList[vtCount].t < -32) vtList[vtCount].s = -31.9;               
            }

		}
		else if (ptr = strstr(line,"f ") )
	    {
            int qd;
            int ret, ret2, ret3;
		    fCount++;
		    
		    if (SWAPXZ)
            {
               ret = sscanf(ptr+2, "\n%d/%d/%d %d/%d/%d %d/%d/%d", &fList[fCount].v3, &fList[fCount].vt3, &fList[fCount].vn3,
 		                                                 &fList[fCount].v2, &fList[fCount].vt2, &fList[fCount].vn2,
		                                                 &fList[fCount].v1, &fList[fCount].vt1, &fList[fCount].vn1);
            }
            else
            {

      	       ret = sscanf(ptr+2, "\n%d/%d/%d %d/%d/%d %d/%d/%d", &fList[fCount].v1, &fList[fCount].vt1, &fList[fCount].vn1,
 		                                                 &fList[fCount].v2, &fList[fCount].vt2, &fList[fCount].vn2,
		                                                 &fList[fCount].v3, &fList[fCount].vt3, &fList[fCount].vn3);
            }
            
           if (ret == 1) // only face data found (or face and normal)
           {
               if (SWAPXZ) ret2 = sscanf(ptr+2, "\n%d %d %d", &fList[fCount].v3, &fList[fCount].v2, &fList[fCount].v1);
               else ret2 = sscanf(ptr+2, "\n%d %d %d", &fList[fCount].v1, &fList[fCount].v2, &fList[fCount].v3);

               if (ret2 == 1)
               {
 		           if (SWAPXZ) sscanf(ptr+2, "\n%d//%d %d//%d %d//%d", &fList[fCount].v3, &fList[fCount].vn3, &fList[fCount].v2, &fList[fCount].vn2, &fList[fCount].v1, &fList[fCount].vn1);
                    else sscanf(ptr+2, "\n%d//%d %d//%d %d//%d", &fList[fCount].v1, &fList[fCount].vn1, &fList[fCount].v2, &fList[fCount].vn2, &fList[fCount].v3, &fList[fCount].vn3);
               }
           }
           else if (ret == 2) // face and texture coordinate
           {
                
              if (SWAPXZ)
              {
  	              sscanf(ptr+2, "\n%d/%d %d/%d %d/%d", &fList[fCount].v1, &fList[fCount].vt1,
                                                     &fList[fCount].v2, &fList[fCount].vt2,
                                                     &fList[fCount].v3, &fList[fCount].vt3);
              }
              else
              {
  	              sscanf(ptr+2, "\n%d/%d %d/%d %d/%d", &fList[fCount].v1, &fList[fCount].vt1,
                                                     &fList[fCount].v2, &fList[fCount].vt2,
                                                     &fList[fCount].v3, &fList[fCount].vt3);
              }
           }

           strcpy(fList[fCount].MtlName, CurrentMTL);

           /* handle collision types */
           for (i = 0; i <= mtlCount; i++)
           {
               if (  strcmp(fList[fCount].MtlName, mtlList[i].MtlName) == 0 )
               {
                     fList[fCount].CollisionType = mtlList[i].CollisionType;

                     if (fList[fCount].CollisionType != fList[fCount-1].CollisionType)
                     {
                         CollisionGroups[cgroupCount].EndFace = fCount-1;
                         cgroupCount++;
                         CollisionGroups[cgroupCount].CParams = mtlList[i].CParams;
                         CollisionGroups[cgroupCount].StartFace = fCount;
                         CollisionGroups[cgroupCount].CollisionType = fList[fCount].CollisionType;
                     }
               }
           }
           CollisionGroups[cgroupCount].EndFace = fCount;

		}	// face parsing

	}  // end of while loop


    CenterVertexList();
    fclose(fp);
}

void AddToVCache(int index, struct face faceList, int the_k_factor)
{

    float TexXR = (mtlList[the_k_factor].width/32); // unused, not necessary for now
    float TexYR = (mtlList[the_k_factor].height/32);

    //TexXR =1;
    //TexYR = 1;
    
    // To Do: How to correctly use normal data?

    //vert 1
    VertexCache_tn[index].x = (vList[faceList.v1].x * SCALING) + OFFSETX;
    VertexCache_tn[index].y = (vList[faceList.v1].y * SCALING) + OFFSETY;
    VertexCache_tn[index].z = (vList[faceList.v1].z * SCALING) + OFFSETZ;
    VertexCache_tn[index].s = (vtList[faceList.vt1].s) * 32 * 32 * (TexXR);
    VertexCache_tn[index].t = (vtList[faceList.vt1].t) * 32 * 32 * (TexYR);
    VertexCache_tn[index].nx = (vnList[faceList.vn1].x  * 127); // (signed char)(vnList[faceList.vn1].x) * 127;
    VertexCache_tn[index].ny = (vnList[faceList.vn1].y * 127); // (signed char)(vnList[faceList.vn1].y) * 127;
    VertexCache_tn[index].nz = (vnList[faceList.vn1].z * 127); // (signed char)(vnList[faceList.vn1].z) * 127;

    // vert2
    VertexCache_tn[index+1].x = (vList[faceList.v2].x * SCALING) + OFFSETX;
    VertexCache_tn[index+1].y = (vList[faceList.v2].y * SCALING) + OFFSETY;
    VertexCache_tn[index+1].z = (vList[faceList.v2].z * SCALING) + OFFSETZ;
    VertexCache_tn[index+1].s = (vtList[faceList.vt2].s) * 32 * 32 * (TexXR);
    VertexCache_tn[index+1].t = (vtList[faceList.vt2].t) * 32 * 32 * (TexYR);
    VertexCache_tn[index+1].nx = (vnList[faceList.vn2].x * 127); // (signed char)(vnList[faceList.vn2].x) * 127;
    VertexCache_tn[index+1].ny = (vnList[faceList.vn2].y * 127); // (signed char)(vnList[faceList.vn2].y) * 127;
    VertexCache_tn[index+1].nz = (vnList[faceList.vn2].z * 127); // (signed char)(vnList[faceList.vn2].z) * 127;

    // vert 3
    VertexCache_tn[index+2].x = (vList[faceList.v3].x * SCALING) + OFFSETX;
    VertexCache_tn[index+2].y = (vList[faceList.v3].y * SCALING) + OFFSETY;
    VertexCache_tn[index+2].z = (vList[faceList.v3].z * SCALING) + OFFSETZ;
    VertexCache_tn[index+2].s = (vtList[faceList.vt3].s) * 32 * 32 * (TexXR);
    VertexCache_tn[index+2].t = (vtList[faceList.vt3].t) * 32 * 32 * (TexYR);
    VertexCache_tn[index+2].nx = (vnList[faceList.vn3].x * 127); // (signed char)(vnList[faceList.vn3].x) * 127;
    VertexCache_tn[index+2].ny = (vnList[faceList.vn3].y * 127); // (signed char)(vnList[faceList.vn3].y) * 127;
    VertexCache_tn[index+2].nz = (vnList[faceList.vn3].z * 127); // (signed char)(vnList[faceList.vn3].z) * 127;

}

void WriteCache(FILE *vert)
{
     int i = 0;
     int check = 0;

    vCachePointer = (ftell(vert) + SEGMENT);

     check = ftell(vert);
     while ( !(check % 0x10 == 0) )
     {
        fputc(0x00, vert); // pad for correct alignment
        check = ftell(vert);
     }

     for(i = 0; i < 15; i++)
     {
        Write16(VertexCache_tn[i].x, vert);
        Write16(VertexCache_tn[i].y, vert);
        Write16(VertexCache_tn[i].z, vert);
        Write16(0x0000, vert); // flag
        Write16(VertexCache_tn[i].s, vert);
        Write16(VertexCache_tn[i].t, vert);
        /*
        fputc(VertexCache_tn[i].nx, vert);
        fputc(VertexCache_tn[i].ny, vert);
        fputc(VertexCache_tn[i].nz, vert);
        */
        fputc(0, vert);
        fputc(0, vert);
        fputc(0, vert);
        fputc(0xFF, vert);  //alpha?
     }

}

void LoadCache(FILE *fp)
{
    Write16(0x04E0, fp);
    Write16(0x00F0, fp);
    Write32(vCachePointer, fp);
}

void WriteTriangle (FILE *fp, int f1, int f2, int f3)
{
     Write16(0xBF00, fp);
     Write16(0x0000, fp);
     fputc(0x00, fp);

     fputc(f1, fp);
     fputc(f2, fp);
     fputc(f3, fp);
}

void InitDL(FILE *fp)
{

   int check;
   check = ftell(fp);
   while ( !(check % 0x10 == 0) )
   {
       fputc(0x00, fp); // pad for correct alignment
       check = ftell(fp);
    }

   if (LEVEL == 31) fwrite(DLInit2, sizeof(DLInit2), 1, fp); // title screen
   else fwrite(DLInit, sizeof(DLInit), 1, fp);

}

void FinishDL(FILE *fp)
{
   fwrite(DLFinish, sizeof(DLFinish), 1, fp);
}

void WriteVert(unsigned int x, unsigned int y, unsigned int z, FILE *fp)
{
    Write16(x, fp);
    Write16(y, fp);
    Write16(z, fp);

}
void WriteCollision(FILE *fp)
{

    int i, k;

    Write16(0x0040, fp);
    Write16(vCount + 4, fp);

    for (i = 1; i <= vCount; i++ )
    {
        Write16((vList[i].x * SCALING) + OFFSETX, fp);
        Write16((vList[i].y * SCALING) + OFFSETY, fp);
        Write16((vList[i].z * SCALING) + OFFSETZ, fp);
    }

    // death at bottom
    WriteVert(0x2000, DEATH_HEIGHT, 0x2000, fp);
    WriteVert(0xDFFF, DEATH_HEIGHT, 0xDF00, fp);
    WriteVert(0xDFFF, DEATH_HEIGHT, 0x2000, fp);
    WriteVert(0x2000, DEATH_HEIGHT, 0xDFFF, fp);

    sprintf(DebugMessage, "\n[Collision groups]");
    DebugPrint(DebugMessage);

    for (i = 1; i <= cgroupCount; i++ )
    {
        sprintf(DebugMessage, "\nCollision group %d of %d", i, cgroupCount);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nStartFace = %d", CollisionGroups[i].StartFace);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nEndFace = %d", CollisionGroups[i].EndFace);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nNumFaces = %d", CollisionGroups[i].EndFace - CollisionGroups[i].StartFace + 1);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nColType = %d", CollisionGroups[i].CollisionType);
        DebugPrint(DebugMessage);
        
        if (CollisionGroups[i].CollisionType == 80) // non-solid object
        {
            sprintf(DebugMessage, " (non-solid)");
            DebugPrint(DebugMessage);
            goto SkipGroup;
        }
        sprintf(DebugMessage, "\nCParams = %d", CollisionGroups[i].CParams);
        DebugPrint(DebugMessage);
        
        Write16(CollisionGroups[i].CollisionType, fp);
        Write16(CollisionGroups[i].EndFace - CollisionGroups[i].StartFace +1, fp);

        int NumFaces = CollisionGroups[i].EndFace - CollisionGroups[i].StartFace;
        for (k = 0; k <= NumFaces; k++)
        {

            Write16(fList[CollisionGroups[i].StartFace+k].v1 - 1, fp);
            Write16(fList[CollisionGroups[i].StartFace+k].v2 - 1, fp);
            Write16(fList[CollisionGroups[i].StartFace+k].v3 - 1, fp);
            
            if (CollisionGroups[i].CollisionType == 0x04 || CollisionGroups[i].CollisionType == 0x0E || CollisionGroups[i].CollisionType == 0x24 || CollisionGroups[i].CollisionType == 0x25 || CollisionGroups[i].CollisionType == 0x27 || CollisionGroups[i].CollisionType == 0x2C || CollisionGroups[i].CollisionType == 0x2D) 
            {
                Write16(CollisionGroups[i].CParams, fp);
            }
        }
        
        SkipGroup: ;

    }

    /* death at bottom */
    if (DEATH_AT_BOTTOM == 1)
    {
         Write16(0x000A, fp);
         Write16(0x0002, fp);
         Write16(vCount + 0, fp);
         Write16(vCount + 1, fp);
         Write16(vCount + 2, fp);
         Write16(vCount + 0, fp);
         Write16(vCount + 3, fp);
         Write16(vCount + 1, fp);
     }
     
    Write16(0x0041, fp); 

    if (WATER == 1)
    {
        Write16(0x0044, fp);
        Write16(WaterCount, fp); // no. of boxes
        
        for (i = 1; i <= WaterCount; i++)
        {
                Write16(WaterList[i].index, fp); // index
// top left
// bottom right
                Write16(WaterList[i].minx, fp );
                Write16(WaterList[i].minz, fp );
                Write16(WaterList[i].maxx, fp );
                Write16(WaterList[i].maxz, fp );
                
                Write16(WaterList[i].height, fp );
        }
    }

    /* finish him! */
    Write16(0x0042, fp);

}

int main ( int argc, char *argv[] )
{

    int i,k, j ;

    // hack to get cvt.exe directory right
    
    StripToPath(argv[0]);
    strcpy(CvtExePath, argv[0]);
    int len = strlen(CvtExePath);
    CvtExePath[len    ] = 'c';
    CvtExePath[len + 1] = 'v';
    CvtExePath[len + 2] = 't';
    CvtExePath[len + 3] = '.';
    CvtExePath[len + 4] = 'e';
    CvtExePath[len + 5] = 'x';
    CvtExePath[len + 6] = 'e';
    CvtExePath[len + 7] = 0x00;

    //debugPath
    strcpy(DebugLogPath, argv[0]);
    len = strlen(DebugLogPath);
    DebugLogPath[len    ] = 'l';
    DebugLogPath[len + 1] = 'o';
    DebugLogPath[len + 2] = 'g';
    DebugLogPath[len + 3] = '.';
    DebugLogPath[len + 4] = 't';
    DebugLogPath[len + 5] = 'x';
    DebugLogPath[len + 6] = 't';
    DebugLogPath[len + 7] = 0x00;

    sprintf(DebugMessage, "[Environment]\nConvert path = %s  ", CvtExePath);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nobj_import.exe version = %s", VERSION);
    DebugPrint(DebugMessage);

    vCount = 0;
    vnCount = 0;
    vtCount = 0;
    fCount = 0;
    cgroupCount = 0;

    if ( argc < 4 || argc > 6 )
	{
		printf("Mario 64 Wavefront .OBJ importer %s by messiaen\n", VERSION);
		printf("Please use Windows GUI (ObjImportGUI.exe).");
		getch();
		//[obj] [rom] [bgfile] [settingsfile]\n");
		exit(-1);
	}

	printf("\nImporting level. Please wait ... ");

    strcpy(ObjFilePath, argv[2]);
    StripToPath(ObjFilePath);
	FILE *ROM = fopen(argv[2], "r+b");
	if (!ROM)
	{
        sprintf(DebugMessage, "\n[Fatal Error]\nError reading ROM file %s", argv[2]);
        DebugPrint(DebugMessage);
        exit(ERROR_ROM);
    }
    
   	FILE *SETTINGS = fopen(argv[4], "r+b");
	if (!SETTINGS)
	{
        sprintf(DebugMessage, "\n[Fatal Error]\nError reading settings file %s", argv[4]);
        DebugPrint(DebugMessage);
        exit(ERROR_ROM); // Adjust
    }


    int RomSize = 0;
    fseek(ROM, 0, SEEK_END);
    RomSize = ftell(ROM);

    if ( RomSize < 0x3000000) exit(0x0102);

    fseek(ROM, 0x12015C7, SEEK_SET);
    unsigned int ch = fgetc(ROM);
    if (ch != 0x14)
    {
        exit (0x0102); // not the right ROM
    }
    else
    {
        fseek(ROM, 0x108678, SEEK_SET);
        Write32(0x80402000, ROM);
        WriteNewPatch(ROM); // update level command 0x17

        fseek(ROM, 0x101B84, SEEK_SET); //extended memory patch
        fwrite(ExtendPatch, sizeof(ExtendPatch), 1, ROM);
    }

    fseek(ROM, 0x120217A, SEEK_SET);
    ch = fgetc(ROM);
    if (ch != 0xE1) exit (0x0102); // not the right ROM

    ReadSettings(SETTINGS);
    WriteWaterPatch(ROM);  
    
    if (RESTORE)
    {
        RestoreLevelPointer(ROM);
        exit(0x09);
   }

    if (OFFSETMODEL == 0) 
    {
        OFFSETX = 0;
        OFFSETY = 0;
        OFFSETZ = 0; 
    }
    
    sprintf(DebugMessage, "\n[Import Parameters] \nObjFile = %s\nScaling = %f\nLevel = %d\nWeather = %d\nBackground = %d\nCreate Death at Bottom plane = %d\nRestore Level Flag = %d\nBank 0x0C = %d\nBank 0x0D = %d\nBank 0x0E = %d\nMusic = %d\nTerrain = %d", argv[1], SCALING, LEVEL, WEATHER, BACKGROUND, DEATH_AT_BOTTOM, RESTORE, BANKC, BANKD, BANKE, MUSIC, TERRAIN);
    DebugPrint(DebugMessage);

    if (SCALING == 0) exit(ERROR_SCALING);

    SEGMENT = 0x0e000000;
    if (LEVEL == 31) SEGMENT = 0x0700C940;

    if (NOMTL == 1)
    {
        sprintf(DebugMessage, "\nNoMtl = 1 (Textures from the game will be used)");
        DebugPrint(DebugMessage);
    }
    
    sprintf(DebugMessage, "\nDeatHeight = %d", DEATH_HEIGHT);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nEnable Fog = %d", FOG);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nFog Preset = %d (0x%08x)", FOGPRESET, FogPresets[FOGPRESET]);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nFog Red = 0x%02x", FOGR);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nFog Green = 0x%02x", FOGG);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nFog Blue = 0x%02x", FOGB);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nCustom Background = %d", CUSTOMBG);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nBackground image = %s", argv[3]);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nOffset Model = %d ( X = %d, Y = %d, Z = %d)", OFFSETMODEL, OFFSETX,OFFSETY, OFFSETZ);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nWater = %d", WATER);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nFlipTextures = %d", FLIPTEXTURES);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nSwapXZ = %d", SWAPXZ);
    DebugPrint(DebugMessage);
    
    if (WATER == 1)
    {
        sprintf(DebugMessage, "\nWater Type = %d", WATERTYPE);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nWater Maximum X = %d", WATERX);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nWater Minimum Z = %d", WATERZ);
        DebugPrint(DebugMessage);
        sprintf(DebugMessage, "\nWater Collision Box Height X = %d", WATERHEIGHT);
        DebugPrint(DebugMessage);
    }


    // should use buffers, not these stupid files
    FILE *VTX = fopen("vtx", "w+b");
    FILE *DL = fopen("dl", "w+b");
    FILE *ALPHADL = fopen("alphadl", "w+b");
    FILE *COL = fopen("col", "w+b");
    FILE *BG = fopen("bg", "w+b");
    
    if (BRIGHT == 0)
    {
        Write32(0xFFFFFFFF, VTX);
        Write32(0xFFFFFFFF, VTX);
        Write32(0xFFFFFFFF, VTX);
        Write32(0xFFFFFFFF, VTX);
    }
    else if (BRIGHT == 1)
    {
        Write32(0xD8D8D8FF, VTX);
        Write32(0xD8D8D8FF, VTX);
        Write32(0xD8D8D8FF, VTX);
        Write32(0xD8D8D8FF, VTX);
    }
    else if (BRIGHT == 2)
    {
        Write32(0x999999FF, VTX);
        Write32(0x999999FF, VTX);
        Write32(0x999999FF, VTX);
        Write32(0x999999FF, VTX);
    }    
    else if (BRIGHT == 3)
    {
        Write32(0x727272FF, VTX);
        Write32(0x727272FF, VTX);
        Write32(0x727272FF, VTX);
        Write32(0x727272FF, VTX);
    }    
    else if (BRIGHT == 4)
    {
        Write32(0x4C4C4CFF, VTX);
        Write32(0x4C4C4CFF, VTX);
        Write32(0x4C4C4CFF, VTX);
        Write32(0x4C4C4CFF, VTX);
    }    
    else if (BRIGHT == 5)
    {
        Write32(0x262626FF, VTX);
        Write32(0x262626FF, VTX);
        Write32(0x262626FF, VTX);
        Write32(0x262626FF, VTX);
    }    

    ParseObj(argv[1], VTX);

    sprintf(DebugMessage, "\n[ObjFile]\nVertex Count = %d\nFace Count = %d\nMaterial Count = %d", vCount, fCount, mtlCount);
    DebugPrint(DebugMessage);

    if (vCount == 0 || fCount == 0) exit(ERROR_OBJ_NO_FACES); // problem with objfile
    
    if (IGNOREMAXFACES == 0) // TODO: Pass this in the importer
    {                       
        if (fCount > 6500)  exit (ERROR_OBJ_MAXFACES | fCount << 8 );
    }
    else if (IGNOREMAXFACES == 1)
    {
        if (fCount > 9000)  exit (ERROR_OBJ_MAXFACES | fCount << 8 );
    }  

    if (mtlCount > 40) MANY_TEXTURES = 1;

    InitDL(DL);
    InitDL(ALPHADL);

    DebugPrint("\n[Display List conversion]");
    
    int alpha = 0;
    
    if (FOG == 1)
    {
        Gfx *glistp = malloc(50000);
        gDPSetCycleType(glistp++,   G_CYC_2CYCLE);
        gDPSetFogColor(glistp++,  FOGR, FOGG, FOGB, 0xff);
        gDPSetRenderMode(glistp++,   G_RM_FOG_SHADE_A,   G_RM_AA_ZB_OPA_SURF2);
        gSPFogPosition(glistp++,  400, -450); // replaced by presets
        gSPSetGeometryMode(glistp++,  G_FOG);
        int *teste = (void*)glistp;
        teste -= 10;
        Write32(*teste, DL); teste++;
        Write32(*teste, DL); teste++;
        Write32(*teste, DL); teste++;
        Write32(*teste, DL); teste++;
        Write32(*teste, DL); teste++;
        Write32(*teste, DL); teste++;
        Write32(0xBC000008, DL);  teste++;
        Write32(FogPresets[FOGPRESET], DL);  teste++;
        Write32(*teste, DL);  teste++;
        Write32(*teste, DL);
        free(glistp);
	      
    }

    for (i = 1; i <= fCount; i +=5 )
    {
        // put data to vertex cache
        for (j = 0; j <= 4; j++)
        {

            if (1)  // don't ask!
            {
                if ( ! (strcmp(fList[(i+j)].MtlName, fList[(i+j)-1].MtlName) == 0) )
                {

                     for (k = 0; k <= mtlCount; k++)
                     {
                         if (  strcmp(fList[(i+j)].MtlName, mtlList[k].MtlName) == 0 )
                         {
                              float TexXR = mtlList[cur_mtl].width/32;
                              float TexYR = mtlList[cur_mtl].height/32;
                              cur_mtl = k;
                         }
                     }

                }
            }

            AddToVCache(j * 3, fList[i+j], cur_mtl);
        }

        fList[i].idx1 = 0 * 0x0A;
        fList[i].idx2 = 1 * 0x0A;
        fList[i].idx3 = 2 * 0x0A;
        fList[i+1].idx1 = 3 * 0x0A;
        fList[i+1].idx2 = 4 * 0x0A;
        fList[i+1].idx3 = 5 * 0x0A;
        fList[i+2].idx1 = 6 * 0x0A;
        fList[i+2].idx2 = 7 * 0x0A;
        fList[i+2].idx3 = 8 * 0x0A;
        fList[i+3].idx1 = 9 * 0x0A;
        fList[i+3].idx2 = 10 * 0x0A;
        fList[i+3].idx3 = 11 * 0x0A;
        fList[i+4].idx1 = 12 * 0x0A;
        fList[i+4].idx2 = 13 * 0x0A;
        fList[i+4].idx3 = 14 * 0x0A;
        WriteCache(VTX);

        LoadCache(DL);   // gspSegment  (0x04)
        LoadCache(ALPHADL);
        
        char message[4048];
        
        for (j = 0; j <= 4; j++)
        {

            if (1)  // LOL don't ask!
            {
                if ( ! (strcmp(fList[(i+j)].MtlName, fList[(i+j)-1].MtlName) == 0) )
                {

                     for (k = 0; k <= mtlCount; k++)
                     {
                         if (  strcmp(fList[(i+j)].MtlName, mtlList[k].MtlName) == 0 )
                         {
                              Gfx *glistp = malloc(50000);

                              gDPLoadTextureBlock (glistp++,mtlList[k].MtlPointer, G_IM_FMT_RGBA, G_IM_SIZ_16b, mtlList[k].width, mtlList[k].height, 0,
                              G_TX_WRAP | G_TX_NOMIRROR , G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                              
                              sprintf(message, "\nFace %d Texture = %s Alpha = %d", i+j, mtlList[k].MtlName, mtlList[k].HasAlpha);
                              DebugPrint(message);

	                          int *teste = (void*)glistp;

	                          teste -= 14;
	                          
//                         sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
	                          
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL);
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);

	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;

//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);
                              
	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "00014050", *teste);   // 4060 if 64x32 ?
//                            DebugPrint(message);
                              
                              if (mtlList[k].width == 64) 
                              {
                                  if (mtlList[k].HasAlpha == 1) Write32(0x00014060, ALPHADL); else Write32(0x00014060, DL); 
                              }
                              else if (mtlList[k].height == 64) 
                              {
                                  if (mtlList[k].HasAlpha == 1) Write32(0x00018050, ALPHADL); else Write32(0x00018050, DL); 
                              }
                              else
                              {
	                              if (mtlList[k].HasAlpha == 1) Write32(0x00014050, ALPHADL); else Write32(0x00014050, DL); 
                               }
                              teste++;
                              
//                            sprintf(message, "\nDL Command = %08x", *teste);
//                            DebugPrint(message);

	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); 
                              teste++;
                              
// 	                          sprintf(message, "%08x", *teste);
//                            DebugPrint(message);

	                          if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL);
                              free(glistp);
                               
                              if (mtlList[k].HasAlpha == 1) goto SkipFog;
                              if (FOG == 1)
                              {
                                  glistp = malloc(50000);
                                  gDPSetCycleType(glistp++,   G_CYC_2CYCLE);
                                  gDPSetFogColor(glistp++,  FOGR, FOGG, FOGB, 0xff);
                                  if (mtlList[k].HasAlpha == 0) gDPSetRenderMode(glistp++,   G_RM_FOG_SHADE_A,   G_RM_AA_ZB_OPA_SURF2);
                                  if (mtlList[k].HasAlpha == 1) gDPSetRenderMode(glistp++,   G_RM_FOG_PRIM_A,   G_RM_AA_ZB_TEX_EDGE2);
                                  gSPFogPosition(glistp++,  400, -450); // replaced by presets
                                  gSPSetGeometryMode(glistp++,  G_FOG);
                                  teste = (void*)glistp;
                                  teste -= 10;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL); teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(0xBC000008, ALPHADL); else Write32(0xBC000008, DL);  // fog preset #1
	                              teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(0x0500FC00, ALPHADL); else Write32(FogPresets[FOGPRESET], DL);
	                              teste++;
	                              
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL);  teste++;
	                              if (mtlList[k].HasAlpha == 1) Write32(*teste, ALPHADL); else Write32(*teste, DL);
	                              free(glistp);
                              
                              }
                              
                              SkipFog: ;
                               
                              if (mtlList[k].HasAlpha == 1 && alpha == 0) 
                              {
                                   fwrite(AlphaInit, sizeof(AlphaInit), 1, ALPHADL);
                                   alpha = 1;
                              }
                              
                               if (alpha == 1 && mtlList[k].HasAlpha == 0)  alpha = 0;
                               
                         }
                     }

                }
            }
            
            if (alpha==1) 
            {
                WriteTriangle(ALPHADL, fList[i+j].idx1  , fList[i+j].idx2  , fList[i+j].idx3);
                AlphaFaceCount++;
            }
            else 
            {
                 WriteTriangle(DL, fList[i+j].idx1  , fList[i+j].idx2  , fList[i+j].idx3);
            } 

        }

    }
    
    if (FOG == 1)
    {
        fwrite(FogEnd, sizeof(FogEnd), 1, DL);
        //fwrite(FogEnd, sizeof(FogEnd), 1, ALPHADL);
        
    }
    
    FinishDL(DL);
    fwrite(AlphaEnd, sizeof(AlphaEnd), 1, ALPHADL);
    FinishDL(ALPHADL);

    int VtxSize = 0;
    fseek(VTX, 0, SEEK_END);
    VtxSize = ftell(VTX);

    int DlSize = 0;
    fseek(DL, 0, SEEK_END);
    DlSize = ftell(DL);


    int LEVEL_ENTRY = 0x1200000 + LEVEL * 0xe0000;
   /* Water Boxes */
    int BoxIndex = 0;
    fseek(ROM, LEVEL_ENTRY + 0x1800, SEEK_SET);
    for (i = 1; i <= WaterCount; i++)
    {
        if ( WaterList[i].type == 0) // regular water
        {
            WaterList[i].index = BoxIndex;
            Write32(BoxIndex, ROM);
            Write32(0x19001C00 + (BoxIndex * 0x20), ROM);
            WaterList[i].pointer = 0x1C00 + (BoxIndex * 0x20);
            BoxIndex += 1;
        }
    }
    Write16(0xFFFF, ROM); // end list
    
    /* Toxic Haze */
    int ToxicIndex = 0x32;
    fseek(ROM, LEVEL_ENTRY + 0x1850, SEEK_SET);
    for (i = 1; i <= WaterCount; i++)
    {
        if ( WaterList[i].type == 1) // toxic haze;
        {
            WaterList[i].index = ToxicIndex;
            Write32(ToxicIndex, ROM);
            Write32(0x19001C00 + (BoxIndex * 0x20), ROM);
            WaterList[i].pointer = 0x1C00 + (BoxIndex * 0x20);
            BoxIndex += 1;
            ToxicIndex += 1;
        }
    }
    Write16(0xFFFF, ROM); // end list    
        
    int MistIndex = 0x33; // TODO: CHECK THIS!! possible conflicts?
    fseek(ROM, LEVEL_ENTRY + 0x18a0, SEEK_SET);    
    for (i = 1; i <= WaterCount; i++)
    {
        if ( WaterList[i].type == 2) // mist
        {
            WaterList[i].index = MistIndex;
            Write32(ToxicIndex, ROM);
            Write32(0x19001C00 + (BoxIndex * 0x20), ROM);
            WaterList[i].pointer = 0x1C00 + (BoxIndex * 0x20);
            BoxIndex += 1;
            MistIndex += 1;
        }
    }
    Write16(0xFFFF, ROM); // end list    
    
    /* Write all Box Data */
    for (i = 1; i <= WaterCount; i++)
    {
            fseek(ROM, LEVEL_ENTRY + WaterList[i].pointer, SEEK_SET);
            Write32(0x00010000, ROM);
            Write32(0x000F0003, ROM);
            
            Write16(WaterList[i].minx, ROM);
            Write16(WaterList[i].minz, ROM);
            Write16(WaterList[i].maxx, ROM);
            Write16(WaterList[i].minz, ROM);
            Write16(WaterList[i].maxx, ROM);
            Write16(WaterList[i].maxz, ROM);
            Write16(WaterList[i].minx, ROM);
            Write16(WaterList[i].maxz, ROM);

            if (WaterList[i].type == 1) Write32(0x000000B4, ROM); else Write32(0x00010078, ROM);
            if (WaterList[i].type == 1) Write32(0x00010000, ROM); else Write32(0x00000000, ROM);
    }


    int ColSize = 0;
    WriteCollision(COL);
    fseek(COL, 0, SEEK_END);
    ColSize = ftell(COL);
    
    int AlphaDlSize = 0;
    fseek(ALPHADL, 0, SEEK_END);
    AlphaDlSize = ftell(ALPHADL);
    
    int LevelSize = VtxSize + DlSize + ColSize + AlphaDlSize;

    rewind(VTX);
    rewind(DL);
    rewind(ALPHADL);

    sprintf(DebugMessage, "\nVertex data size = 0x%x bytes", VtxSize);
    DebugPrint(DebugMessage);
    sprintf(DebugMessage, "\nDisplay List data size = 0x%x bytes", DlSize);
    DebugPrint(DebugMessage);
    
    if (AlphaFaceCount > 0)
    {
        sprintf(DebugMessage, "\nAlpha Display List data size = 0x%x bytes", AlphaDlSize);
        DebugPrint(DebugMessage);
    }

    sprintf(DebugMessage, "\nCollision data size = 0x%x bytes", ColSize);
    DebugPrint(DebugMessage);

    if (LEVEL == 31) goto IntroScreen;

    if (CUSTOMBG == 0)
    {
        if ( LevelSize > 0xd0000 )
        {
            sprintf(DebugMessage,"\n[Fatal Error]\nMaximum level size exceeded!");
            DebugPrint(DebugMessage);
            exit(ERROR_SIZE);
        }
    }
    else
    {
    if ( LevelSize > 0xA7EC0 )
        {
            sprintf(DebugMessage,"\n[Fatal Error]\nMaximum level size exceeded!");
            DebugPrint(DebugMessage);
            exit(ERROR_SIZE);
        }
    }


    int VTX_ENTRY = 0x1210000 + LEVEL * 0xe0000;
    LEVEL_ENTRY = 0x1200000 + LEVEL * 0xe0000;

    fread(buffer, 1, VtxSize, VTX);
    fseek(ROM, VTX_ENTRY, SEEK_SET);
    fwrite(buffer, 1, VtxSize, ROM);

    int DLPointer = ftell(ROM) + 0x0e000000 - VTX_ENTRY;

    sprintf(DebugMessage, "\n[ROM]\nDisplay List pointer = 0x%08x (ROM 0x%x)", DLPointer, ftell(ROM));
    DebugPrint(DebugMessage);

    fread(buffer, 1, DlSize, DL);
    fwrite(buffer, 1, DlSize, ROM);

    int CollisionPointer = ftell(ROM) + 0x0e000000 - VTX_ENTRY;
    sprintf(DebugMessage, "\nCollision Pointer = 0x%08x (ROM 0x%x)", CollisionPointer, ftell(ROM));
    DebugPrint(DebugMessage);
    WriteCollision(ROM);
    
    fread(buffer, 1, AlphaDlSize, ALPHADL);
    int check;
    check = ftell(ROM);
    while ( !(check % 0x10 == 0) )
    {
         fputc(0x00, ROM); // word align ROM
         check = ftell(ROM);
    }
    int AlphaDlPointer = ftell(ROM) + 0x0e000000 - VTX_ENTRY;
    
    
    if (AlphaFaceCount > 0 )
    {
        sprintf(DebugMessage, "\nAlpha Display List pointer = 0x%08x (ROM 0x%x)", AlphaDlPointer, ftell(ROM));
        DebugPrint(DebugMessage);
    }
    sprintf(DebugMessage, "\nAlpha Faces = %d", AlphaFaceCount);
    DebugPrint(DebugMessage);
    
    fwrite(buffer, 1, AlphaDlSize, ROM);
    
    int BGSize = 0;
    if (CUSTOMBG == 1) 
    {
        int BGOk = ProcessCustomBackground(argv[3], BG);
        if (BGOk == 0) CUSTOMBG = 0;
        BGSize = ftell(BG);
        sprintf(DebugMessage, "\nBackground Bank size = 0x%x", BGSize );
        DebugPrint(DebugMessage);
        rewind(BG);
        
        fseek(ROM, 0x1202500, SEEK_SET); // 0x80402500
        fwrite(BackgroundPointerTable, sizeof(BackgroundPointerTable), 1, ROM);
     
        // change pointers for background
        fseek(ROM, 0x8AA8A, SEEK_SET); // 0x80402500
        Write16(0x8040, ROM);
        fseek(ROM, 0x8AA9A, SEEK_SET); // 0x80402500
        Write16(0x2500, ROM);
    }
    
    if (CUSTOMBG == 1)
    {
        fread(buffer, 1, BGSize, BG);                 
        fseek(ROM, VTX_ENTRY + 0xA7EC0, SEEK_SET);
        fwrite(buffer, 1, BGSize, ROM);
        
        fseek(ROM, LEVEL_ENTRY + 0x5c, SEEK_SET);
        WriteCommand17(0x0A, VTX_ENTRY + 0xA7EC0, (VTX_ENTRY + 0xA7EC0 + BGSize), ROM);
        
    }
  
    LevelSize = ftell(ROM) - VTX_ENTRY;
    sprintf(DebugMessage, "\nLevel Size = 0x%x", ( ftell(ROM) - VTX_ENTRY) + BGSize );
    DebugPrint(DebugMessage);
    

    // write pointer for 0x0010 level command (bank 0x19)
    fseek(ROM, LEVEL_LIST + (LEVEL * 0x14), SEEK_SET);
    Write32(0x00100019, ROM);
    Write32(LEVEL_ENTRY, ROM); // level script entry
    Write32(LEVEL_ENTRY + 0x2000, ROM);
    Write32(0x1900001C, ROM);
    
     sprintf(DebugMessage, "\nLevel Script Pointer(0x%x) = 0x%x - 0x%x", LEVEL_LIST + (LEVEL * 0x14), LEVEL_ENTRY, LEVEL_ENTRY + 0x2000 );
    DebugPrint(DebugMessage);


    // correct ROMS corrupted by v0.8
    fseek(ROM, LEVEL_ENTRY + 0x270, SEEK_SET);
    ch = fgetc(ROM);
    if (ch != 0x24) 
    {
        fseek(ROM, LEVEL_ENTRY + 0x270, SEEK_SET);
        unsigned char FixCorruptCommands[48] =
        {
            0x24, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x0A, 0x00, 0x00, 0x13, 0x00, 0x2F, 0x74, 0x24, 0x18, 0x1F, 0x00, 0x02, 0x47, 0x0A, 0x7B, 
            0xEA, 0xF5, 0x00, 0x00, 0xFF, 0x66, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x13, 0x00, 0x07, 0x5C, 
        } ;
        fwrite(FixCorruptCommands, sizeof(FixCorruptCommands), 1, ROM);
        
        sprintf(DebugMessage, "\nFixed ROM corrupted caused by 0.8 versions.");
        DebugPrint(DebugMessage);


    }
    
    // hack toad's tool entry point
    fseek(ROM, LEVEL_ENTRY, SEEK_SET);
    Write32(0x80080000, ROM);
    Write32(0x1900001C, ROM);

    // terrain layout pointer
    fseek(ROM, LEVEL_ENTRY + 0x268, SEEK_SET);
    Write32(0x1F080100, ROM);
    Write32(0x19000008, ROM);

    fseek(ROM, LEVEL_ENTRY + 0x1C, SEEK_SET);
    Write32(0x1B040000, ROM); // level script entry

    // collision/dl pointers
    fseek(ROM, LEVEL_ENTRY + 0x116c, SEEK_SET);
    Write32(CollisionPointer, ROM);
    
    //terain
    fseek(ROM, LEVEL_ENTRY + 0x268, SEEK_SET);
    Write32(0x1F080100, ROM); // area 1
    Write32(0x19001700, ROM); // pointer for terrain geolayout 
    
    fseek(ROM, LEVEL_ENTRY + 0x1700, SEEK_SET);
    WriteTerrainLayout(ROM, DLPointer, 1, AlphaDlPointer);
    
    fseek(ROM, LEVEL_ENTRY + 0x174B, SEEK_SET); // camera setting
    fputc(CAMERA, ROM);
    
    if (CUSTOMBG == 1)
    {
        fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
        fputc(0x0A, ROM);
    }
    

    // write pointers for Bank 0x0e (polygons)
    fseek(ROM, LEVEL_ENTRY + 0x44, SEEK_SET);
    WriteCommand17(0x0e, VTX_ENTRY, VTX_ENTRY + LevelSize + 0xD0000, ROM);
    //WriteCommand17(0x0e, VTX_ENTRY, VTX_ENTRY + LevelSize + BGSize + 0xD0000, ROM);

    // weather effect
     fseek(ROM, LEVEL_ENTRY + 0x38, SEEK_SET);
     WriteCommand17(0x0b, 0x00DA2785, 0x00DA951D, ROM);

     if (AlphaFaceCount > 0) fseek(ROM, LEVEL_ENTRY + 0x1777, SEEK_SET);
     else fseek(ROM, LEVEL_ENTRY + 0x176F, SEEK_SET);
     switch (WEATHER)
     {
            case 0:  fputc(0x00, ROM); break;
            case 1:  fputc(0x01, ROM); break;
            case 2:  fputc(0x03, ROM); break;
            case 3:  fputc(0x0b, ROM); break;
            case 4:  fputc(0x0c, ROM); break;
            case 5:  fputc(0x0d, ROM); break;
            case 6:  fputc(0x02, ROM); break;
     }
     
     if (SETMARIOPOS == 1)
     {
         fseek(ROM, LEVEL_ENTRY + 0x118E, SEEK_SET);
         Write16(MARIOX, ROM);
         Write16(MARIOY, ROM);
         Write16(MARIOZ, ROM);
         // what about rotation?
     }

     // background:
    fseek(ROM, LEVEL_ENTRY + 0x1724, SEEK_SET);
    Write32(0x802763D4, ROM);
    
    if (CUSTOMBG == 1) goto SkipBG;

    fseek(ROM, LEVEL_ENTRY + 0x5c, SEEK_SET);
    Write32(0x170C000A, ROM);
    switch (BACKGROUND)
    {
           case 0:
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0, ROM);
               Write32(0x00000000, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
           break;

           case 1:
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x06, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00C3AFD5, ROM);
               Write32(0x00C4F915, ROM);
           break;

           case 2:
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x04, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00B5D855, ROM);
               Write32(0x00B7D995, ROM);
           break;

           case 3: // ss land
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x05, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00C12E95, ROM);
               Write32(0x00C32FD5, ROM);
           break;

           case 4: // BOB
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x00, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00B35715, ROM);
               Write32(0x00B55855, ROM);
           break;

           case 5: // WD
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x02, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00BC2C15, ROM);
               Write32(0x00BE2D55, ROM);
           break;

           case 6: // JLRB
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x08, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00B85995, ROM);
               Write32(0x00B9A2D5, ROM);
           break;

           case 7: // RBW
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x03, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00BEAD55, ROM);
               Write32(0x00C0AE95, ROM);
           break;

           case 8: // RBW
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x07, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00C57915, ROM);
               Write32(0x00C77A55, ROM);
           break;

           case 9: // LAVA
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x01, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00BA22D5, ROM);
               Write32(0x00BBAC15, ROM);
           break;

           case 10: // BOWSER3
               fseek(ROM, LEVEL_ENTRY + 0x1723, SEEK_SET);
               fputc(0x09, ROM);
               fseek(ROM, LEVEL_ENTRY + 0x60, SEEK_SET);
               Write32(0x00C7FA55, ROM);
               Write32(0x00C9FB95, ROM);
           break;
    }
    
    SkipBG: ;

    fseek(ROM, LEVEL_ENTRY + 0x98, SEEK_SET);
    WriteCommand17(0x08, 0x00A8181C, 0x00AAA40C, ROM);
    WriteCommand17(0x0F, 0x002008D0, 0x00201410, ROM);
    
    unsigned char LevelData[16] = { 0x1D, 0x04, 0x00, 0x00, 0x25, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x13, 0x00, 0x2E, 0xC0,  } ;
    fwrite(LevelData, sizeof(LevelData), 1, ROM);

    //music
    fseek(ROM, LEVEL_ENTRY + 0x1179, SEEK_SET);
    fputc(MUSIC, ROM);

    //terain
    fseek(ROM, LEVEL_ENTRY + 0x117F, SEEK_SET);
    fputc(TERRAIN, ROM);

    // bank pointers
    Clear22Commands(ROM);
    WriteBankPointers(BANKC, BANKD, BANKE, ROM);

    CommonExitTasks:
    fclose(VTX);
    fclose(DL);
    fclose(COL);
    fclose(ROM);
    fclose(DebugLog);
    fclose(SETTINGS);
    free(buffer);

    remove("col");
    remove("vtx");
    remove("dl");
    remove("alphadl");
    remove("bg");

    int exit_flags = 0;

    if (OUT_OF_RANGE == 1) exit_flags = exit_flags | 0x1;
    if (MTL_NOT_FOUND == 1) exit_flags = exit_flags | 0x10;
    if (VT_OUT_OF_RANGE == 1) exit_flags = exit_flags | 0x40;
    if (CVT_PROBLEM == 1) exit_flags = exit_flags | 0x80;
    if (MANY_TEXTURES == 1 && NOMTL == 0 && MTL_NOT_FOUND == 0)  exit_flags = exit_flags | 0x100;
    if (BIG_TEXTURES == 1)  exit_flags = exit_flags | 0x200;
    if (TEXTURE_NOT_FOUND == 1)  exit_flags = exit_flags | 0x800;
    if (vtCount == 0 ) exit_flags = exit_flags | 0x400;

    return (SUCESS | exit_flags);

    IntroScreen:

    if ( (DlSize + VtxSize) > 0x436C0 )
    {
         sprintf(DebugMessage, "\n[Fatal Error]\nMaximum Intro screen size exceeded!");
         DebugPrint(DebugMessage);
         exit(ERROR_SIZE);
    }

    fseek(ROM, 0x11B0000, SEEK_SET);
    fwrite(IntroScreen, sizeof(IntroScreen), 1, ROM);

    fread(buffer, 1, VtxSize, VTX);
    fwrite(buffer, 1, VtxSize, ROM);

    int ScreenPtr = (ftell(ROM) - 0x11B0000) + 0x07000000;
    sprintf(DebugMessage,"\n[ROM]\nDisplay List pointer = 0x%08x", ScreenPtr);
    DebugPrint(DebugMessage);

    fread(buffer, 1, DlSize, DL);
    fwrite(buffer, 1, DlSize, ROM);

    fseek(ROM, 0x11BB3D0, SEEK_SET); // DL pointer for intro screen
    Write32(0x06000000, ROM);
    //Write32(ScreenPtr, ROM);  // To Do: Fix this (recheck whole section)
    Write32(0x0700c6a0, ROM);
    Write32(0xBB000000, ROM);
    Write32(0xFFFFFFFF, ROM);
    Write32(0xE7000000, ROM);
    Write32(0x00000000, ROM);
    Write32(0xFCFFFFFF, ROM);
    Write32(0xFFFE793C, ROM);
    Write32(0x06000000, ROM);
    Write32(ScreenPtr, ROM);

    Write32(0xB7000000, ROM);
    Write32(0x00020000, ROM);
    Write32(0xB8000000, ROM);
    Write32(0x00000000, ROM);

    fseek(ROM, 0x269EC0, SEEK_SET); // command 0x17 to load bank 0x07 for intro screen
    Write32(0x170c0107, ROM);
    Write32(0x011B0000, ROM);
    Write32(0x01200000, ROM);

    goto CommonExitTasks;
}

void ParseMTL(char *mtllib_file, FILE *vtx)
{
    FILE *mtl = fopen(mtllib_file, "r");
    char line[2048];
    char MtlName[256];
    char KdName[256];
    char ShellCommand[1024];
    char ShellParams[2048];
    int returnvalue = 0;
    int wait = 0;
    
    sprintf(ObjFilePath, "\"%s\"", ObjFilePath); // include quotation marks into ObjFilePath to use it as working directory in ShellExecute

    if (!mtl)
    {
       sprintf(DebugMessage, "\n[Error]\nMaterial library %s not found! Using textures from the game instead", mtllib_file);
       DebugPrint(DebugMessage);
       MTL_NOT_FOUND = 1;
       goto GiveUp;
    }

    while (fgets(line, 2048, mtl))
    {

        char *ptr = (void*)0;
        char *ptr2 = (void*)0;

        if (ptr = strstr(line,"collisionType ") )
        {
            int CParam1 = 0;
            int CParam2 = 0;
            int CRet = 0;
            
            CRet = sscanf(ptr+14, "%d %d %d", &mtlList[mtlCount].CollisionType, &CParam1, &CParam2);
            
            if (CRet == 1)  // old format (no collision params specified)
            {
                CParam1 = 0;
                CParam2 = 0;
            }
            
            mtlList[mtlCount].CParams = (CParam1 << 8) + CParam2;
            
            sprintf(DebugMessage, "\n[Material Collision]\nCollisionType = %d CParams = %x", mtlList[mtlCount].CollisionType, mtlList[mtlCount].CParams);
            DebugPrint(DebugMessage);

        }
        if (ptr = strstr(line,"newmtl ") )
        {
            mtlCount++;
            strcpy(MtlName, ReadString(ptr+7));
            sprintf(DebugMessage, "\nMaterial #%d = %s", mtlCount, MtlName);
            DebugPrint(DebugMessage);
            strcpy(mtlList[mtlCount].MtlName, MtlName);
        }
        // mapKd
        else if ( (ptr = strstr(line,"map_Kd ")) || (ptr = strstr(line,"map_Ka ")) || (ptr = strstr(line,"map_kd ")) || (ptr = strstr(line,"map_ka ")) || (ptr = strstr(line,"mapKd ")) || (ptr = strstr(line,"mapkd ")))
        {
            char FileExtension[256];
            char PngExtension[24];
            int ProcessAlpha = 0;
            
            // mapKd hack
            if (  ( ptr2 = strstr(line,"mapKd ") ) || ( ptr2 = strstr(line,"mapkd ") ) )
            {
                strcpy(KdName, ReadString(ptr+6));
            }
            else
            {
                strcpy(KdName, ReadString(ptr+7));
            }
            
            _splitpath(KdName, NULL, NULL, NULL, FileExtension);
            strcpy(PngExtension, ".png");
            
            if ( strcmpi(FileExtension, PngExtension) == 0 ) ProcessAlpha = 1; // if file is PNG, try to read alpha
           
            sprintf(DebugMessage, "\nTexture File = %s", KdName);
            DebugPrint(DebugMessage);

            FILE *texture = fopen(KdName, "rb");
            if (!texture)
            {
                sprintf(DebugMessage, "\n[Warning]\nError loading file %s - using texture from the game", KdName);
                DebugPrint(DebugMessage);
                TEXTURE_NOT_FOUND = 1;
                goto Stop;
            }

            if ( CheckBMP(texture) && (CheckBMPsize(texture) == 1) )  // miracle! File is in BMP format in correct size
            {
                sprintf(ShellCommand, "\"%s\"", CvtExePath);
                sprintf(ShellParams, "\"%s\" -channel matte -separate +channel -negate \"BMP3:%salpha.bmp\"", KdName, KdName);
                
                SHELLEXECUTEINFO process = { sizeof(process) };
                process.fMask = SEE_MASK_NOCLOSEPROCESS;
                process.nShow = SW_HIDE;
                process.lpVerb = "open";
                process.lpFile = ShellCommand;
                process.lpParameters = ShellParams;
                process.lpDirectory = ObjFilePath;
                wait = ShellExecuteEx(&process);
                if (wait == 1) WaitForSingleObject(process.hProcess, INFINITE);
                
                DebugPrint("\n[Alpha Mask]\nShell Execute = ");
                DebugPrint(ShellCommand);
                DebugPrint(" ");
                DebugPrint(ShellParams);
                
                char alphatexture[256];
                sprintf(alphatexture, "%salpha.bmp", KdName);
                FILE *alpha = fopen(alphatexture, "rb");
                if (!alpha)
                {
                    sprintf(DebugMessage, "\n[Warning]\nError opening alpha mask (%s) - skipping texture", alphatexture);
                    DebugPrint(DebugMessage);
                    CVT_PROBLEM = 1;
                    goto Stop;
                }
                
                if ( CheckAlpha(alpha) && ProcessAlpha == 1)
                {
                    sprintf(DebugMessage, "\nAlpha Channel found in Texture %s", KdName);
                    mtlList[mtlCount].HasAlpha = 1;
                    DebugPrint(DebugMessage);
                }
                
                if (ProcessAlpha == 1) mtlList[mtlCount].MtlPointer = BMPtoRGBA(texture, vtx, alpha, 1);
                else if (ProcessAlpha == 0 ) mtlList[mtlCount].MtlPointer = BMPtoRGBA(texture, vtx, NULL, 0);
                
                fclose(texture);
                fclose(alpha);
                remove(alphatexture);
            }
            else  // not a .bmp file
            {
                char newtexturename[256];
                char newtexturename2[256];
                fclose(texture);
                
                DebugPrint("\n[BMP conversion]\nShellExecute = ");
                sprintf(ShellCommand, "\"%s\"", CvtExePath);
                sprintf(ShellParams, "\"%s\" \"BMP3:%s.bmp\"", KdName, KdName);
                DebugPrint(ShellCommand);
                DebugPrint(" ");
                DebugPrint(ShellParams);
                
                SHELLEXECUTEINFO process = { sizeof(process) };
                process.fMask = SEE_MASK_NOCLOSEPROCESS;
                process.nShow = SW_HIDE;
                process.lpVerb = "open";
                process.lpFile = ShellCommand;
                process.lpParameters = ShellParams;
                process.lpDirectory = ObjFilePath;
                wait = ShellExecuteEx(&process);
                if (wait == 1) WaitForSingleObject(process.hProcess, INFINITE);

                sprintf(newtexturename, "%s.bmp", KdName);
                FILE *newtexture = fopen(newtexturename, "rb");

                if (newtexture) // conversion sucessfull
                {
                    if ( CheckBMPsize(newtexture) == 1) // adequate size
                    {
                        // alpha stuff
                        
                        sprintf(ShellCommand, "\"%s\"", CvtExePath);
                        sprintf(ShellParams, "\"%s\" -channel matte -separate +channel -negate \"BMP3:%salpha.bmp\"", KdName, KdName);

                        SHELLEXECUTEINFO process = { sizeof(process) };
                        process.fMask = SEE_MASK_NOCLOSEPROCESS;
                        process.nShow = SW_HIDE;
                        process.lpVerb = "open";
                        process.lpFile = ShellCommand;
                        process.lpParameters = ShellParams;
                        process.lpDirectory = ObjFilePath;
                        wait = ShellExecuteEx(&process);
                        if (wait == 1) WaitForSingleObject(process.hProcess, INFINITE);

                        DebugPrint("\n[Alpha Mask]\nShell Execute = ");
                        DebugPrint(ShellCommand);
                        DebugPrint(" ");
                        DebugPrint(ShellParams);
                        
                        char alphatexture[256];
                        sprintf(alphatexture, "%salpha.bmp", KdName);
                        FILE *alpha = fopen(alphatexture, "rb");
                        if (!alpha)
                        {
                            sprintf(DebugMessage, "\n[Warning]\nError opening alpha mask (%s) - skipping texture", alphatexture);
                            DebugPrint(DebugMessage);
                            CVT_PROBLEM = 1;
                            goto Stop;
                        }

                        if ( CheckAlpha(alpha) && ProcessAlpha == 1 )
                        {
                            mtlList[mtlCount].HasAlpha = 1;
                        }

                        if (ProcessAlpha == 1) mtlList[mtlCount].MtlPointer = BMPtoRGBA(newtexture, vtx, alpha, 1);
                        else if (ProcessAlpha == 0 ) mtlList[mtlCount].MtlPointer = BMPtoRGBA(newtexture, vtx, NULL, 0);
                        
                        fclose(newtexture);
                        fclose(alpha);
                        remove(alphatexture);
                    }
                    else if ( CheckBMPsize(newtexture) == 666 )
                    {
                        BIG_TEXTURES = 1;
                        fseek(newtexture, 0x12, SEEK_SET);
                        int width = Read32(newtexture);
                        fseek(newtexture, 0x16, SEEK_SET);
                        int height = Read32(newtexture);
                        
                        if (width == 8 && height == 8 || width == 16 && height == 16 || width == 64 && height == 64 || width == 128 && height == 128)
                        {
                            width = 32;
                            height = 32;
                            goto Jumanji;
                        }

                        while( (width*height*2) > 4096 )   // divide by 2 while result fits doesn't fit texture cache
                        { 
                            width = width/2;
                            height = height/2;
                        }

                        if (width%2) width -= 1;   // make it an even number
                        if (height%2) height -= 1;
                        if (width == 1) width=2;  // minimum size
                        if (height == 1) width=2;
                        
                        Jumanji: ;
                        
                        fclose(newtexture);
                        
                        sprintf(ShellCommand, "\"%s\"", CvtExePath);
                        sprintf(ShellParams, "-resize %dx%d \"%s.bmp\" \"BMP3:%s.bmp\"", width, height, KdName, KdName);
                        
                        SHELLEXECUTEINFO process = { sizeof(process) };
                        process.fMask = SEE_MASK_NOCLOSEPROCESS;
                        process.nShow = SW_HIDE;
                        process.lpVerb = "open";
                        process.lpFile = ShellCommand;
                        process.lpParameters = ShellParams;
                        process.lpDirectory = ObjFilePath;
                        wait = ShellExecuteEx(&process);
                        if (wait == 1) WaitForSingleObject(process.hProcess, INFINITE);
                        DebugPrint("\n[BMP Resizing]\nShellExecute = ");
                        DebugPrint(ShellCommand);
                        DebugPrint(" ");
                        DebugPrint(ShellParams);

                        sprintf(newtexturename2, "%s.bmp", KdName);
                        newtexture = fopen(newtexturename2, "rb");
                        
                        // alpha stuff
                        sprintf(ShellCommand, "\"%s\"", CvtExePath);
                        sprintf(ShellParams, "\"%s\" -resize %dx%d -channel matte -separate +channel -negate \"BMP3:%salpha.bmp\"", KdName, width, height, KdName);
                        
                        SHELLEXECUTEINFO process2 = { sizeof(process2) };
                        process2.fMask = SEE_MASK_NOCLOSEPROCESS;
                        process2.nShow = SW_HIDE;
                        process2.lpVerb = "open";
                        process2.lpFile = ShellCommand;
                        process2.lpParameters = ShellParams;
                        process2.lpDirectory = ObjFilePath;
                        wait = ShellExecuteEx(&process2);
                        if (wait == 1) WaitForSingleObject(process2.hProcess, INFINITE);
                        
                        DebugPrint("\n[Alpha Mask]\nShell Execute = ");
                        DebugPrint(ShellCommand);
                        DebugPrint(" ");
                        DebugPrint(ShellParams);

                        char alphatexture[256];
                        sprintf(alphatexture, "%salpha.bmp", KdName);
                        FILE *alpha = fopen(alphatexture, "rb");
                        if (!alpha)
                        {
                            sprintf(DebugMessage, "\n[Warning]\nError opening alpha mask (%s) - skipping texture", alphatexture);
                            DebugPrint(DebugMessage);
                            CVT_PROBLEM = 1;
                            goto Stop;
                        }

                        if ( CheckAlpha(alpha) && ProcessAlpha == 1 )
                        {
                            sprintf(DebugMessage, "\nAlpha Channel found in Texture %s", KdName);
                            mtlList[mtlCount].HasAlpha = 1;
                            DebugPrint(DebugMessage);
                        }

                        if (ProcessAlpha == 1) mtlList[mtlCount].MtlPointer = BMPtoRGBA(newtexture, vtx, alpha, 1);
                        else if (ProcessAlpha == 0 ) mtlList[mtlCount].MtlPointer = BMPtoRGBA(newtexture, vtx, NULL, 0);

                        fclose(newtexture);
                        fclose(alpha);
                        remove(alphatexture);
                    }

                    remove(newtexturename);
                    remove(newtexturename2);
                }
                else
                {
                    sprintf(DebugMessage, "\n[Warning]\nError opening texture \"%s.bmp\"", KdName);
                    DebugPrint(DebugMessage);
                }
            }

            fclose(texture);

            Stop: ;

        }
        else if (ptr = strstr(line,"Ka ") )
		{
	  	    sscanf(ptr+3, "%f %f %f", &mtlList[mtlCount].Ka[0], &mtlList[mtlCount].Ka[1], &mtlList[mtlCount].Ka[2]);
        }
        else if (ptr = strstr(line,"Kd ") )
		{
	  	    sscanf(ptr+3, "%f %f %f", &mtlList[mtlCount].Kd[0], &mtlList[mtlCount].Kd[1], &mtlList[mtlCount].Kd[2]);
        }
        else if (ptr = strstr(line,"Ks ") )
		{
	  	    sscanf(ptr+3, "%f %f %f", &mtlList[mtlCount].Ks[0], &mtlList[mtlCount].Ks[1], &mtlList[mtlCount].Ks[2]);
        }

    }   // while loop

    fclose(mtl);

    GiveUp: ;
}

int CheckBMP(FILE *bmp)
{
    unsigned char magic1, magic2;

    fseek(bmp, 0, SEEK_SET);
    magic1 = fgetc(bmp);
    magic2 = fgetc(bmp);

    if (magic1 == 0x42 && magic2 == 0x4D) return 1; /* BM */
    else return 0;

}

int CheckBMPsize (FILE *bmp)
{
     int width;
     int height;
     int bits;

     fseek(bmp, 0x12, SEEK_SET);
     width = Read32(bmp);
     fseek(bmp, 0x16, SEEK_SET);
     height = Read32(bmp);
     fseek(bmp, 0x1c, SEEK_SET);
     bits = fgetc(bmp);


     if ( (width*height*2) > 4096 )
     {
          return 666;
     }
     
     if (width == 8 && height == 8 || width == 16 && height == 16 || width == 64 && height == 64 || width == 128 && height == 128) return 666;

     if ( bits != 24) return 0;

     return 1;

}

int CheckAlpha(FILE* alphamask)
{

     int width;
     int height;
     int pointer;
     unsigned char r, g, b;
     int x,y;
     int heightIndex = 0;
     unsigned int dataoffset;
     int check;
     int padding;
     int AlphaFound = 0;

     fseek(alphamask, 0x12, SEEK_SET);
     width = Read32(alphamask);
     fseek(alphamask, 0x16, SEEK_SET);
     height = Read32(alphamask);
     fseek(alphamask, 0x0a, SEEK_SET);
     dataoffset = fgetc(alphamask); // ToDo : actually 32bits stored little-endian

     padding = 4 - ((width * 3) % 4);
     if (padding == 4) padding = 0;

     fseek(alphamask, dataoffset, SEEK_SET);

     unsigned short buffer2[width*height];
     unsigned short rgba[width*height];

     int i = 0;

     for( y=0; y<height; ++y)
     {
         for( x=0; x<width; ++x)
         {
              b = fgetc(alphamask);
              g = fgetc(alphamask);
              r = fgetc(alphamask);

              if ( ! (b == 0xFF || b == 0xFE && g == 0xFF || g == 0xFE && r == 0xFF || r == 0xFE)) 
              {
                   AlphaFound = 1;
              }

         }
         fseek(alphamask, padding, SEEK_CUR);

     }
     
     
     if (AlphaFound) return 1;
     else return 0;

}

int BMPtoRGBA(FILE* bmp, FILE* vtx, FILE* alphamask, int doAlpha)
{

     int width;
     int height;
     int pointer;
     unsigned char r, g, b, a, a1, a2, a3;
     int x,y;
     int heightIndex = 0;
     unsigned int dataoffset;
     int check;
     int padding;

     fseek(bmp, 0x12, SEEK_SET);
     width = Read32(bmp);
     fseek(bmp, 0x16, SEEK_SET);
     height = Read32(bmp);
     fseek(bmp, 0x0a, SEEK_SET);
     dataoffset = fgetc(bmp); // ToDo : actually 32bits stored little-endian
     //sprintf(DebugMessage, "\n.BMP DataOffset = 0x%x", dataoffset);
     //DebugPrint(DebugMessage);

     mtlList[mtlCount].width = width;
     mtlList[mtlCount].height = height;

     padding = 4 - ((width * 3) % 4);
     if (padding == 4) padding = 0;

     a = 1;

     fseek(bmp, dataoffset, SEEK_SET);
     if (doAlpha) fseek(alphamask, dataoffset, SEEK_SET);

     unsigned short buffer2[width*height];
     unsigned short rgba[width*height];

     int i = 0;

     for( y=0; y<height; ++y)
     {
         for( x=0; x<width; ++x)
         {
              b = fgetc(bmp);
              g = fgetc(bmp);
              r = fgetc(bmp);
              
              if (doAlpha) 
              {
                  a1 = fgetc(alphamask);
                  a2 = fgetc(alphamask);
                  a3 = fgetc(alphamask);
                  if ( ! (a1 == 0xFF || a1 == 0xFE && a2 == 0xFF || a2 == 0xFE && a3 == 0xFF || a3 == 0xFE)) a = 0;
              }

              buffer2[y*width + x] = ((((r)<<8) & 0xf800) |  (((g)<<3) & 0x7c0) | (((b)>>2) & 0x3e) | ((a) & 0x1));
             
              a = 1;

         }
         fseek(bmp, padding, SEEK_CUR);
         if (doAlpha) fseek(alphamask, padding, SEEK_CUR);

     }


    // flip upside down (?)
    for( y=height-1;  y>=0; --y)
    {

           for( x=0; x<width; ++x) rgba[heightIndex*width + x] = buffer2[y*width + x];

         ++heightIndex;
    }

    check = ftell(vtx);
    while ( !(check % 0x10 == 0) )
    {
        fputc(0x00, vtx); // pad for correct alignment
        check = ftell(vtx);
    }

    pointer = ftell(vtx) + SEGMENT;

    sprintf(DebugMessage, "\n(%dx%d) Texture Pointer = 0x%08x", width, height, pointer);
    DebugPrint(DebugMessage);

    for (i = 0; i < (width*height); i++)
    {
        if (FLIPTEXTURES == 1) Write16( rgba[i], vtx); 
        else Write16( buffer2[i], vtx); 
    }

    return pointer;

}

int FlippedBMPtoRGBA(FILE* bmp, FILE* vtx, FILE* alphamask, int doAlpha)
{

     int width;
     int height;
     int pointer;
     unsigned char r, g, b, a, a1, a2, a3;
     int x,y;
     int heightIndex = 0;
     unsigned int dataoffset;
     int check;
     int padding;

     fseek(bmp, 0x12, SEEK_SET);
     width = Read32(bmp);
     fseek(bmp, 0x16, SEEK_SET);
     height = Read32(bmp);
     fseek(bmp, 0x0a, SEEK_SET);
     dataoffset = fgetc(bmp); // ToDo : actually 32bits stored little-endian
     //sprintf(DebugMessage, "\n.BMP DataOffset = 0x%x", dataoffset);
     //DebugPrint(DebugMessage);

     mtlList[mtlCount].width = width;
     mtlList[mtlCount].height = height;

     padding = 4 - ((width * 3) % 4);
     if (padding == 4) padding = 0;

     a = 1;

     fseek(bmp, dataoffset, SEEK_SET);
     if (doAlpha) fseek(alphamask, dataoffset, SEEK_SET);

     unsigned short buffer2[width*height];
     unsigned short rgba[width*height];

     int i = 0;

     for( y=0; y<height; ++y)
     {
         for( x=0; x<width; ++x)
         {
              b = fgetc(bmp);
              g = fgetc(bmp);
              r = fgetc(bmp);
              
              if (doAlpha) 
              {
                  a1 = fgetc(alphamask);
                  a2 = fgetc(alphamask);
                  a3 = fgetc(alphamask);
                  if ( ! (a1 == 0xFF || a1 == 0xFE && a2 == 0xFF || a2 == 0xFE && a3 == 0xFF || a3 == 0xFE)) a = 0;
              }

              buffer2[y*width + x] = ((((r)<<8) & 0xf800) |  (((g)<<3) & 0x7c0) | (((b)>>2) & 0x3e) | ((a) & 0x1));
             
              a = 1;

         }
         fseek(bmp, padding, SEEK_CUR);
         if (doAlpha) fseek(alphamask, padding, SEEK_CUR);

     }


    // flip upside down (?)
    for( y=height-1;  y>=0; --y)
    {

           for( x=0; x<width; ++x) rgba[heightIndex*width + x] = buffer2[y*width + x];

         ++heightIndex;
    }

    check = ftell(vtx);
    while ( !(check % 0x10 == 0) )
    {
        fputc(0x00, vtx); // pad for correct alignment
        check = ftell(vtx);
    }

    pointer = ftell(vtx) + SEGMENT;

    sprintf(DebugMessage, "\n(%dx%d) Texture Pointer = 0x%08x", width, height, pointer);
    DebugPrint(DebugMessage);

    for (i = 0; i < (width*height); i++)
    {
        Write16( rgba[i], vtx); 
    }

    return pointer;

}


char* ReadString(char *ptr)
{
     static char String[2048];
     int HasSpace = 0;
     int idx = 0;

    sscanf(ptr, "%s", String);

     /* check for spaces */
     while (ptr[idx] != '\n')
     {
         if ( ptr[idx] == 0x20 ) HasSpace = 1;
         idx++;
    }

    /* rewrite string if spaces are found */
    if (HasSpace == 1)
    {
        idx = 0;
        while (ptr[idx] != '\n')
        {
            String[idx] = ptr[idx];
            idx++;
        }
        String[idx] = '\0'; /* add null terminator */
    }

     return String;
}


void InitMTL()
{
    int i = 0;

    for (i = 0; i <= MAX_MTL; i++)
    {
        mtlList[i].width = 32;
        mtlList[i].height = 32;
        mtlList[i].Ka[0] = 0;
        mtlList[i].Ka[1] = 0;
        mtlList[i].Ka[2] = 0;
        mtlList[i].Kd[0] = 0;
        mtlList[i].Kd[1] = 0;
        mtlList[i].Kd[2] = 0;
        mtlList[i].Ks[0] = 0;
        mtlList[i].Ks[1] = 0;
        mtlList[i].Ks[2] = 0;
        mtlList[i].HasAlpha = 0;
    }


    if (LEVEL == 31) // title screen
    {
        for (i = 0; i < MAX_MTL; i++)
        {
            strcpy(mtlList[i].MtlName, "grass");
            mtlList[i].MtlPointer = 0x070086a0;
            mtlList[i].CollisionType = 0;
        }
    }
    else
    {
        for (i = 0; i < 10; i++)
        {
            strcpy(mtlList[i].MtlName, "grass");
            mtlList[1 + (i*16)].MtlPointer = 0x09005800;
            mtlList[2 + (i*16)].MtlPointer = 0x09009800;
            mtlList[3 + (i*16)].MtlPointer = 0x09006000;
            mtlList[4 + (i*16)].MtlPointer = 0x09003800;
            mtlList[5 + (i*16)].MtlPointer = 0x09002000;
            mtlList[6 + (i*16)].MtlPointer = 0x09001800;
            mtlList[7 + (i*16)].MtlPointer = 0x09005000;
            mtlList[8 + (i*16)].MtlPointer = 0x09004800;
            mtlList[9 + (i*16)].MtlPointer = 0x09003000;
            mtlList[10 + (i*16)].MtlPointer = 0x09008800;
            mtlList[11 + (i*16)].MtlPointer = 0x09009000;
            mtlList[12 + (i*16)].MtlPointer = 0x0900A000;
            mtlList[13 + (i*16)].MtlPointer = 0x09005000;
            mtlList[14 + (i*16)].MtlPointer = 0x09001000;
            mtlList[15 + (i*16)].MtlPointer = 0x09007000;
            mtlList[16 + (i*16)].MtlPointer = 0x09000800;
            mtlList[17 + (i*17)].MtlPointer = 0x0900A800;
            mtlList[18 + (i*18)].MtlPointer = 0x09004000;
            mtlList[19 + (i*19)].MtlPointer = 0x09002800;
            mtlList[20 + (i*20)].MtlPointer = 0x09008000;
            mtlList[21 + (i*21)].MtlPointer = 0x09000000;
            mtlList[22 + (i*22)].MtlPointer = 0x09007800;
         }
    }

}

void CenterVertexList()
{
    int i = 0;
    float AverageX = 0;
    float AverageY = 0;
    float AverageZ = 0;

    for (i = 1; i <= vCount; i++)
    {
        AverageX += vList[i].x;
        AverageY += vList[i].y;
        AverageZ += vList[i].z;
    }

    AverageX = AverageX / vCount;
    AverageY = AverageY / vCount;
    AverageZ = AverageZ / vCount;
    sprintf(DebugMessage, "\n[ObjFile]\nAverage X = %f\nAverage Y = %f\nAverage Z = %f", AverageX*SCALING, AverageY*SCALING, AverageZ*SCALING);
    DebugPrint(DebugMessage);

    for (i = 1; i <= vCount; i++)
    {
        vList[i].x -= AverageX;
        vList[i].y -= AverageY;
        vList[i].z -= AverageZ;
    }

}

void RestoreLevelPointer(FILE *ROM)
{

     if (LEVEL == 31)
     {
        fseek(ROM, 0x269EC0, SEEK_SET); // command 0x17 to load bank 0x07 for intro screen
        Write32(0x170C0007, ROM);
        Write32(0x00AED714, ROM);
        Write32(0x00AFA054, ROM);
        return;
     }

     char message[2048];
     sprintf(message, "\nRestoring Level = %d", LEVEL);
     DebugPrint(message);
     sprintf(message, "\nROM Position = 0x%x", LEVEL_LIST + (LEVEL * 0x14));
     DebugPrint(message);

     fseek(ROM, LEVEL_LIST + (LEVEL * 0x14), SEEK_SET);
     Write32(0x0010000E, ROM);
     fseek(ROM, LEVEL_LIST + (LEVEL * 0x14) + 0x04, SEEK_SET);

     switch (LEVEL)
     {
         case  0: Write32(0x003828C0, ROM); Write32(0x00383950, ROM); Write32(0x0E000418, ROM); break;
         case  1: Write32(0x00395C90, ROM); Write32(0x00396340, ROM); Write32(0x0E000178, ROM); break;
         case  2: Write32(0x003CF0D0, ROM); Write32(0x003D0DC0, ROM); Write32(0x0E000AF8, ROM); break;
         case  3: Write32(0x003E6A00, ROM); Write32(0x003E76B0, ROM); Write32(0x0E000388, ROM); break;
         case  4: Write32(0x003FB990, ROM); Write32(0x003FC2B0, ROM); Write32(0x0E0002B8, ROM); break;
         case  5: Write32(0x00405A60, ROM); Write32(0x00405FB0, ROM); Write32(0x0E000264, ROM); break;
         case  6: Write32(0x0040E840, ROM); Write32(0x0040ED70, ROM); Write32(0x0E000100, ROM); break;
         case  7: Write32(0x00419F90, ROM); Write32(0x0041A760, ROM); Write32(0x0E000370, ROM); break;
         case  8: Write32(0x00423B20, ROM); Write32(0x004246D0, ROM); Write32(0x0E0006EC, ROM); break;
         case  9: Write32(0x0042C6E0, ROM); Write32(0x0042CF20, ROM); Write32(0x0E000290, ROM); break;
         case 10: Write32(0x00437400, ROM); Write32(0x00437870, ROM); Write32(0x0E0000C8, ROM); break;
         case 11: Write32(0x0044A140, ROM); Write32(0x0044ABC0, ROM); Write32(0x0E0003E4, ROM); break;
         case 12: Write32(0x004545E0, ROM); Write32(0x00454E00, ROM); Write32(0x0E000508, ROM); break;
         case 13: Write32(0x0045BF60, ROM); Write32(0x0045C600, ROM); Write32(0x0E000174, ROM); break;
         case 14: Write32(0x00461220, ROM); Write32(0x004614D0, ROM); Write32(0x0E0000CC, ROM); break;
         case 15: Write32(0x0046A840, ROM); Write32(0x0046B090, ROM); Write32(0x0E000234, ROM); break;
         case 16: Write32(0x0046C1A0, ROM); Write32(0x0046C3A0, ROM); Write32(0x0E000050, ROM); break;
         case 17: Write32(0x00477D00, ROM); Write32(0x004784A0, ROM); Write32(0x0E0001E8, ROM); break;
         case 18: Write32(0x0048C9B0, ROM); Write32(0x0048D930, ROM); Write32(0x0E00067C, ROM); break;
         case 19: Write32(0x00495A60, ROM); Write32(0x00496090, ROM); Write32(0x0E00026C, ROM); break;
         case 20: Write32(0x0049DA50, ROM); Write32(0x0049E710, ROM); Write32(0x0E000568, ROM); break;
         case 21: Write32(0x004AC4B0, ROM); Write32(0x004AC570, ROM); Write32(0x0E000000, ROM); break;
         case 22: Write32(0x004AF670, ROM); Write32(0x004AF930, ROM); Write32(0x0E000098, ROM); break;
         case 23: Write32(0x004B7F10, ROM); Write32(0x004B80D0, ROM); Write32(0x0E000000, ROM); break;
         case 24: Write32(0x004BE9E0, ROM); Write32(0x004BEC30, ROM); Write32(0x0E000068, ROM); break;
         case 25: Write32(0x004C2700, ROM); Write32(0x004C2920, ROM); Write32(0x0E000038, ROM); break;
         case 26: Write32(0x004C41C0, ROM); Write32(0x004C4320, ROM); Write32(0x0E000000, ROM); break;
         case 27: Write32(0x004CD930, ROM); Write32(0x004CDBD0, ROM); Write32(0x0E0000B0, ROM); break;
         case 28: Write32(0x004CE9F0, ROM); Write32(0x004CEC00, ROM); Write32(0x0E00007C, ROM); break;
         case 29: Write32(0x004D14F0, ROM); Write32(0x004D1910, ROM); Write32(0x0E00016C, ROM); break;
         case 30: Write32(0x004EB1F0, ROM); Write32(0x004EC000, ROM); Write32(0x0E000394, ROM); break;
         default: break;
     }

}


void DebugPrint(char* message)
{

     if (!DebugFileCreated)
     {
         DebugLog = fopen(DebugLogPath, "w+b");
         DebugFileCreated = 1;
     }
     fputs(message, DebugLog);
}

void WriteCommand1A(unsigned int rom_start, unsigned int rom_end, FILE *fp)
{
     Write32(0x1A0C0009, fp);
     Write32(rom_start, fp);
     Write32(rom_end, fp);
}


void WriteCommand17(unsigned int bank, unsigned int rom_start, unsigned int rom_end, FILE *fp)
{

     int cmd = 0;

     if ( bank == 0x0E ||bank == 0x07 || bank == 0x06 || bank == 0x05 ) cmd = 0x170c0100 + bank;
     else cmd = 0x170c0000 + bank;

     Write32(cmd, fp);
     Write32(rom_start, fp);
     Write32(rom_end, fp);
}

void WriteCommand06(unsigned int jump_address, FILE* fp)
{
     Write32(0x06080000, fp);
     Write32(jump_address, fp);
}

void WriteCommand22(unsigned int model_id, unsigned int polygon_pointer, FILE* fp)
{

     int model_bank = (polygon_pointer & 0xFF000000)>>24;
     if (model_bank == 0x0e)  polygon_pointer = (polygon_pointer & 0x00FFFFFF) + 0x12000000;  // hack to transform 0x0e pointers into 0x12 pointers
     
     int cmd = 0x22080000 + model_id;

     Write32(cmd, fp);
     Write32(polygon_pointer, fp);
}



void WriteBankPointers(unsigned int bankC, unsigned int bankD, unsigned int bankE, FILE *rom)
{

     int LEVEL_ENTRY = 0x1200000 + LEVEL * 0xe0000;
     int COMMANDS_22_POSITION = LEVEL_ENTRY + 0xC8;

    fseek(rom, LEVEL_ENTRY + 0x68, SEEK_SET);

     switch (bankC)
     {
         case 0:  // haunted house, castle courtyard

             WriteCommand17(0x05, 0x0091BE8C, 0x0092C004, rom);
             WriteCommand17(0x0C, 0x0016D5C0, 0x0016D870, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x1500084C, rom);
             break;

         case 1:  // ccm/snow

             WriteCommand17(0x05, 0x008FB894, 0x009089C4, rom);
             WriteCommand17(0x0C, 0x001656E0, 0x00165A50, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x1500080C, rom);
             break;

         case 2:  // hazy, tall tall mountain

             WriteCommand17(0x05, 0x008DD7DC, 0x008F3894, rom);
             WriteCommand17(0x0C, 0x001602E0, 0x00160670, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150007E8, rom);
             break;

         case 3:  // ssl

             WriteCommand17(0x05, 0x008C118C, 0x008D57DC, rom);
             WriteCommand17(0x0C, 0x00151B70, 0x001521D0, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150007B4, rom);
             break;

         case 4:  // bob-omb

             WriteCommand17(0x05, 0x0088C3BC, 0x0089D45C, rom);
             WriteCommand17(0x0C, 0x0013B5D0, 0x0013B910, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x1500076C, rom);
             break;

         case 5:   // wet-dry, tick tock, whomp

             WriteCommand17(0x05, 0x00860EDC, 0x0087623C, rom);
             WriteCommand17(0x0C, 0x00132850, 0x00132C60, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x1500071C, rom);
             break;

         case 6:   // jolly, aquarium

             WriteCommand17(0x05, 0x008A545C, 0x008B918C, rom);
             WriteCommand17(0x0C, 0x00145C10, 0x00145E90, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000788, rom);
             break;

         case 7:   // TH island, rainbow ride, bowser1Course, dire dire docks

             WriteCommand17(0x05, 0x00960204, 0x009770C4, rom);
             WriteCommand17(0x0C, 0x00187FA0, 0x00188440, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150008A4, rom);
             break;

         case 8:  // castle grounds

             WriteCommand17(0x05, 0x00934004, 0x00958204, rom);
             WriteCommand17(0x0C, 0x00180540, 0x00180BB0, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000888, rom);
             break;

         case 9:   // vanish cap

             WriteCommand17(0x05, 0x009109C4, 0x00913E8C, rom);
             WriteCommand17(0x0C, 0x00166BD0, 0x00166C60, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000830, rom);
             WriteCommand22(0xB6, 0x0C000018, rom);
             WriteCommand22(0xB7, 0x0C000030, rom);
             WriteCommand22(0xB5, 0x0C000000, rom);
             break;

         case 10: // bowser fire sea, lethal lava land

             WriteCommand17(0x05, 0x0087E23C, 0x008843BC, rom);
             WriteCommand17(0x0C, 0x00134A70, 0x00134D20, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000750, rom);
             break;

         default:
         break;

     }

    fseek(rom, LEVEL_ENTRY + 0x80, SEEK_SET);

     switch (bankD)
     {
         case 0:  // haunted house, hazy maze

             WriteCommand17(0x06, 0x00A647AC, 0x00A7981C, rom);
             WriteCommand17(0x0D, 0x001F1B30, 0x001F2200, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150009DC, rom);
             break;

         case 1:  // 1 = ccm, snowman

             WriteCommand17(0x06, 0x00A56934, 0x00A5C7AC, rom);
             WriteCommand17(0x0D, 0x001E7D90, 0x001E7EE0, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150009C0, rom);
             break;

         case 2:  // inside castle, castle grounds

             WriteCommand17(0x06, 0x00A36ABC, 0x00A4E934, rom);
             WriteCommand17(0x0D, 0x001E4BF0, 0x001E51F0, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x1500099C, rom);
             break;

         case 3:  // bob-omb, tiny huge

             WriteCommand17(0x06, 0x00A09934, 0x00A2EABC, rom);
             WriteCommand17(0x0D, 0x001D7C90, 0x001D8310, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000958, rom);
             break;

         case 4:  // wet-dry, jolly

             WriteCommand17(0x06, 0x009E9FD4, 0x00A01934, rom);
             WriteCommand17(0x0D, 0x001C3DB0, 0x001C4230, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x15000914, rom);
             break;

         case 5:  // bowser fight

             WriteCommand17(0x06, 0x0097F0C4, 0x009E1FD4, rom);
             WriteCommand17(0x0D, 0x001B9070, 0x001B9CC0, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand06(0x150008D8, rom);
             break;

         default:
             break;

    }

    fseek(rom, LEVEL_ENTRY + 0x20, SEEK_SET);

     switch (bankE)
     {
         case 0:  // haunted house
             WriteCommand17(0x12, 0x003828C0, 0x00383950, rom);
             WriteCommand17(0x07, 0x00DB151D, 0x00DD8361, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET); // texture bank
             WriteCommand1A(0x00CBD3A9, 0x00CCB4BD, rom); //load textures
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x1D, 0x16000720, rom);
             WriteCommand22(0x35, 0x0E0005B0, rom);
             WriteCommand22(0x36, 0x0E0005C8, rom);
             WriteCommand22(0x37, 0x0E0005E0, rom);
             WriteCommand22(0x38, 0x0E0005F8, rom);
             WriteCommand22(0x39, 0x0E000610, rom);
             WriteCommand22(0x3A, 0x0E000628, rom);
             WriteCommand22(0x3B, 0x0E000640, rom);
             WriteCommand22(0x3C, 0x0E000640, rom);
             break;

         case 1:  // ccm
             WriteCommand17(0x12, 0x00395C90, 0x00396340, rom);
             WriteCommand17(0x07, 0x00DE0361, 0x00E03B07, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET); // texture bank
             WriteCommand1A(0x00D0FDF9, 0x00D1B20D, rom); //load textures
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E00043C, rom);
             WriteCommand22(0x04, 0x0E00046C, rom);
             WriteCommand22(0x05, 0x0E0004A4, rom);
             WriteCommand22(0x06, 0x0E0004CC, rom);
             WriteCommand22(0x07, 0x0E0004F4, rom);
             WriteCommand22(0x27, 0x1600043C, rom);
             WriteCommand22(0x36, 0x0E0003E0, rom);
             WriteCommand22(0xD2, 0x0E000400, rom);
             WriteCommand22(0x37, 0x0E00041C, rom);
             break;

         case 2:  // inside castle
             WriteCommand17(0x12, 0x003CF0D0, 0x003D0DC0, rom);
             WriteCommand17(0x07, 0x00E0BB07, 0x00E84C1F, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET); // texture bank
             WriteCommand1A(0x00D2320D, 0x00D31321, rom); //load textures
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x35, 0x0E000F18, rom);
             WriteCommand22(0x36, 0x0E001940, rom);
             WriteCommand22(0x37, 0x0E001530, rom);
             WriteCommand22(0x38, 0x0E001548, rom);
             WriteCommand22(0x39, 0x0E001518, rom);
             WriteCommand22(0x27, 0x160004D0, rom);
             WriteCommand22(0x29, 0x160005F8, rom);
             WriteCommand22(0x1C, 0x160003A8, rom);
             WriteCommand22(0x1D, 0x160004D0, rom);
             WriteCommand22(0xD0, 0x0E000F00, rom);
             WriteCommand22(0xD5, 0x0E000F00, rom);
             WriteCommand22(0xD1, 0x0E000F00, rom);
             WriteCommand22(0xD6, 0x0E000F00, rom);

             break;

         case 3: //hazy
             WriteCommand17(0x12, 0x003E6A00, 0x003E76B0, rom);
             WriteCommand17(0x07, 0x00E8CC1F, 0x00EB8587, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET); // texture bank
             WriteCommand1A(0x00D2320D, 0x00D31321, rom); //load textures
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x1D, 0x160004D0, rom);
             WriteCommand22(0x36, 0x0E0005A0, rom);
             WriteCommand22(0x37, 0x0E0005B8, rom);
             WriteCommand22(0x38, 0x0E0005D0, rom);
             WriteCommand22(0x39, 0x0E000548, rom);
             WriteCommand22(0x3A, 0x0E000570, rom);
             WriteCommand22(0x3B, 0x0E000588, rom);
             WriteCommand22(0x3C, 0x0E000530, rom);
             break;

         case 4: //ssl
             WriteCommand17(0x12, 0x003FB990, 0x003FC2B0, rom);
             WriteCommand17(0x07, 0x00EC0587, 0x00EE8E37, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CD34BD, 0x00CE03D1, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x1B, 0x16001048, rom);
             WriteCommand22(0x03, 0x0E0005C0, rom);
             WriteCommand22(0x04, 0x0E0005D8, rom);
             WriteCommand22(0x3A, 0x0E000618, rom);
             WriteCommand22(0x36, 0x0E000734, rom);
             WriteCommand22(0x37, 0x0E000764, rom);
             WriteCommand22(0x38, 0x0E000794, rom);
             WriteCommand22(0x39, 0x0E0007AC, rom);
             WriteCommand22(0xC7, 0x0E000630, rom);
             break;

         case 5: //bob
             WriteCommand17(0x12, 0x00405A60, 0x00405FB0, rom);
             WriteCommand17(0x07, 0x00EF0E37, 0x00F025F9, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CD34BD, 0x00CE03D1, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0E000440, rom);
             WriteCommand22(0x37, 0x0E000458, rom);
             WriteCommand22(0x38, 0x0E000470, rom);
             break;

         case 6: //snowmansland
             WriteCommand17(0x12, 0x0040E840, 0x0040ED70, rom);
             WriteCommand17(0x07, 0x00F0A5F9, 0x00F1A081, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D0FDF9, 0x00D1B20D, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0E000390, rom);
             WriteCommand22(0x37, 0x0E000360, rom);
             WriteCommand22(0x38, 0x0E000378, rom);
             WriteCommand22(0x12, 0x16001018, rom);
             break;

         case 7: //wetdry
             WriteCommand17(0x12, 0x00419F90, 0x0041A760, rom);
             WriteCommand17(0x07, 0x00F22081, 0x00F3A809, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D611C9, 0x00D6E9DD, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0E000580, rom);
             WriteCommand22(0x37, 0x0E000598, rom);
             WriteCommand22(0x38, 0x0E0005C0, rom);
             WriteCommand22(0x39, 0x0E0005E8, rom);
             WriteCommand22(0x3A, 0x0E000610, rom);
             WriteCommand22(0x3B, 0x0E000628, rom);
             WriteCommand22(0x3C, 0x0E000640, rom);
             break;

         case 8: // jlrb
             WriteCommand17(0x12, 0x00423B20, 0x004246D0, rom);
             WriteCommand17(0x07, 0x00F42809, 0x00F53BB5, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CE83D1, 0x00CF64E5, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x35, 0x0E000978, rom);
             WriteCommand22(0x36, 0x0E0009B0, rom);
             WriteCommand22(0x37, 0x0E0009E8, rom);
             WriteCommand22(0x38, 0x0E000A00, rom);
             WriteCommand22(0x39, 0x0E000990, rom);
             WriteCommand22(0x3A, 0x0E0009C8, rom);
             WriteCommand22(0x3B, 0x0E000930, rom);
             WriteCommand22(0x3C, 0x0E000960, rom);
             WriteCommand22(0x3D, 0x0E000900, rom);
             WriteCommand22(0x3E, 0x0E000918, rom);
             WriteCommand22(0x3F, 0x0E000948, rom);
             break;

         case 9: // th island
             WriteCommand17(0x12, 0x0042C6E0, 0x0042CF20, rom);
             WriteCommand17(0x07, 0x00F5BBB5, 0x00F69F71, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D611C9, 0x00D6E9DD, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E0005F0, rom);
             WriteCommand22(0x36, 0x0E0005B0, rom);
             WriteCommand22(0x37, 0x0E0005C8, rom);
             break;

         case 10: // tt clock
             WriteCommand17(0x12, 0x00437400, 0x00437870, rom);
             WriteCommand17(0x07, 0x00F71F71, 0x00F88991, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D39321, 0x00D430B5, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0E000240, rom);
             WriteCommand22(0x37, 0x0E000258, rom);
             WriteCommand22(0x38, 0x0E000270, rom);
             WriteCommand22(0x39, 0x0E000288, rom);
             WriteCommand22(0x3A, 0x0E0002A8, rom);
             WriteCommand22(0x3B, 0x0E0002C8, rom);
             WriteCommand22(0x3C, 0x0E0002E0, rom);
             WriteCommand22(0x3D, 0x0E0002F8, rom);
             WriteCommand22(0x3E, 0x0E000310, rom);
             WriteCommand22(0x3F, 0x0E000328, rom);
             WriteCommand22(0x40, 0x0E000340, rom);
             WriteCommand22(0x41, 0x0E000358, rom);
             WriteCommand22(0x42, 0x0E000370, rom);
             WriteCommand22(0x43, 0x0E000388, rom);
             WriteCommand22(0x44, 0x0E0003A0, rom);
             break;

         case 11: // rainbow ride
             WriteCommand17(0x12, 0x0044A140, 0x0044ABC0, rom);
             WriteCommand17(0x07, 0x00F90991, 0x00FBF807, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CFE4E5, 0x00D07DF9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E000660, rom);
             WriteCommand22(0x04, 0x0E000678, rom);
             WriteCommand22(0x05, 0x0E000690, rom);
             WriteCommand22(0x06, 0x0E0006A8, rom);
             WriteCommand22(0x07, 0x0E0006C0, rom);
             WriteCommand22(0x08, 0x0E0006D8, rom);
             WriteCommand22(0x09, 0x0E0006F0, rom);
             WriteCommand22(0x0A, 0x0E000708, rom);
             WriteCommand22(0x0B, 0x0E000720, rom);
             WriteCommand22(0x0C, 0x0E000738, rom);
             WriteCommand22(0x0D, 0x0E000758, rom);
             WriteCommand22(0x0E, 0x0E000770, rom);
             WriteCommand22(0x0F, 0x0E000788, rom);
             WriteCommand22(0x10, 0x0E0007A0, rom);
             WriteCommand22(0x11, 0x0E0007B8, rom);
             WriteCommand22(0x12, 0x0E0007D0, rom);
             WriteCommand22(0x13, 0x0E0007E8, rom);
             WriteCommand22(0x14, 0x0E000800, rom);
             WriteCommand22(0x15, 0x0E000818, rom);
             WriteCommand22(0x16, 0x0E000830, rom);
             WriteCommand22(0x36, 0x0E0008C0, rom);
             WriteCommand22(0x37, 0x0E000848, rom);
             WriteCommand22(0x38, 0x0E0008A8, rom);
             WriteCommand22(0x39, 0x0E000878, rom);
             WriteCommand22(0x3A, 0x0E0008D8, rom);
             WriteCommand22(0x3B, 0x0E000890, rom);
             WriteCommand22(0x3C, 0x0E000908, rom);
             WriteCommand22(0x3D, 0x0E000940, rom);
             WriteCommand22(0x3E, 0x0E000860, rom);
             WriteCommand22(0x3F, 0x0E000920, rom);
             WriteCommand22(0x40, 0x0E0008F0, rom);
             WriteCommand22(0x41, 0x0E000958, rom);
             WriteCommand22(0x42, 0x0E000970, rom);
             WriteCommand22(0x43, 0x0E000988, rom);
             WriteCommand22(0x44, 0x0E0009A0, rom);
             WriteCommand22(0x45, 0x0E0009B8, rom);
             break;

         case 12: // castle grounds
             WriteCommand17(0x12, 0x004545E0, 0x00454E00, rom);
             WriteCommand17(0x07, 0x00FC7807, 0x00FD907F, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D769DD, 0x00D84671, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E0006F4, rom);
             WriteCommand22(0x29, 0x160005F8, rom);
             WriteCommand22(0x36, 0x0E00070C, rom);
             WriteCommand22(0x37, 0x0E000660, rom);
             WriteCommand22(0x38, 0x0E000724, rom);
             break;

         case 13: // bowser 1course
             WriteCommand17(0x12, 0x0045BF60, 0x0045C600, rom);
             WriteCommand17(0x07, 0x00FE107F, 0x00FF0EAF, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CFE4E5, 0x00D07DF9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E0003C0, rom);
             WriteCommand22(0x04, 0x0E0003D8, rom);
             WriteCommand22(0x05, 0x0E0003F0, rom);
             WriteCommand22(0x06, 0x0E000408, rom);
             WriteCommand22(0x07, 0x0E000420, rom);
             WriteCommand22(0x08, 0x0E000438, rom);
             WriteCommand22(0x09, 0x0E000450, rom);
             WriteCommand22(0x0A, 0x0E000468, rom);
             WriteCommand22(0x0B, 0x0E000480, rom);
             WriteCommand22(0x0C, 0x0E000498, rom);
             WriteCommand22(0x0D, 0x0E0004B0, rom);
             WriteCommand22(0x0E, 0x0E0004C8, rom);
             WriteCommand22(0x0F, 0x0E0004E0, rom);
             WriteCommand22(0x10, 0x0E0004F8, rom);
             WriteCommand22(0x11, 0x0E000510, rom);
             WriteCommand22(0x36, 0x0E000558, rom);
             WriteCommand22(0x37, 0x0E000540, rom);
             WriteCommand22(0x38, 0x0E000528, rom);
             WriteCommand22(0x39, 0x0E000570, rom);
             WriteCommand22(0x3A, 0x0E000588, rom);
             WriteCommand22(0x3B, 0x0E0005A0, rom);
             WriteCommand22(0x3C, 0x0E0005B8, rom);
             WriteCommand22(0x3D, 0x0E0005D0, rom);
             WriteCommand22(0x3E, 0x0E0005E8, rom);
             WriteCommand22(0x3F, 0x0E000600, rom);
             break;

         case 14: // vanish
             WriteCommand17(0x12, 0x00461220, 0x004614D0, rom);
             WriteCommand17(0x07, 0x00FF8EAF, 0x01003B77, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D769DD, 0x00D84671, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0E0001F0, rom);
             break;

         case 15: // fire sea
             WriteCommand17(0x12, 0x0046A840, 0x0046B090, rom);
             WriteCommand17(0x07, 0x0100BB77, 0x0102177F, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CFE4E5, 0x00D07DF9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E0004B0, rom);
             WriteCommand22(0x04, 0x0E0004C8, rom);
             WriteCommand22(0x05, 0x0E0004E0, rom);
             WriteCommand22(0x06, 0x0E0004F8, rom);
             WriteCommand22(0x07, 0x0E000510, rom);
             WriteCommand22(0x08, 0x0E000528, rom);
             WriteCommand22(0x09, 0x0E000540, rom);
             WriteCommand22(0x0A, 0x0E000558, rom);
             WriteCommand22(0x0B, 0x0E000570, rom);
             WriteCommand22(0x0C, 0x0E000588, rom);
             WriteCommand22(0x0D, 0x0E0005A0, rom);
             WriteCommand22(0x0E, 0x0E0005B8, rom);
             WriteCommand22(0x0F, 0x0E0005D0, rom);
             WriteCommand22(0x10, 0x0E0005E8, rom);
             WriteCommand22(0x11, 0x0E000600, rom);
             WriteCommand22(0x12, 0x0E000618, rom);
             WriteCommand22(0x13, 0x0E000630, rom);
             WriteCommand22(0x14, 0x0E000648, rom);
             WriteCommand22(0x15, 0x0E000660, rom);
             WriteCommand22(0x36, 0x0E000758, rom);
             WriteCommand22(0x37, 0x0E0006C0, rom);
             WriteCommand22(0x38, 0x0E000770, rom);
             WriteCommand22(0x39, 0x0E0006A8, rom);
             WriteCommand22(0x3A, 0x0E000690, rom);
             WriteCommand22(0x3B, 0x0E000678, rom);
             WriteCommand22(0x3C, 0x0E000708, rom);
             WriteCommand22(0x3D, 0x0E000788, rom);
             WriteCommand22(0x3E, 0x0E000728, rom);
             WriteCommand22(0x3F, 0x0E000740, rom);
             WriteCommand22(0x40, 0x0E0006D8, rom);
             WriteCommand22(0x41, 0x0E0006F0, rom);
             break;

         case 16: // third course
             WriteCommand17(0x12, 0x00477D00, 0x004784A0, rom);
             WriteCommand17(0x07, 0x01034AAF, 0x010502A3, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CFE4E5, 0x00D07DF9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0E000430, rom);
             WriteCommand22(0x04, 0x0E000448, rom);
             WriteCommand22(0x05, 0x0E000460, rom);
             WriteCommand22(0x06, 0x0E000478, rom);
             WriteCommand22(0x07, 0x0E000490, rom);
             WriteCommand22(0x08, 0x0E0004A8, rom);
             WriteCommand22(0x09, 0x0E0004C0, rom);
             WriteCommand22(0x0A, 0x0E0004D8, rom);
             WriteCommand22(0x0B, 0x0E0004F0, rom);
             WriteCommand22(0x0C, 0x0E000508, rom);
             WriteCommand22(0x0D, 0x0E000520, rom);
             WriteCommand22(0x0E, 0x0E000538, rom);
             WriteCommand22(0x0F, 0x0E000550, rom);
             WriteCommand22(0x10, 0x0E000568, rom);
             WriteCommand22(0x11, 0x0E000580, rom);
             WriteCommand22(0x12, 0x0E000598, rom);
             WriteCommand22(0x13, 0x0E0005B0, rom);
             WriteCommand22(0x14, 0x0E0005C8, rom);
             WriteCommand22(0x36, 0x0E0005E0, rom);
             WriteCommand22(0x37, 0x0E0005F8, rom);
             WriteCommand22(0x39, 0x0E000610, rom);
             WriteCommand22(0x3C, 0x0E000628, rom);
             WriteCommand22(0x3D, 0x0E000640, rom);
             WriteCommand22(0x3E, 0x0E000658, rom);
             WriteCommand22(0x3F, 0x0E000670, rom);
             WriteCommand22(0x40, 0x0E000688, rom);
             WriteCommand22(0x41, 0x0E0006A0, rom);
             WriteCommand22(0x42, 0x0E0006B8, rom);
             WriteCommand22(0x43, 0x0E0006D0, rom);
             WriteCommand22(0x44, 0x0E0006E8, rom);
             WriteCommand22(0x45, 0x0E000700, rom);
             break;

         case 17: 
             WriteCommand17(0x12, 0x0048C9B0, 0x0048D930, rom);
             WriteCommand17(0x07, 0x010582A3, 0x01080B73, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CA7B95, 0x00CB53A9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0e0009E0, rom);
             WriteCommand22(0x04, 0x0e0009F8, rom);
             WriteCommand22(0x05, 0x0e000A10, rom);
             WriteCommand22(0x06, 0x0e000A28, rom);
             WriteCommand22(0x07, 0x0e000A40, rom);
             WriteCommand22(0x08, 0x0e000A60, rom);
             WriteCommand22(0x0A, 0x0e000A90, rom);
             WriteCommand22(0x0B, 0x0e000AA8, rom);
             WriteCommand22(0x0C, 0x0e000AC0, rom);
             WriteCommand22(0x0D, 0x0e000AD8, rom);
             WriteCommand22(0x0E, 0x0e000AF0, rom);
             WriteCommand22(0x38, 0x0e000B20, rom);
             WriteCommand22(0x3A, 0x0e000B38, rom);
             WriteCommand22(0x3E, 0x0e000BB0, rom);
             WriteCommand22(0x3F, 0x0e000BC8, rom);
             WriteCommand22(0x40, 0x0e000BE0, rom);
             WriteCommand22(0x41, 0x0e000BF8, rom);
             WriteCommand22(0x43, 0x0e000C10, rom);
             WriteCommand22(0x44, 0x0e000C30, rom);
             WriteCommand22(0x45, 0x0e000C50, rom);
             WriteCommand22(0x46, 0x0e000C70, rom);
             WriteCommand22(0x47, 0x0e000C90, rom);
             WriteCommand22(0x48, 0x0e000CB0, rom);
             WriteCommand22(0x49, 0x0e000CD0, rom);
             WriteCommand22(0x4A, 0x0e000CF0, rom);
             WriteCommand22(0x4B, 0x0e000D10, rom);
             WriteCommand22(0x4C, 0x0e000D30, rom);
             WriteCommand22(0x4D, 0x0e000D50, rom);
             WriteCommand22(0x4E, 0x0e000D70, rom);
             WriteCommand22(0x4F, 0x0e000D90, rom);
             WriteCommand22(0x50, 0x0e000DB0, rom);
             WriteCommand22(0x36, 0x0e000B08, rom);
             WriteCommand22(0x37, 0x0e000DD0, rom);
             WriteCommand22(0x39, 0x0e000DE8, rom);
             WriteCommand22(0x09, 0x0e000A78, rom);
             WriteCommand22(0x35, 0x0e000B50, rom);
             WriteCommand22(0x3B, 0x0e000B68, rom);
             break;

         case 18: // dire dire docks
             WriteCommand17(0x12, 0x00495A60, 0x00496090, rom);
             WriteCommand17(0x07, 0x01088B73, 0x01098883, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CE83D1, 0x00CF64E5, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0e000478, rom);
             WriteCommand22(0x37, 0x0e0004A0, rom);
             WriteCommand22(0x38, 0x0e000450, rom);
             break;

         case 19: // whomp's
             WriteCommand17(0x12, 0x0049DA50, 0x0049E710, rom);
             WriteCommand17(0x07, 0x010A0883, 0x010B269B, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D611C9, 0x00D6E9DD, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0e0007e0, rom);
             WriteCommand22(0x04, 0x0e000820, rom);
             WriteCommand22(0x05, 0x0e000860, rom);
             WriteCommand22(0x06, 0x0e000878, rom);
             WriteCommand22(0x07, 0x0e000890, rom);
             WriteCommand22(0x08, 0x0e0008A8, rom);
             WriteCommand22(0x09, 0x0e0008E8, rom);
             WriteCommand22(0x0A, 0x0e000900, rom);
             WriteCommand22(0x0C, 0x0e000940, rom);
             WriteCommand22(0x0D, 0x0e000AE0, rom);
             WriteCommand22(0x0E, 0x0e000958, rom);
             WriteCommand22(0x0F, 0x0e0009A0, rom);
             WriteCommand22(0x10, 0x0e0009B8, rom);
             WriteCommand22(0x11, 0x0e0009D0, rom);
             WriteCommand22(0x12, 0x0e0009E8, rom);
             WriteCommand22(0xAE, 0x0e000A00, rom);
             WriteCommand22(0xB1, 0x0e000A40, rom);
             WriteCommand22(0xAF, 0x0e000A58, rom);
             WriteCommand22(0xAD, 0x0e000A98, rom);
             WriteCommand22(0xB0, 0x0e000AB0, rom);
             WriteCommand22(0xB2, 0x0e000AC8, rom);
             WriteCommand22(0x2C, 0x0e000AF8, rom);
             WriteCommand22(0x2D, 0x0e000B10, rom);
             WriteCommand22(0x2E, 0x0e000B38, rom);
             WriteCommand22(0x2F, 0x0e000B60, rom);
             WriteCommand22(0x36, 0x0e000B78, rom);
             WriteCommand22(0x37, 0x0e000B90, rom);
             WriteCommand22(0x38, 0x0e000BA8, rom);
             WriteCommand22(0x39, 0x0e000BE0, rom);
             WriteCommand22(0x3A, 0x0e000BC8, rom);
             break;

         case 20:  // castle
             WriteCommand17(0x12, 0x004AF670, 0x004AF930, rom);
             WriteCommand17(0x07, 0x010E99EB, 0x010F0867, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D769DD, 0x00D84671, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x27, 0x160004D0, rom);
             WriteCommand22(0x03, 0x0e000200, rom);
             break;

         case 21: // wingcap
             WriteCommand17(0x12, 0x004C2700, 0x004C2920, rom);
             WriteCommand17(0x07, 0x011258AB, 0x0112E271, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CFE4E5, 0x00D07DF9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x03, 0x0e000160, rom);
             break;

         case 22:  //bowser2bat
             WriteCommand17(0x12, 0x004CE9F0, 0x004CEC00, rom);
             WriteCommand17(0x07, 0x0115C4E7, 0x0115E087, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00CA7B95, 0x00CB53A9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(36, 0x0e000170, rom);
             break;

         case 23:  //bowser3bat
             WriteCommand17(0x12, 0x004D14F0, 0x004D1910, rom);
             WriteCommand17(0x07, 0x01166087, 0x0116B143, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand17(0x0A, 0x00C7FA55, 0x00C9FB95, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x36, 0x0e000290, rom);
             WriteCommand22(0x37, 0x0e0002A8, rom);
             WriteCommand22(0x38, 0x0e0002C0, rom);
             WriteCommand22(0x39, 0x0e0002D8, rom);
             WriteCommand22(0x3A, 0x0e0002F0, rom);
             WriteCommand22(0x3B, 0x0e000308, rom);
             WriteCommand22(0x3C, 0x0e000320, rom);
             WriteCommand22(0x3D, 0x0e000338, rom);
             WriteCommand22(0x3E, 0x0e000350, rom);
             WriteCommand22(0x3F, 0x0e000368, rom);
             WriteCommand22(0x03, 0x0e000380, rom);

         case 24:  // tall tall
             WriteCommand17(0x12, 0x004EB1F0, 0x004EC000, rom);
             WriteCommand17(0x07, 0x01173143, 0x011A35B7, rom);
             fseek(rom, LEVEL_ENTRY + 0x50, SEEK_SET);
             WriteCommand1A(0x00D4B0B5, 0x00D591C9, rom);
             fseek(rom, COMMANDS_22_POSITION, SEEK_SET);
             COMMANDS_22_POSITION = COMMANDS_22_POSITION + 0x08;
             WriteCommand22(0x7B, 0x0e000DF4, rom);
             WriteCommand22(0x35, 0x0e000730, rom);
             WriteCommand22(0x36, 0x0e000710, rom);
             WriteCommand22(0x37, 0x0e000D14, rom);
             WriteCommand22(0x38, 0x0e000D4C, rom);
             WriteCommand22(0x39, 0x0e000D84, rom);
             WriteCommand22(0x3A, 0x0e000DBC, rom);
             WriteCommand22(0x03, 0x0e000748, rom);
             WriteCommand22(0x04, 0x0e000778, rom);
             WriteCommand22(0x05, 0x0e0007A8, rom);
             WriteCommand22(0x06, 0x0e0007D8, rom);
             WriteCommand22(0x07, 0x0e000808, rom);
             WriteCommand22(0x08, 0x0e000830, rom);
             WriteCommand22(0x09, 0x0e000858, rom);
             WriteCommand22(0x0A, 0x0e000880, rom);
             WriteCommand22(0x0B, 0x0e0008A8, rom);
             WriteCommand22(0x0C, 0x0e0008D0, rom);
             WriteCommand22(0x0D, 0x0e0008F8, rom);
             WriteCommand22(0x0F, 0x0e000920, rom);
             WriteCommand22(0x10, 0x0e000948, rom);
             WriteCommand22(0x11, 0x0e000970, rom);
             WriteCommand22(0x12, 0x0e000990, rom);
             WriteCommand22(0x13, 0x0e0009C0, rom);
             WriteCommand22(0x14, 0x0e0009F0, rom);
             WriteCommand22(0x15, 0x0e000A18, rom);
             WriteCommand22(0x16, 0x0e000A40, rom);
             break;

         default:
             break;

     }
     
     WriteCommand22(0x16, 0x16000388, rom); // pipe
     WriteCommand22(0x17, 0x16000FE8, rom); // tree
     WriteCommand22(0x18, 0x16001000, rom); // Spiky Tree (0x18)     
     WriteCommand22(0x19, 0x16001018, rom); // Snow Tree (0x19)
     WriteCommand22(0x1F, 0x160005F8, rom); // Metal Door (0x1F)
     WriteCommand22(0x20, 0x1600068C, rom); // Hazy Maze Door (0x20)
     WriteCommand22(0x22, 0x160007B4, rom); // Castle Door 1 Star
     WriteCommand22(0x23, 0x16000868, rom); // Castle Door 2 Star
     WriteCommand22(0x24, 0x1600091C, rom); // Castle Door 3 Star
     WriteCommand22(0x25, 0x160009D0, rom); // Castle Door with lock
     WriteCommand22(0x26, 0x160003A8, rom); // Castle Door

     // To DO: Other trees and Check this!
     
     DebugPrint("\n[0x22 Geo Layout Pointers]");
     sprintf(DebugMessage, "ROM Position = 0x%x", ftell(rom));
     DebugPrint(DebugMessage);
}

void Clear22Commands(FILE *fp)
{

    int i;
    int LEVEL_ENTRY = 0x1200000 + LEVEL * 0xe0000;

    fseek(fp, LEVEL_ENTRY + 0xC0, SEEK_SET);

    WriteCommand06(0x15000660, fp);

    for (i = 0; i <= 51; i ++)
    {
        //dummy command
        Write32(0x10080000, fp);
        Write32(0x00000000, fp);
    }

}

void WriteNewPatch(FILE *fp)
{
    fseek(fp, 0x1202000, SEEK_SET);
    fwrite(Cmd17Patch, sizeof(Cmd17Patch), 1, fp);

}

void WriteWaterPatch(FILE *fp)
{
    fseek(fp, 0x1203000, SEEK_SET);
    fwrite(WaterHack, sizeof(WaterHack), 1, fp);
    fseek(fp, 0x8C1AC, SEEK_SET);
    Write32(0x0C100C00, fp);       // JAL to 0x80403000
    
    fseek(fp, 0x1203500, SEEK_SET);
    fwrite(WaterHack2, sizeof(WaterHack2), 1, fp);
    fseek(fp, 0x8C1E8, SEEK_SET);
    Write32(0x0C100D40, fp); // JAL to 0x80403500

}


void WriteTerrainLayout(FILE *fp, unsigned int dlpointer, int alpha, unsigned int alphadlpointer)
{
    fwrite(TerrainLayoutStart, sizeof(TerrainLayoutStart), 1, fp);
    if (FOG == 1) Write32(0x15040000, fp); else Write32(0x15010000, fp);
    Write32(dlpointer, fp);
    
    if (alpha == 1)
    {
        if (AlphaFaceCount > 0)
        {
            Write32(0x15060000, fp); // render mode 6 for alpha
            Write32(alphadlpointer, fp);
        }
    }
    
    Write32(0x17000000, fp);
    Write32(0x18000000, fp);
    Write32(0x802761D0, fp);
    
    if (WATER == 1)
    {
        int HasWater = 0;
        int HasToxic = 0;
        int HasMist = 0;
        int i = 0;
        
        for (i = 1; i <= WaterCount; i++)
        {
            if ( WaterList[i].type == 1) HasToxic = 1;
            else if ( WaterList[i].type == 0) HasWater = 1;
            else if ( WaterList[i].type == 2) HasMist = 1;
        }

        
        if (HasWater)
        {
            Write32(0x18005000, fp);
            Write32(0x802D104C, fp);
        }
        
        if (HasToxic)
        {
            Write32(0x18005001, fp);
            Write32(0x802D104C, fp);
        }
        
        if (HasMist)
        {
            Write32(0x18005002, fp);
            Write32(0x802D104C, fp);
        }
        
        
    }

    Write32(0x05000000, fp);
    
    fwrite(TerrainLayoutEnd, sizeof(TerrainLayoutEnd), 1, fp);
}

int ProcessCustomBackground(char* bgfile, FILE* bgbank)
{
     
     char BGFilePath[800];
     char ShellCommand[1024];
     char ShellParams[2048];
     int wait = 0;
     int i = 0;
     int k = 0;
     int BGPointer[80];
          
     strcpy(BGFilePath, bgfile);
     StripToPath(BGFilePath);
     
     // convert to BMP 256x256
     sprintf(ShellCommand, "\"%s\"", CvtExePath);
     sprintf(ShellParams, "-resize 256x256! \"%s\" \"BMP3:m64background.bmp\"", bgfile);
                
     SHELLEXECUTEINFO process = { sizeof(process) };
     process.fMask = SEE_MASK_NOCLOSEPROCESS;
     process.nShow = SW_HIDE;
     process.lpVerb = "open";
     process.lpFile = ShellCommand;
     process.lpParameters = ShellParams;
     process.lpDirectory = BGFilePath;
     wait = ShellExecuteEx(&process);
     if (wait == 1) WaitForSingleObject(process.hProcess, INFINITE);
                
     DebugPrint("\n[Background Resizing/conversion]\nShell Execute = ");
     DebugPrint(ShellCommand);
     DebugPrint(" ");
     DebugPrint(ShellParams);
     
     //crop
     
     char mod[] = "%02d";
     sprintf(ShellCommand, "\"%s\"", CvtExePath);
     sprintf(ShellParams, "m64background.bmp -crop 32x32 BMP3:bg%s.bmp", mod);
                
     SHELLEXECUTEINFO process2 = { sizeof(process) };
     process2.fMask = SEE_MASK_NOCLOSEPROCESS;
     process2.nShow = SW_HIDE;
     process2.lpVerb = "open";
     process2.lpFile = ShellCommand;
     process2.lpParameters = ShellParams;
     process2.lpDirectory = BGFilePath;
     wait = ShellExecuteEx(&process2);
     if (wait == 1) WaitForSingleObject(process2.hProcess, INFINITE);
                
     DebugPrint("\n[Background cropping]\nShell Execute = ");
     DebugPrint(ShellCommand);
     DebugPrint(" ");
     DebugPrint(ShellParams);

     for (i = 0; i < 64; i++)
     {
         char bgpiece[800];
         char debugmessage[800];
         
         sprintf(bgpiece, "%sbg%02d.bmp", BGFilePath, i);
         FILE* bg = fopen(bgpiece, "rb");
         
         if (bg)
         {
             BGPointer[i] = FlippedBMPtoRGBA(bg, bgbank, NULL, 0); 
             fclose(bg);
             int test = remove(bgpiece);
             sprintf(DebugMessage, "\nRemoving (return value= %d) %s", test, bgpiece);
             DebugPrint(DebugMessage);
             
         }
         else
         {
               DebugPrint("\nError Processing Background File. ");
               DebugPrint("\nBG piece = ");
               DebugPrint(bgpiece);
               return 0;
               
         }
         
         BGPointer[i] = (BGPointer[i] & 0xFFFFFF) + 0x0A000000;
         sprintf(debugmessage, "\nBG piece %s, Pointer = 0x%x", bgpiece, BGPointer[i]);
         DebugPrint(debugmessage);
         
     }
     
     for (i = 0; i < 8; i++)
     {
         
         Write32(BGPointer[7 + (i*8)], bgbank);
         for (k =0; k < 8; k++)
         {
             Write32(BGPointer[k + (i*8)], bgbank);
         }
         Write32(BGPointer[(i*8)], bgbank);
     }
     
     
      char bgbmp[800];
      sprintf(bgbmp, "%sm64background.bmp", BGFilePath);
      int test =  remove(bgbmp);
      sprintf(DebugMessage, "\nRemoving (return value= %d) %s", test, bgbmp);
      DebugPrint(DebugMessage);
     
    
     return 1;
     
}


void ReadSettings(FILE *fp)
{

    char line[2048];
    while (fgets(line, 2048, fp))
    {
        char *ptr = (void*)0;

        if(ptr = strstr(line,"OffsetPosition ") )
	    {
             sscanf(ptr+15, "%d", &OFFSETMODEL);
        }
        else if(ptr = strstr(line,"DeathAtBottom ") )
	    {
             sscanf(ptr+14, "%d", &DEATH_AT_BOTTOM);
        }
        else if(ptr = strstr(line,"FlipTextures ") )
	    {
             sscanf(ptr+13, "%d", &FLIPTEXTURES);
        }
        
        else if(ptr = strstr(line,"DeathHeight ") )
	    {
             sscanf(ptr+12, "%d", &DEATH_HEIGHT);
        }
        else if(ptr = strstr(line,"SetMarioPos " ) )
	    {
             sscanf(ptr+12, "%d", &SETMARIOPOS);
        }
       else if(ptr = strstr(line,"Background ") )
	    {
             sscanf(ptr+11, "%d", &BACKGROUND);
        }
        else if(ptr = strstr(line,"FogPreset ") )
	    {
             sscanf(ptr+10, "%d", &FOGPRESET);
        }
        else if(ptr = strstr(line,"CustomBG ") )
	    {
             sscanf(ptr+9, "%d", &CUSTOMBG);
        }
        else if(ptr = strstr(line,"WaterBox ") )
	    {
             int type = 0;
             int x = 0;
             int z = 0;
             int x2 = 0;
             int z2 = 0;

             int mheight = 0;
             sscanf(ptr+9, "Type = %d, (%d, %d), (%d, %d), Height = %d ", &type, &x, &z, &x2, &z2, &mheight);
             
             int minx = 0;
             int maxx = 0;
             int minz = 0;
             int maxz = 0;
             
             if (x > x2)  
             {
                 minx = x2; 
                 maxx = x;
             }
             else
             {
                 maxx = x2;
                 minx = x;
             }
             
             if (z > z2)  
             {
                 minz = z2; 
                 maxz = z;
             }
             else
             {
                 maxz = z2;
                 minz = z;
             }
             

             sprintf(DebugMessage, "\nWaterBox Params = %d, (%d, %d), (%d, %d), %d", type, x, z, x2, z2, mheight);
             DebugPrint(DebugMessage);
             sprintf(DebugMessage, "\nMin X = %d Max X = %d, Min Z = %d, Max Z = %d", minx,maxx,minz,maxz);
             DebugPrint(DebugMessage);
             
             if (SWAPXZ) AddWaterBox(z, x, z2, x2, mheight, type);
             else AddWaterBox(x, z, x2, z2, mheight, type);
        }
        else if (ptr = strstr(line,"Scaling ") )
	    {
             sscanf(ptr+8, "%f", &SCALING);
        }
        else if(ptr = strstr(line,"Terrain ") )
	    {
             sscanf(ptr+8, "%d", &TERRAIN);
        }
        else if(ptr = strstr(line,"Weather ") )
	    {
             sscanf(ptr+8, "%d", &WEATHER);
        }
        else if(ptr = strstr(line,"Restore ") )
	    {
             sscanf(ptr+8, "%d", &RESTORE);
        }
        else if(ptr = strstr(line,"OffsetX ") )
	    {
             sscanf(ptr+8, "%d", &OFFSETX);
        }
        else if(ptr = strstr(line,"OffsetY ") )
	    {
             sscanf(ptr+8, "%d", &OFFSETY);
        }
        else if(ptr = strstr(line,"OffsetZ ") )
	    {
             sscanf(ptr+8, "%d", &OFFSETZ);
        }
        else if(ptr = strstr(line,"RotateY ") )
	    {
             sscanf(ptr+8, "%d", &ROTATEY);
        }
        else if(ptr = strstr(line,"Bright ") )
	    {
             sscanf(ptr+7, "%d", &BRIGHT);
        }        
        else if(ptr = strstr(line,"Camera ") )
	    {
             sscanf(ptr+7, "%d", &CAMERA);
        }
        else if(ptr = strstr(line,"MarioX ") )
	    {
             sscanf(ptr+7, "%d", &MARIOX);
        }
        else if(ptr = strstr(line,"MarioY ") )
	    {
             sscanf(ptr+7, "%d", &MARIOY);
        }
        else if(ptr = strstr(line,"MarioZ ") )
	    {
             sscanf(ptr+7, "%d", &MARIOZ);
        }
        else if(ptr = strstr(line,"SwapXZ ") )
	    {
             sscanf(ptr+7, "%d", &SWAPXZ);
        }
        else if(ptr = strstr(line,"AngleY ") )
	    {
             sscanf(ptr+7, "%d", &ANGLEY);
        }
        else if(ptr = strstr(line,"Bright ") )
	    {
             sscanf(ptr+7, "%d", &BRIGHT);
        }
        else if(ptr = strstr(line,"BankC ") )
	    {
             sscanf(ptr+6, "%d", &BANKC);
        }
        else if(ptr = strstr(line,"Level ") )
	    {
             sscanf(ptr+6, "%d", &LEVEL);
        }
        else if(ptr = strstr(line,"BankD ") )
	    {
             sscanf(ptr+6, "%d", &BANKD);
        }
        else if(ptr = strstr(line,"BankE ") )
	    {
             sscanf(ptr+6, "%d", &BANKE);
        }
        else if(ptr = strstr(line,"Music ") )
	    {
             sscanf(ptr+6, "%d", &MUSIC);
        }
        else if(ptr = strstr(line,"NoMtl ") )
	    {
             sscanf(ptr+6, "%d", &NOMTL);
        }
        else if(ptr = strstr(line,"Water ") )
	    {
             sscanf(ptr+6, "%d", &WATER);
        }
        else if(ptr = strstr(line,"FogR ") )
	    {
             sscanf(ptr+5, "%d", &FOGR);
        }
        else if(ptr = strstr(line,"FogG ") )
	    {
             sscanf(ptr+5, "%d", &FOGG);
        }
        else if(ptr = strstr(line,"FogB ") )
	    {
             sscanf(ptr+5, "%d", &FOGB);
        }
        else if(ptr = strstr(line,"Fog ") )
	    {
             sscanf(ptr+4, "%d", &FOG);
        }
        
        /* Missing: BG File string, ROM, file names */
        
     }
        
}

void AddWaterBox(int x, int z, int x2, int z2, int mheight, int type)
{
     WaterCount++;
     WaterList[WaterCount].x = x;
     WaterList[WaterCount].z = z;
     WaterList[WaterCount].x2 = x2;
     WaterList[WaterCount].z2 = z2;
     WaterList[WaterCount].height = mheight;
     WaterList[WaterCount].type = type;
     
     int minx = 0;
     int maxx = 0;
     int minz = 0;
     int maxz = 0;
             
             if (x > x2)  
             {
                 minx = x2; 
                 maxx = x;
             }
             else
             {
                 maxx = x2;
                 minx = x;
             }
             
             if (z > z2)  
             {
                 minz = z2; 
                 maxz = z;
             }
             else
             {
                 maxz = z2;
                 minz = z;
             }
             
     WaterList[WaterCount].minx = minx;
     WaterList[WaterCount].maxx = maxx;
     WaterList[WaterCount].minz = minz;
     WaterList[WaterCount].maxz = maxz;             
             
}
