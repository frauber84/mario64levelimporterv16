#include <stdio.h>

#define REGULAR_SIZE  0x800000
#define EXTENDED_SIZE 0x1800000
#define EXPANDED_SIZE 0x3000000

char *buffer;

int main ( int argc, char *argv[] )
{
    
    int FileSize = 0;
    
    FILE *fp = fopen(argv[1], "r+b");
    if (!fp) return;
    
    fseek(fp, 0, SEEK_END);
    FileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (FileSize == REGULAR_SIZE)
    {
       printf("\nNot an extended Super Mario 64 1.0 US ROM");
       fclose(fp);
       getch();
       exit(-1);
    }
    else if (FileSize == EXPANDED_SIZE)
    {
       printf("\nThis ROM has already been expanded");
       fclose(fp);
       getch();
       exit(-1);
    }
    else if (FileSize == EXTENDED_SIZE)
    {
                 
       printf("\nExpanding ROM, please wait ... ");
       fseek(fp, 0, SEEK_END);
       
       int check;
       check = ftell(fp);
       while ( !(check == EXPANDED_SIZE) ) 
       {
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          fputc(0x01, fp);
          check = ftell(fp);
       }
       
       printf(" ROM Sucessfully expanded! \nYou can now apply obj_import6.ppf patch with PPF-o-Matic 3.");
       getch();
       fclose(fp);
       exit(1);
                 
    }
    else
    {
        printf("\nWrong ROM! Extend a clean Super Mario 64 1.0 US ROM before using this program.");
        getch();    
        fclose(fp);
    }
    
}
