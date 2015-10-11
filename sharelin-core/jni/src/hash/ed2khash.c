#include "ed2khash.h"
#include <string.h>

#define ED2K_BLOCK 4096 * 2375
//#define BLOCK_SIZE 4096
//#define ED2K_BLOCK 2375
#define true 1
#define false 0

/*************************************************************************
some code is borrowed from ed2k hash utility by Stephane D'Alu, 
http://www.sdalu.com/

*************************************************************************/


void ed2k_init(ed2k_ctx_t* p_ctx)
{
	md4_init(&p_ctx->cur_ctx);
	md4_init(&p_ctx->ed2k_ctx);
	p_ctx->cur_size = 0;
//	p_ctx->ctx_opened = true;
	p_ctx->count = 0;
}

void ed2k_update(ed2k_ctx_t* p_ctx, unsigned char* p_data, uint len)
{
	uint left = ED2K_BLOCK - p_ctx->cur_size;
	
	uint n = len <= left ? len : left;
	
/*	if(p_ctx->ctx_opened == false) {
		md4_init(&p_ctx->cur_ctx);
		p_ctx->ctx_opened = true;
		p_ctx->count++;
	}
*/	
	md4_update(&p_ctx->cur_ctx, p_data, n);
	
	p_ctx->cur_size += n;
	
	if(n == left) {
		md4_t digest;
		md4_finish(&p_ctx->cur_ctx, digest);
		md4_update(&p_ctx->ed2k_ctx, digest, sizeof(md4_t));
		md4_init(&p_ctx->cur_ctx);
		
		p_ctx->cur_size = len - n;
		
		p_ctx->count++;
		
		if(n < len) 
			md4_update(&p_ctx->cur_ctx, p_data + n, len - n);
	}
}


void ed2k_finish(ed2k_ctx_t* p_ctx, md4_t digest)
{
	if (p_ctx->count > 0 && p_ctx->cur_size == 0)
		p_ctx->count--;
	
	if(p_ctx->cur_size > 0 || (p_ctx->cur_size == 0 && p_ctx->count == 0)) {
		md4_finish(&p_ctx->cur_ctx, digest);
		md4_update(&p_ctx->ed2k_ctx, digest, sizeof(md4_t));
	}
	
	if(p_ctx->count > 0) md4_finish(&p_ctx->ed2k_ctx, digest);
}


/*==> RSA Data Security, Inc. MD4 Message-Digest Algorithm =============*/

/* Copyright (C) 1990-2, RSA Data Security, Inc. All rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD4 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD4 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

static unsigned char md4_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


#define FF(a, b, c, d, x, s) {						\
    (a) += (((b) & (c)) | ((~b) & (d))) + (x);				\
    (a)  = ((a) << (s)) | ((a) >> (32-(s)));				\
}
#define GG(a, b, c, d, x, s) {						\
    (a) += (((b)&(c))|((b)&(d))|((c)&(d))) + (x) + (u_int32_t)0x5a827999; \
    (a)  = ((a) << (s)) | ((a) >> (32-(s)));				\
}
#define HH(a, b, c, d, x, s) {						\
    (a) += ((b) ^ (c) ^ (d)) + (x) + (u_int32_t)0x6ed9eba1;		\
    (a)  = ((a) << (s)) | ((a) >> (32-(s)));				\
}

static void md4_transform (u_int32_t state[4], unsigned char block[64]) {
    unsigned int i, j;

    u_int32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    
    for (i = 0, j = 0 ; j < 64 ; i++, j += 4)
	x[i] = ((u_int32_t)block[j]) | (((u_int32_t)block[j+1]) << 8) |
	    (((u_int32_t)block[j+2]) << 16) | (((u_int32_t)block[j+3]) << 24);

    FF (a, b, c, d, x[ 0],   3);	FF (d, a, b, c, x[ 1],   7);
    FF (c, d, a, b, x[ 2],  11);	FF (b, c, d, a, x[ 3],  19);
    FF (a, b, c, d, x[ 4],   3);	FF (d, a, b, c, x[ 5],   7);
    FF (c, d, a, b, x[ 6],  11);	FF (b, c, d, a, x[ 7],  19);
    FF (a, b, c, d, x[ 8],   3);	FF (d, a, b, c, x[ 9],   7);
    FF (c, d, a, b, x[10],  11);	FF (b, c, d, a, x[11],  19);
    FF (a, b, c, d, x[12],   3);	FF (d, a, b, c, x[13],   7);
    FF (c, d, a, b, x[14],  11);	FF (b, c, d, a, x[15],  19);

    GG (a, b, c, d, x[ 0],   3);	GG (d, a, b, c, x[ 4],   5);
    GG (c, d, a, b, x[ 8],   9);	GG (b, c, d, a, x[12],  13);
    GG (a, b, c, d, x[ 1],   3);	GG (d, a, b, c, x[ 5],   5);
    GG (c, d, a, b, x[ 9],   9);	GG (b, c, d, a, x[13],  13);
    GG (a, b, c, d, x[ 2],   3);	GG (d, a, b, c, x[ 6],   5);
    GG (c, d, a, b, x[10],   9);	GG (b, c, d, a, x[14],  13);
    GG (a, b, c, d, x[ 3],   3);	GG (d, a, b, c, x[ 7],   5);
    GG (c, d, a, b, x[11],   9);	GG (b, c, d, a, x[15],  13);

    HH (a, b, c, d, x[ 0],   3);	HH (d, a, b, c, x[ 8],   9);
    HH (c, d, a, b, x[ 4],  11);	HH (b, c, d, a, x[12],  15);
    HH (a, b, c, d, x[ 2],   3);	HH (d, a, b, c, x[10],   9);
    HH (c, d, a, b, x[ 6],  11);	HH (b, c, d, a, x[14],  15);
    HH (a, b, c, d, x[ 1],   3);	HH (d, a, b, c, x[ 9],   9);
    HH (c, d, a, b, x[ 5],  11);	HH (b, c, d, a, x[13],  15);
    HH (a, b, c, d, x[ 3],   3);	HH (d, a, b, c, x[11],   9);
    HH (c, d, a, b, x[ 7],  11);	HH (b, c, d, a, x[15],  15);

    state[0] += a;    state[1] += b;
    state[2] += c;    state[3] += d;
}

static void md4_encode(unsigned char *output, 
		       u_int32_t *input, unsigned int len) {
    unsigned int i, j;
    
    for (i = 0, j = 0; j < len; i++, j += 4) {
	output[j]   = (unsigned char) (input[i]        & 0xff);
	output[j+1] = (unsigned char)((input[i] >> 8)  & 0xff);
	output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
	output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

void md4_init(md4_ctx_t *context) {
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

void md4_update(md4_ctx_t *context, unsigned char *input, unsigned int len) {
    unsigned int i, index, partLen;
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    if ((context->count[0] += ((u_int32_t)len << 3)) < ((u_int32_t)len << 3))
	context->count[1]++;
    context->count[1] += ((u_int32_t)len >> 29);
    
    partLen = 64 - index;
    
    if (len >= partLen) {
	memcpy(&context->buffer[index], input, partLen);
	md4_transform(context->state, context->buffer);
	
	for (i = partLen; i + 63 < len; i += 64)
	    md4_transform(context->state, &input[i]);
	
	index = 0;
    } else {
	i = 0;
    }

    memcpy(&context->buffer[index], &input[i], len-i);
}

void md4_finish (md4_ctx_t *context, md4_t digest) {
    unsigned char bits[8];
    unsigned int index, padLen;

    md4_encode (bits, context->count, 8);
    
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    md4_update (context, md4_padding, padLen);
    
    md4_update (context, bits, 8);
    md4_encode (digest, context->state, 16);
}
