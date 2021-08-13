
#ifndef HW05_TARBASE_H
#define HW05_TARBASE_H
int createTar(char* tarPath, char** files, int start, int filesCount, bool v, char blank);
int extractTar(char* path, bool v);

#endif //HW05_TARBASE_H
