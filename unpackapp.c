/*
 * =====================================================================================
 *
 *       Filename:  unpack.c
 *
 *    Description:  A tool for unpack huawei *.app file.
 *
 *        Version:  1.0
 *        Created:  2013年03月28日 22时34分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  linkscue (scue), 
 *   Organization:  
 *
 * =====================================================================================
 */



#include    <stdlib.h>
#include    <stdio.h>
#include    <strings.h>
#define u8 unsigned char
#define u32 unsigned int
#define u16 unsigned short
#define BUFFER_SIZE 4

/* 
 * u32  magic
 * u32  packet_size
 * u32  flag=0x1
 * u8   Hardware[8]
 * u32  Filetype
 * u32  data_size
 * u8   date[16]
 * u8   time[16]
 * u8   filename[32]
 * u8   other[packet_size-92]
 * */

int realStrlen(char *string){
    int i=0;
    char ch=NULL;
    while ( (ch = *(string + i)) != '\xFF' ) {
        i++;
    }
    return i;
}
void hello(){
    printf("\n");
    printf("Welcome to use linkscue unpackapp tool!\n");
}
int main ( int argc, char *argv[] )
{
    hello();
    if (argc == 1) {
        printf("\n");
        printf("usage: %s unpack.app\n", argv[0]);
        exit(0);
    }
    char *file;
    FILE *fp,*fd;
    u32 tmp;
    u32 magic=0xa55aaa55;
    u32 packet_size=0;
    u32 flag=1;
    u8  hwid[8]="";
    u32 filetype=0;
    u32 data_size=0;
    u8  date[16]="";
    u8  time[16]="";
    u8  filename[32]="";
    int other_size=0;
    int number=1;
    char basename[32]="";
    char basehwid[8]="";
    int fp_start=0;
    int fp_local=0;
    int i=0,count=0,counts=0;
    int end_point=0;
    file=argv[1];
    if ( (fp=fopen(file,"rb")) == NULL) {
        printf("open file failure!\n",file);
        exit(1);
    }
    printf("\n");
    while (!feof(fp)) {
        fscanf(fp, "%4c", &tmp);
        if (tmp == magic) {
            fp_start=(ftell(fp)-4);
            fscanf(fp, "%4c",&packet_size);
            fscanf(fp, "%4c",&flag);
            fscanf(fp, "%8c",&hwid);
            fscanf(fp, "%4c",&filetype);
            fscanf(fp, "%4c",&data_size);
            fscanf(fp, "%16c",&date);
            fscanf(fp, "%16c",&time);
            fscanf(fp, "%32c",&filename);
            other_size=( packet_size - 92 );
            fseek(fp, other_size, SEEK_CUR);
            if ( strcmp(filename,"INPUT") == 0 ) {
                sprintf(basename, "output_%02d.img", number);
                strncpy(filename, basename, sizeof(filename));
                number++;
            }
//            if ( (int)packet_size <= 0 || (int)data_size <= 0) {
//                continue;
//            }
            strncpy(basehwid, hwid , realStrlen(hwid));
            printf("At: 0x%08x hw: %s size: %-10d time: %s_%s -->  %s\n", fp_start, basehwid, data_size, date, time, filename);
            if ((fd=fopen(filename,"wb"))==NULL) {
                printf("open %s to write data failure!\n", filename);
                fseek(fp, 88, SEEK_CUR);
                continue;
            }
            u8 buffer[BUFFER_SIZE];
            counts = 0;
            while ( counts < data_size ){
                if (feof(fp)) {                 /* if the end of file, break! */
                    break;
                }
                count = fread(buffer, 1, BUFFER_SIZE, fp);
                fwrite(buffer, 1 , BUFFER_SIZE, fd);
                counts+=count;
            } 
//            printf("packet size is 0x%08x, count is %d,  data start at 0x%08x, this time end at 0x%08x\n", packet_size,counts,  (packet_size+fp_start), ftell(fp));
            fclose(fd);
            fseek(fp, ( fp_start + 92 ) , SEEK_SET);
        }
    }
//    printf("Unpack %s ok!\n", file);
    fclose(fp);
    return EXIT_SUCCESS;
}
