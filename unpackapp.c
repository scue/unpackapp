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
#include    <string.h>
#include    <strings.h>
#include    <ctype.h>
#define u8 unsigned char
#define u32 unsigned int
#define u16 unsigned short
#define BUFFER_SIZE 4

typedef struct _image
{
    // u32  magic
    u32  packet_size;
    u32  flag;
    char hardware[8];
    u32  filetype;
    u32  data_size;
    char date[16];
    char time[16];
    char filename[32];
    // u8   other[packet_size-92]
} image;

// 字符串真实长度
int realStrlen(char *string){
    int i=0;
    char ch=0;
    while ( (ch = *(string + i)) != '\xFF' ) {
        i++;
    }
    return i;
}

// 去掉字符串尾部 \xFF
void trim_str(char *string){
    char tmpstr[128]="";
    strncpy(tmpstr, string, realStrlen(string));
    strcpy(string, tmpstr);
}

// 字符串大写转换小写
void str_tolower(char *string){
    int i=0;
    for (i = 0; i < strlen(string); ++i) {
        string[i]=tolower(string[i]);
    }
}

// 重命名镜像文件
void rename_imgfname(char *filename, int number){
    // 针对MTK, 它们所有Image名字都是INPUT
    char tmp[32]="";
    if ( strcmp(filename,"INPUT") == 0 ) {
        sprintf(tmp, "output_%02d.img", number);
        strcpy(filename, tmp);
    }
    // 针对普通机型
    else {
        str_tolower(filename);
        snprintf(tmp, sizeof(tmp)-1, "%s.img", filename);
        strcpy(filename, tmp);
    }
}

// 欢迎信息
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

    // 文件
    char *file;                                 /* app文件 */
    FILE *fp,*fd;

    // 镜像信息
    u32 magic=0xa55aaa55;
    image img;
    int other_size=0;

    u32 tmp;                                    /* for compare magic */
    int number=1;                               /* for MTK only */
    u8 buffer[BUFFER_SIZE];                     /* for read image file */
    int fp_start=0;                             /* for image file start point */
    int i=0,count=0,counts=0;

    file=argv[1];
    if ( (fp=fopen(file,"rb")) == NULL) {
        printf("open file %s failure!\n",file);
        exit(1);
    }
    printf("\n");
    while (!feof(fp)) {
        fread( &tmp, 1, sizeof(tmp), fp ); 
        if (tmp == magic) {
            memset(&img, 0, sizeof(img));
            fp_start=(ftell(fp)-4);

            // 读取镜像相关信息
            fread( &img, 1, sizeof(img), fp ); 
            other_size=( img.packet_size - 92 );

            // 字符串处理
            trim_str(img.hardware);
            trim_str(img.filename);
            rename_imgfname(img.filename, number);

            printf("At: 0x%08x hw: %s size: %-10d time: %s_%s -->  %s\n",
                    fp_start, img.hardware, img.data_size, img.date, img.time, img.filename);

            // 写入镜像文件 
            fseek(fp, other_size, SEEK_CUR);
            if ((fd=fopen(img.filename,"wb"))==NULL) {
                printf("open %s to write data failure!\n", img.filename);
                fseek(fp, sizeof(image), SEEK_CUR);
                continue;
            }
            counts = 0;
            while ( counts < img.data_size ){
                if (feof(fp)) {                 /* if the end of file, break! */
                    break;
                }
                count = fread(buffer, 1, BUFFER_SIZE, fp);
                fwrite(buffer, 1 , BUFFER_SIZE, fd);
                counts+=count;
            } 
            fclose(fd);

            // 移动文件，使之以4字节对齐
            fseek(fp, (fp_start+(img.data_size/sizeof(magic))*sizeof(magic)) , SEEK_SET);
            number++;
        }
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
