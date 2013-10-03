#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <errno.h>
/* these were not used in program, keep it here for refrence only */
struct PIC_INFO{
    int     offset;    //offset of picture actual data
    int     pic_size;
    char    *pic_name; //variant len
};

struct HEADER {
    char    type[4];    //buka
    int     unknown1; // 29 not changed, between same commic archives
    int     unknown2; // 1 not changed, not related to pic data
    int     comic_id;
    int     vol_num;
    char *name;         //utf8 encoded string, variant len
    int     date1;      //unknown but looks like date, so do next two
    int     date2;
    int     date3;
    char    index_file[11]; //index2.dat
    struct PIC_INFO *pic_infos; //variant len
};

#define BUF_LEN 1024
int get_str(char *buf, size_t len, FILE *fp)
{
    int c, n;
    n = 0;
    do{
        c = fgetc(fp);
        buf[n]=c;
        n += 1;
    } while( c != '\0' && c != EOF);
    return n;
}
static void _mkdir(const char *dir, mode_t mode) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, mode);
            *p = '/';
        }
    mkdir(tmp, mode);
}
int main(int argc, char *argv[])
{
    int i=1;
    char buf[BUF_LEN] = {0};

    if(argc < 2){
        printf("%s same_file.buka\n", argv[0]);
        return 1;
    }

    for(; i < argc; i+=1){
        FILE *fp = fopen(argv[i], "rb");
        int32_t comic_id= 0;
        int32_t vol_num =0;
        int32_t index_pos =0;
        int32_t file_size =0;
        int32_t offset=0;
        int32_t jpg_size=0;
        char *name;
        int len =0;
        int ret = 0;
        char *dir_name;
        int next_pos = 0;
        char *filename;

        if(fp < 0){
            printf("open file error: %s", argv[i]);
            return 2;
        }
        fseek(fp, 0L, SEEK_SET);
        memset(buf, '\0', 4+1);
        fread(buf, 1, 4, fp);
        if(0 != strncmp(buf, "buka", 4)){
            printf("%s is not buka format \n", argv[i]);
            return 3;
        }
        fseek(fp, 12, SEEK_SET);
        memset(buf, 0, 8+1);
        fread(buf, 1, 8, fp);
        comic_id = *(int32_t *)buf;
        vol_num = *(int32_t *)(buf+4);
        printf("comic:%d, vol:%d\n", comic_id, vol_num);
        memset(buf, '\0', BUF_LEN);
        //fgets(buf, BUF_LEN, fp); // this a trick
        len = get_str(buf, BUF_LEN, fp);
        printf("name:%s\n", buf);
{
        name = (char *)malloc(len + 1);
        memcpy(name, buf, len);
        memset(buf, '\0', BUF_LEN);
        sprintf(buf,"%d(%s)/%d", comic_id, name, vol_num);
        index_pos = 12 + 8 + len+1 + 11;
        len = strlen(buf);
        dir_name = (char *)malloc(len + 1);
        memcpy(dir_name, buf, len);
        _mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);
}

        fseek(fp, index_pos, SEEK_SET);
        memset(buf, '\0', BUF_LEN);
        //fgets(buf, BUF_LEN, fp);
        len = get_str(buf, BUF_LEN, fp);
        puts(buf);
        if(0 != strncmp(buf, "index2.dat", 10)){
            printf("not found index data, may be they changed the format\n", argv[i]);
            return 5;
        }
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        next_pos = index_pos+10+1;
        fseek(fp, next_pos, SEEK_SET);
        do{
            FILE *fp_jpg;

            offset=0;
            jpg_size=0;
            memset(buf, 0, BUF_LEN);
            fseek(fp, next_pos, SEEK_SET);
            fread(buf, 4, 2, fp);
            offset = *(int32_t *)buf;
            jpg_size = *(int32_t *)(buf+4);
            //fgets(buf, BUF_LEN, fp);
            memset(buf, '\0', BUF_LEN);
            len = get_str(buf, BUF_LEN, fp);
            next_pos += 8 + len;
        //if(filename != NULL)
         //   free(filename);
            filename = (char *)malloc(len+1);
            memset(filename, '\0', len+1);
            memcpy(filename, buf, len);
            printf("offset:%d, jpg_size:%d, filename:%s\n", offset, jpg_size, filename);

            memset(buf, '\0', BUF_LEN);
            sprintf(buf, "%s/%s", dir_name, filename);
            puts(buf);
            fp_jpg = fopen(buf, "wb");
            if(fp_jpg == NULL){
                printf("ouput jpg file(%s)fail\n", buf);
                return 6;
            }
            {
                int n_read =0, n_write = 0;
                int total = jpg_size;
                int len_to_read =BUF_LEN;
                fseek(fp, offset, SEEK_SET);
                do{
                    len_to_read = (len_to_read < total)?len_to_read :total;
                    memset(buf, '\0', BUF_LEN);
                    n_read = fread(buf, 1, len_to_read, fp);
                    n_write = fwrite(buf, 1, n_read, fp_jpg);
                    if(n_write != n_read){
                        printf("write to jpg file fail\n");
                        fclose(fp);
                        fclose(fp_jpg);
                        return 7;
                    }
                    total -= n_read;
                } while(total > 0);
                fflush(fp_jpg);
            }
            fclose(fp_jpg);
        }while(0 != strncmp(filename, "logo", 4)
            && offset+jpg_size < file_size);
        /*
        if(filename != NULL)
            free(filename);
            */
        if(name != NULL)
            free(name);
        if(dir_name != NULL)
            free(dir_name);
        fclose(fp);
    }
    return 0;
}
