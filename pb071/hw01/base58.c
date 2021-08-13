#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char* base58;


unsigned int asciiCodesToNum(int const words[4]) // convert four ascii codes to one number
{
    unsigned num = 0;
    for (int i = 0; i < 3; ++i) { // iterate through ascii codes
        num |= words[i];
        num = num << 8;
    }
    num |= words[3];
    return num;
}

void numToAscii(unsigned int n) // convert from one number to four ascii codes
{
    int ascii;
    unsigned parts[4] = {0, 0,0, 0};
    for (int i = 3; i >= 0; i--) {
        parts[i] = 255 << (3-i)*8;

    }

    for (int i = 0; i < 4; ++i) {
        ascii = (int) (n & parts[i]) >> (8 * 3 - 8 * i);
        putchar(ascii);
    }
}


void numToBase58(unsigned int num)
{
    int base = 58;
    int rem;
    char result[6];
    char c;
    for (int i = 0; i < 6; ++i) {
        rem = (int) (num % base);
        num /= base;
        c = base58[rem];
        result[5 - i] = c;
    }
    for (int i = 0; i < 6; ++i) {
        printf("%c", result[i]);
    }
}

void asciiToBase58(int words[4])
{
    unsigned int num = asciiCodesToNum(words);
    numToBase58(num);
}

bool encode(void)
{
    base58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    int words[4] = { '\0', '\0', '\0', '\0' };
    int entered = 0;
    int c;
    while ((c = getchar()) != EOF) {
        words[entered] = c;
        entered++;
        if (entered < 4) {
            continue;
        }
        entered = 0;
        asciiToBase58(words);
    }
    if (entered != 0) {
        for (int i = entered; i < 4; ++i) {
            words[i] = '\0';
        }
        asciiToBase58(words);
    }
    printf("\n");
    return true;
}

unsigned int baseCodesToNum(int const codes[6])
{
    unsigned int prev = codes[0];
    for (int i = 1; i < 6; ++i) {
        prev = prev * 58 + codes[i];
    }
    return prev;
}

bool base58toCodes(int const words[6], int codes[6])
{
    for (int i = 0; i < 6; ++i) {
        for (int n = 0; n < 58; ++n) {
            if (base58[n] == words[i]) {
                codes[i] = n;
                break;
            }
            if (n == 57) {
                return false;
            }
        }
    }
    return true;
}

bool decode(void)
{
    base58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    int words[6];
    int codes[6];
    int entered = 0;
    int c;
    unsigned int num;

    while ((c = getchar()) != EOF) {
        if (isspace(c)) {
            continue;
        }
        words[entered] = c;
        entered++;
        if (entered < 6) {
            continue;
        }
        entered = 0;

        if (!base58toCodes(words, codes)) {
            return false;
        }

        num = baseCodesToNum(codes);
        numToAscii(num);
    }

    if (entered != 0) {
        return false;
    }
    return true;
}

// ================================
// DO NOT MODIFY THE FOLLOWING CODE
// ================================
int main(int argc, char **argv)
{
    if ((argc == 1) || (argc == 2 && !strcmp(argv[1], "-e"))) {
        encode();
    } else if (argc == 2 && !strcmp(argv[1], "-d")) {
        if (!decode()) {
            fprintf(stderr, "Input isn't encoded via Base58!\n");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Invalid switch, use -e or -d\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
