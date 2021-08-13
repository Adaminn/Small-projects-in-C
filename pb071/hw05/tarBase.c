
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include "tarBase.h"
#include <time.h>
#include <sys/types.h>

# define DT_DIR                4
# define DT_REG                8
# define ITEMS                16
/*

const int NAME = 0;
const int MODE = 100;
const int OWNER_ID = 108;
const int GROUP_ID = 116;
const int SIZE = 124;
const int TIME = 136;
const int CHECKSUM = 148;
const int TYPE = 156;
const int LINKED_NAME = 157;
const int MAGIC = 257;
const int VERSION = 263;
const int OWNER_NAME = 265;
const int GROUP_NAME = 297;
const int MAJOR = 329;
const int MINOR = 337;
const int PREFIX = 345;
const int HEADER[ITEMS] = {NAME  ,  MODE  ,  OWNER_ID  ,  GROUP_ID  ,  SIZE  ,  TIME  ,  CHECKSUM  ,  TYPE  ,  LINKED_NAME  ,  MAGIC  ,  VERSION  ,  OWNER_NAME  ,  GROUP_NAME  ,  MAJOR  ,  MINOR  ,  PREFIX};
*/

struct Header {

    char name[100];
    char mode[8];
    char owner_id[8];
    char group_id[8];
    char size[12];
    char time[12];
    char checksum[8];
    char type[1];
    char linked_name[100];
    char magic[6];
    char version[2];
    char owner_name[32];
    char group_name[32];
    char major[8];
    char minor[8];
    char prefix[155];
    //char* content[ITEMS];

};

size_t getCheckSum(struct Header* hd){
    size_t checksum = 256;
    char *readableHeader = (char *) hd;
    for (size_t i = 0; i < 512; i++) {
        if (i >= 148 && i <= 155) {
            continue;
        }
        checksum += (unsigned ) readableHeader[i];
    }
    return checksum;
}

int fillHeaderFromFile(struct Header* hd, char* path){
    struct stat st;
    stat(path, &st);
    // name
    if (strlen(path) >= 100){
        if (strlen(path) > 265){
            fprintf(stderr, "%s\n", "too long path");
            return 1;
        }
        int i = 100;
        while (path[i] != '/'){
            i--;
        }
        path[i] = '\0';
        sprintf(hd->name, "%s", path);

        if (strlen(path) > 100){
            sprintf(hd->prefix, "%s", &path[++i]);
        }
    }else{
        sprintf(hd->name, "%s", path);
    }

    // mod
    sprintf(hd->mode, "%07o", 511 & st.st_mode);

    // owner id
    sprintf(hd->owner_id, "%07o", st.st_uid);

    // group id
    sprintf(hd->group_id, "%07o", st.st_gid);

    // size
    if (S_ISREG(st.st_mode)){
        sprintf(hd->size, "%011lo", st.st_size);
    } else{
        sprintf(hd->size, "%011o", 0);
    }

    // time
    sprintf(hd->time, "%011lo", st.st_mtim.tv_sec);



    // type
    if (S_ISREG(st.st_mode)){
        sprintf(hd->type, "%c", '0');
    } else {
        sprintf(hd->type, "%c", '5');
    }
    // linked name

    // magic nmb
    sprintf(hd->magic, "%s", "ustar");
    // version
    sprintf(hd->version, "%s", "00");

    struct passwd *pwd = getpwuid(st.st_uid);
    if (pwd != NULL){
        sprintf(hd->owner_name, "%s", pwd->pw_name);
    } else{
        fprintf(stderr, "%s\n", "pwd");
        return -1;
    }
    struct group *grp = getgrgid(st.st_gid);
    if (grp != NULL){
        sprintf(hd->group_name, "%s", grp->gr_name);
    }else{
        fprintf(stderr, "%s\n", "grp");
        return -1;
    }

    // major
    sprintf(hd->major, "%07o", 0);

    // minor
    sprintf(hd->minor, "%07o", 0);

    //checksum
    unsigned checksum = getCheckSum(hd);
    sprintf(hd->checksum, "%07o", checksum);

    return 0;
}

int addFileToTar(FILE* tar, char* path, char blank){

    struct Header* hd = NULL;
    if (posix_memalign((void **) &hd, 512, 512) != 0){
        return -1;
    }
    memset(hd, blank, 512);

    if (fillHeaderFromFile(hd, path) != 0){
        return 1;
    }

    // write header to the file
    char *readableHeader = (char *) hd;
    for (int i = 0; i < 512; ++i) {
        fputc(readableHeader[i], tar);
    }


    if (hd->type[0] == 48){ //reg
        // write the content of the file
        FILE* src = fopen(path,  "r");
        if (src == NULL){
            fprintf(stderr, "%s\n", "reading failed");
            return 1;
        }

        char buffer[512];
        size_t bytes;
        size_t last_bytes = 1;
        while ((bytes = fread(buffer, 1,  sizeof(buffer), src)) > 0 ){
            last_bytes = bytes;
            fwrite(buffer , 1 , bytes , tar );
        }

        // padding
        for (size_t i = 0; i < 512 - last_bytes; ++i) {
            fputc( '\0', tar);
        }

        fclose(src);
    }

    return 0;
}
int is_dir(const struct dirent *d)
{
    return d->d_type == DT_DIR && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0 ;
}
int is_reg(const struct dirent *d)
{
    return d->d_type == DT_REG;
}

int preorder_iter(FILE* tar, char* path, bool v, char blank, bool* added, bool* error){
    if (v){
        fprintf(stderr, "%s\n", path);
    }
    struct stat st;
    if ( stat(path, &st) != 0){
        fprintf(stderr, "%s\n", "iter - stat");
        *error = true;
    }else if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)){
        if (addFileToTar(tar, path, blank) != 0){
            *error = true;
        }
        *added = true;
    }

    if (S_ISDIR(st.st_mode)){
        struct dirent **content;
        int (*filter)(const struct dirent *) = is_dir;
        for (int i = 0; i < 2; ++i) {  // for dir, then reg

            if (i == 1){
                filter = is_reg;
            }

            int n = scandir(path, &content, filter, alphasort);

			fprintf(stderr, "naskenovano %d\n", n);
            if (n < 0) {
                fprintf(stderr, "%s\n", "scandir");
                return -1;
            } else {
                while (n>0) {
                    n--;

                    char newPath[512] = {0};
                    strcpy(newPath, path);
                    strcat(newPath, "/");
                    strcat(newPath, content[n]->d_name);
                    if (  preorder_iter(tar, newPath, v, blank, added, error) != 0){
                        return -1;
                    }

                    free(content[n]);
                }
                free(content);
            }
        }


    }

    return 0;
}
int writeBlankHeaderToTar(FILE* tar){
    for (int i = 0; i < 512; ++i) {
        fputc('\0', tar);
    }

    return 0;
}

int createTar(char* tarPath, char** files, int start, int filesCount, bool v, char blank){

    FILE* tar = fopen(tarPath, "w");
    if(tar == NULL)
    {
        fprintf(stderr, "%s\n", "open");
        return -1;
    }
    bool added = false;
    bool error = false;
    for (int i = start; i < filesCount; i++){

        if (preorder_iter(tar, files[i], v, blank, &added, &error) != 0){/*
            fclose(tar);
            return -1;*/
        }

    }
    writeBlankHeaderToTar(tar);
    fclose(tar);
    if (!(added)){/*
        fprintf(stderr, "%s\n", "nothing was added");
        return -1;*/
    }
    if (error){/*
        return -1;*/
    }
    return 0;
}

int fillHeaderFromTar(FILE* tar, struct Header* hd){

    char *readableHeader = (char *) hd;
    for (int i = 0; i < 512; i++) {
        readableHeader[i] = (char) fgetc(tar);
        /*
        if (readableHeader[i] == 0){
            fprintf(stderr, "(%d)", i);
        }else{
            fprintf(stderr, "%c", readableHeader[i]);
        }*/
    }


    return 0;
}

int createAllFilesFromPath(char* path, bool dir){
    struct stat st = {0};
    char path_cpy[512] = {0};
    strcpy(path_cpy, path);
    char* part = strtok(path_cpy, "/");
    char lastPart[512] = {0};
    char fullPart[512] = {0};

    while(part != NULL)
    {
        if (strlen(part) == 0){
            break;
        }
        if (strlen(lastPart) != 0) {
            strcat(fullPart, "/");
        }
        strcat(fullPart, part);
        if (stat(fullPart, &st) == -1) {
            if (strlen(lastPart) != 0) {
                mkdir(lastPart, 0777);
            }

        }
        strcpy(lastPart, fullPart);
        part = strtok(NULL, "/");
    }

    if (stat(lastPart, &st) == -1) {
        if (dir){  // dir

            mkdir(lastPart, 0777);

        }else { // reg
            creat(lastPart, 0700);
        }

    }else{
        if (dir){  // dir
            return 1;

        }else { // reg
            fprintf(stderr, "file %s already exists\n", lastPart);
            return -1;
        }

    }
    return 0;
}

int setFileAttributes(char* dest, struct Header* hd){
    chmod(dest, strtol(hd->mode, NULL, 8));

    struct utimbuf time;
    time.modtime = strtol(hd->time, NULL, 8);
    utime(dest, &time);
    return 0;
}

int extractNextFile(FILE* tar, bool v){
    struct Header* hd = NULL;
    if (posix_memalign((void **) &hd, 512, 512) != 0){
        return -1;
    }
    memset(hd, '\0', 512);

    if (fillHeaderFromTar(tar, hd) != 0){
        return 1;
    }

    if (strlen(hd->name) == 0){
        return 1;
    }

    unsigned temp = getCheckSum(hd);
    if (strtol(hd->checksum, NULL, 8) != temp){
        fprintf(stderr, "wrong checksum should be %s  got %07o\n", hd->checksum, temp );
        return -1;
    }



    char path[512] = {0};

    if (strlen(hd->prefix) > 0){
        strcpy(path, hd->prefix);
        strcat(path, "/");

    }
    strcat(path, hd->name);
    if (v) {
        fprintf(stderr, "%s\n", path);
    }
    bool dir;
    if (hd->type[0] == '5'){
        dir = true;
    }else{
        dir = false;
    }
    if(createAllFilesFromPath(path, dir) != 0){
        if (hd->type[0] == '5'){ // dir already exists
            return 0;
        }
        return -1;
    }

    setFileAttributes(path, hd);

    if (hd->type[0] == '0'){
        FILE *dest = fopen(path, "w");
        if (dest == NULL) {
            fprintf(stderr, "opening %s. errno: %d\n", path, errno);
            return -1;
        }

        // write the content to the file
        size_t to_read = strtol(hd->size, NULL, 8);
        char *buffer = malloc(sizeof(to_read));

        size_t bytes;

        bytes = fread(buffer, 1, to_read, tar);

        fwrite(buffer, 1, bytes, dest);
        free(buffer);
        fseek(tar, (long) (512 - to_read%512), SEEK_CUR);

        fclose(dest);
    }
    return 0;
}

int extractTar(char* tarPath, bool v){
    FILE* tar = fopen(tarPath, "r");
    if (tar == NULL){
        fprintf(stderr, "open\n");
        return -1;
    }

    while (true) {

        int res = extractNextFile(tar, v);
        if (res == 1){
            break;
        }
        if (res != 0){
            fclose(tar);
            return -1;
        }

    }
    fclose(tar);
    return 0;
}


