#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "md5.h"

int alterChars(char origin, char* alters, bool t_switch, bool c_switch){
    int index = 0;
    alters[index++] = origin;
    char lower = origin;
    if (origin >= 65 && origin <= 90){
        lower = (char) (origin + 32);
        if (c_switch){
            alters[index++] = (char) (origin + 32);
        }

    }else if (origin >= 97 && origin <= 122){  
        if (c_switch){
            alters[index++] = (char) (origin - 32);
        }
    }

    if (t_switch){
        switch (lower) {
            case 'a':
                alters[index++] = '@';
                alters[index++] = '4';
                break;
            case 'b':

                alters[index++] = '8';
                break;
            case 'e':
                alters[index++] = '3';
                break;
            case 'i':
                alters[index++] = '!';
                break;
            case 'l':
                alters[index++] = '1';
                break;
            case 'o':
                alters[index++] = '0';
                break;
            case 's':
                alters[index++] = '$';
                alters[index++] = '5';
                break;
            case 't':
                alters[index++] = '7';
                break;
            default:
                break;
        }
    }

    return index;

}


bool checkHash(MD5_CTX* md5_ctx, char* line, unsigned line_len, char* hash) {
    unsigned char md5_hash[17] = {'\0'};
    MD5_Init(md5_ctx);
    MD5_Update(md5_ctx, line, line_len);
    MD5_Final(md5_hash, md5_ctx);

    char hexs[33] = {'\0'};
    for (int i = 0; i < 16; i++) {
        sprintf(&hexs[2*i],"%02x", md5_hash[i]);
    }

    if (strcmp(hexs, hash) == 0){
        return true;
    }

    return false;

}

bool findPasswordInLine(char* line, int line_len, int locked_chars, char* hash, bool t_switch, bool c_switch){
    MD5_CTX md5_ctx;
    if (checkHash(&md5_ctx, line, line_len, hash)){

        return true;
    }
    if (!t_switch && !c_switch){
        return false;
    }

    if (line_len <= locked_chars){
        return false;
    }

    char alters[5] = {'\0'};
    char origin = line[locked_chars];
    int alters_cnt = alterChars(origin, alters, t_switch, c_switch);
    for (int j = 0; j < alters_cnt; ++j) {  // foreach char variants

        line[locked_chars] = alters[j];
        if (findPasswordInLine(line, line_len, locked_chars+1, hash, t_switch, c_switch)){
            return true;
        }
    }
    line[locked_chars] = origin;

    return false;
}

char* findPassword(FILE* dict, char* hash, bool t_switch, bool c_switch){

    char* line = NULL;

    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, dict)) != -1) {  // foreach line
        if (read < 1){
            continue;
        }
        if (line[read-1] == '\n'){
            line[read-1] = '\0';
            if (read < 2){
                continue;
            }
            if (line[read-2] == '\r'){
                line[read-2] = '\0';
                read--;
            }
            read--;
        }

        if (findPasswordInLine(line, read, 0, hash, t_switch, c_switch)){
            fclose(dict);
            return line;
        }

    }

    fclose(dict);
    if (line){
        free(line);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 3){
        fprintf(stderr, "invalid arg count");
        return 1;
    }
    int arg_ind = 1;
    char* switches = argv[arg_ind];

    bool t = false, c = false;
    if (switches[0] == '-'){
        if (argc < 4){
            fprintf(stderr, "invalid arg count");
            return 1;
        }
        arg_ind++;
        if (strchr(switches, 't') != NULL){
            t = true;
        }
        if (strchr(switches, 'c') != NULL){
            c = true;
        }

        if (!c && !t){
            fprintf(stderr, "invalid switch");
            return 1;
        }

    }


    char* dict_path = argv[arg_ind++];
    char* hash = argv[arg_ind];

    if (strlen(hash) != 32){
        fprintf(stderr, "invalid hash length");
        return 1;
    }

    FILE* dict = fopen(dict_path, "r");

    if (dict == NULL){
        fprintf(stderr, "open");
        return 1;
    }


    char* password = findPassword(dict, hash, t, c);
    if (password){
        printf("password found\n%s\n", password);
    }else{
        printf("password not found\n");
    }

    if (password){
        free(password);
    }

    return 0;
}


