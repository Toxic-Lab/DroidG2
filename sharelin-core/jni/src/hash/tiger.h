#ifndef TIGER_H
#define TIGER_H

typedef unsigned long long int word64;

#if defined(__amd64__)
typedef unsigned int word32;
#else
typedef unsigned long word32;
#endif

typedef unsigned char byte;

#ifdef __cplusplus
extern "C" {
#endif

void tiger(word64 *str, word64 length, word64 res[3]);

#ifdef __cplusplus
}
#endif

#endif
