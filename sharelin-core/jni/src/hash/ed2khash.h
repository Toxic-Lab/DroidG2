#ifndef ED2KHASH_H
#define ED2KHASH_H
#include <sys/types.h>

typedef unsigned char md4_t[16];
//typedef unsigned int u_int32_t;
typedef struct {
    u_int32_t		state[4];
    u_int32_t		count[2];
    unsigned char	buffer[64];
} md4_ctx_t;

typedef struct  {
	u_int32_t cur_size;
	md4_ctx_t cur_ctx;
	md4_ctx_t ed2k_ctx;
	int ctx_opened;
	int count;
	
} ed2k_ctx_t;

#ifdef __cplusplus
extern "C" {
#endif 

void md4_init(md4_ctx_t *);
void md4_update(md4_ctx_t *, unsigned char *, unsigned int);
void md4_finish(md4_ctx_t *, md4_t);

void ed2k_init(ed2k_ctx_t* p_ctx);
void ed2k_update(ed2k_ctx_t* p_ctx, unsigned char* p_data, unsigned int len);
void ed2k_finish(ed2k_ctx_t* p_ctx, md4_t);

#ifdef __cplusplus
}
#endif 

#endif //ED2KHASH_H
