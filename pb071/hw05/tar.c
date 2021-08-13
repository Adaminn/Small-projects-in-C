#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "tarBase.h"
// some includes here

// TODO implement me

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "invalid switches argc\n");
        return -1;
    }
    int arg_ind = 1;
    if (strcmp("tar", argv[arg_ind++]) != 0){
        fprintf(stderr, "invalid switches argv1\n");
        return -1;
    }
    char* switches = argv[arg_ind++];
    bool v = false, c = false, x = false;
    int usedSwitches = 0;
    if (strchr(switches, 'v') != NULL){
        v = true;
        usedSwitches++;
    }
    if (strchr(switches, 'c') != NULL){
        c = true;
        usedSwitches++;
    }
    if (strchr(switches, 'x') != NULL){
        if (c){
            fprintf(stderr, "invalid switches argv2\n");
            return -1;
        }
        x = true;
        usedSwitches++;
    }

    if (strlen(switches) - usedSwitches != 0){
        fprintf(stderr, "invalid switches argv2\n");
        return -1;
    }
    char blank = '\0';
    if (c){
        char* start = strstr(argv[arg_ind], "--null-character=");
        if(start != NULL) {
            if (strlen(argv[arg_ind]) != 18){
                return -1;
            }
            arg_ind++;
            blank = *(++start);
        }
    }

    char* dest = argv[arg_ind++];


    if (c){
        if ( createTar(dest, argv, arg_ind, argc, v, blank) < 0){
            return -1;
        }
    }
    else if (x)
    {
        if (argc > 4){
            fprintf(stderr, "too many args\n");
            return -1;
        }
        if (extractTar(dest, v) != 0){
            return -1;
        }

    }else
    {
        fprintf(stderr, "invalid switches\n");
        return -1;
    }
    return 0;
}

