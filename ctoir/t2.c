#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 256   // 2^8

void f1(unsigned char *a, unsigned char *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int h1(char *key, unsigned char *S) {

    int len = strlen(key);
    int j = 0;

    for(int i = 0; i < N; i++)
        S[i] = i;

    for(int i = 0; i < N; i++) {
        j = (j + S[i] + key[i % len]) % N;

        f1(&S[i], &S[j]);
    }

    return 0;
}

int g1(unsigned char *S, unsigned char *plaintext, unsigned char *ciphertext, int size) {

    int i = 0;
    int j = 0;

    for(size_t n = 0, len = size; n < len; n++) {
        i = (i + 1) % N;
        j = (j + S[i]) % N;

        f1(&S[i], &S[j]);
        int rnd = S[(S[i] + S[j]) % N];

        ciphertext[n] = rnd ^ plaintext[n];

    }

    return 0;
}

int t2(char *key, unsigned char *plaintext, unsigned char *ciphertext, int size) {

    unsigned char S[N];
    h1(key, S);

    g1(S, plaintext, ciphertext,size);

    return 0;
}