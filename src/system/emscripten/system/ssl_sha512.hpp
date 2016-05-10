/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2016
   Author(s): Christophe Grosjean

   sha512 headers
*/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/bitfu.hpp"


enum: int {
    SHA512_DIGEST_LENGTH = 64
};


class SslSha512_direct
{
     struct sha512 {
        uint64_t len;     /* processed message length */
        uint64_t h[8];    /* hash state */
        uint8_t buf[128]; /* message block buffer */
    } sha512;

    static uint64_t ror(uint64_t n, int k) {
        return (n >> k) | (n << (64-k));
    }

    static uint64_t Ch(uint64_t x,uint64_t y,uint64_t z) {
        return z ^ (x & (y ^ z));
    }

    static uint64_t Maj(uint64_t x,uint64_t y,uint64_t z) {
        return (x & y) | (z & (x | y));
    }

    static uint64_t S0(uint64_t x) {
        return ror(x,28) ^ ror(x,34) ^ ror(x,39);
    }

    static uint64_t S1(uint64_t x) {
        return ror(x,14) ^ ror(x,18) ^ ror(x,41);
    }

    static uint64_t R0(uint64_t x) {
        return ror(x,1) ^ ror(x,8) ^ (x>>7);
    }

    static uint64_t R1(uint64_t x) {
        return ror(x,19) ^ ror(x,61) ^ (x>>6);
    }


    static void processblock(struct sha512 *s, const uint8_t *buf)
    {
        uint64_t W[80], t1, t2, a, b, c, d, e, f, g, h;
        int i;

        static const uint64_t K[80] = {
        0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
        0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
        0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
        0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
        0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
        0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
        0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
        0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
        0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
        0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
        0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
        0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
        0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
        0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
        0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
        0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
        0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
        0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
        0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
        0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
        };

        for (i = 0; i < 16; i++) {
            W[i] = static_cast<uint64_t>(buf[8*i])<<56;
            W[i] |= static_cast<uint64_t>(buf[8*i+1])<<48;
            W[i] |= static_cast<uint64_t>(buf[8*i+2])<<40;
            W[i] |= static_cast<uint64_t>(buf[8*i+3])<<32;
            W[i] |= static_cast<uint64_t>(buf[8*i+4])<<24;
            W[i] |= static_cast<uint64_t>(buf[8*i+5])<<16;
            W[i] |= static_cast<uint64_t>(buf[8*i+6])<<8;
            W[i] |= buf[8*i+7];
        }
        for (; i < 80; i++)
            W[i] = R1(W[i-2]) + W[i-7] + R0(W[i-15]) + W[i-16];
        a = s->h[0];
        b = s->h[1];
        c = s->h[2];
        d = s->h[3];
        e = s->h[4];
        f = s->h[5];
        g = s->h[6];
        h = s->h[7];
        for (i = 0; i < 80; i++) {
            t1 = h + S1(e) + Ch(e,f,g) + K[i] + W[i];
            t2 = S0(a) + Maj(a,b,c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        s->h[0] += a;
        s->h[1] += b;
        s->h[2] += c;
        s->h[3] += d;
        s->h[4] += e;
        s->h[5] += f;
        s->h[6] += g;
        s->h[7] += h;
    }

    static void pad(struct sha512 *s)
    {
        unsigned r = s->len % 128;

        s->buf[r++] = 0x80;
        if (r > 112) {
            memset(s->buf + r, 0, 128 - r);
            r = 0;
            processblock(s, s->buf);
        }
        memset(s->buf + r, 0, 120 - r);
        s->len *= 8;
        s->buf[120] = s->len >> 56;
        s->buf[121] = s->len >> 48;
        s->buf[122] = s->len >> 40;
        s->buf[123] = s->len >> 32;
        s->buf[124] = s->len >> 24;
        s->buf[125] = s->len >> 16;
        s->buf[126] = s->len >> 8;
        s->buf[127] = s->len;
        processblock(s, s->buf);
    }

    static void sha512_init(struct sha512 *s)
    {
        s->len = 0;
        s->h[0] = 0x6a09e667f3bcc908ULL;
        s->h[1] = 0xbb67ae8584caa73bULL;
        s->h[2] = 0x3c6ef372fe94f82bULL;
        s->h[3] = 0xa54ff53a5f1d36f1ULL;
        s->h[4] = 0x510e527fade682d1ULL;
        s->h[5] = 0x9b05688c2b3e6c1fULL;
        s->h[6] = 0x1f83d9abfb41bd6bULL;
        s->h[7] = 0x5be0cd19137e2179ULL;
    }

    static void sha512_sum(struct sha512 *s, uint8_t *md)
    {
        int i;

        pad(s);
        for (i = 0; i < 8; i++) {
            md[8*i] = s->h[i] >> 56;
            md[8*i+1] = s->h[i] >> 48;
            md[8*i+2] = s->h[i] >> 40;
            md[8*i+3] = s->h[i] >> 32;
            md[8*i+4] = s->h[i] >> 24;
            md[8*i+5] = s->h[i] >> 16;
            md[8*i+6] = s->h[i] >> 8;
            md[8*i+7] = s->h[i];
        }
    }

    static void sha512_update(struct sha512 *s, const uint8_t *m, unsigned long len)
    {
        const uint8_t *p = m;
        unsigned r = s->len % 128;

        s->len += len;
        if (r) {
            if (len < 128 - r) {
                memcpy(s->buf + r, p, len);
                return;
            }
            memcpy(s->buf + r, p, 128 - r);
            len -= 128 - r;
            p += 128 - r;
            processblock(s, s->buf);
        }
        for (; len >= 128; len -= 128, p += 128)
            processblock(s, p);
        memcpy(s->buf, p, len);
    }

    public:
    SslSha512_direct()
    {
        SslSha512_direct::sha512_init(&this->sha512);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        SslSha512_direct::sha512_update(&this->sha512, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(SHA512_DIGEST_LENGTH == out_data_size);
        SslSha512_direct::sha512_sum(&this->sha512, out_data);
    }
};


class SslHMAC_Sha512_direct
{
    uint8_t k_ipad[128];
    uint8_t k_opad[128];
    SslSha512_direct context;

    public:
    SslHMAC_Sha512_direct(const uint8_t * const key, size_t key_len)
        : k_ipad{
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
         },
         k_opad{
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
         }
    {
         const uint8_t * k = key;
         if (key_len > 128) {
             unsigned char digest[SHA512_DIGEST_LENGTH];
             SslSha512_direct sha512;
             sha512.update(digest, SHA512_DIGEST_LENGTH);
             sha512.final(digest, SHA512_DIGEST_LENGTH);
             key_len = SHA512_DIGEST_LENGTH;
             k = key;
         }
         size_t i;
         for (i = 0; i < key_len; i++){
            k_ipad[i] ^= k[i];
            k_opad[i] ^= k[i];
         }
         context.update(k_ipad, 128);
    }

    ~SslHMAC_Sha512_direct()
    {
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        context.update(data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(SHA512_DIGEST_LENGTH == out_data_size);
        context.final(out_data, SHA512_DIGEST_LENGTH);

        SslSha512_direct sha512;
        sha512.update(this->k_opad, 128);
        sha512.update(out_data, SHA512_DIGEST_LENGTH);
        sha512.final(out_data, SHA512_DIGEST_LENGTH);
    }
};



typedef SslHMAC_Sha512_direct SslHMAC_Sha512;
typedef SslSha512_direct SslSha512;

