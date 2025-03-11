/*
 * ***************************************************************
 * Filename:        utils.c
 * Created at:      2017.05.10
 * Description:     common utils API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include "utils.h"


const MF_U32 crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


/* Return a 32-bit CRC of the contents of the buffer. */
inline MF_U32
crc_32(const void *buf, MF_U32 size)
{
    const MF_U8 *p;
    MF_U32 crc;

    p = buf;
    crc = ~0U;

    while (size > 0) {
        crc = crc32_tab[(crc ^ *p++) & 0xff] ^ (crc >> 8);
        size--;
    }

    return crc ^ ~0U;
}


MF_U8 PADDING[] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                  };

static void
MD5Encode(MF_U8 *output, MF_U32 *input, MF_U32 len)
{
    MF_U32 i = 0, j = 0;
    while (j < len) {
        output[j] = input[i] & 0xFF;
        output[j + 1] = (input[i] >> 8) & 0xFF;
        output[j + 2] = (input[i] >> 16) & 0xFF;
        output[j + 3] = (input[i] >> 24) & 0xFF;
        i++;
        j += 4;
    }
}
static void
MD5Decode(MF_U32 *output, MF_U8 *input, MF_U32 len)
{
    MF_U32 i = 0, j = 0;
    while (j < len) {
        output[i] = (input[j]) |
                    (input[j + 1] << 8) |
                    (input[j + 2] << 16) |
                    (input[j + 3] << 24);
        i++;
        j += 4;
    }
}
static void
MD5Transform(MF_U32 state[4], MF_U8 block[64])
{
    MF_U32 a = state[0];
    MF_U32 b = state[1];
    MF_U32 c = state[2];
    MF_U32 d = state[3];
    MF_U32 x[64];
    MD5Decode(x, block, 64);
    FF(a, b, c, d, x[ 0], 7, 0xd76aa478); /* 1 */
    FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */
    FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */
    FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */
    FF(a, b, c, d, x[ 4], 7, 0xf57c0faf); /* 5 */
    FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */
    FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */
    FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */
    FF(a, b, c, d, x[ 8], 7, 0x698098d8); /* 9 */
    FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
    FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], 7, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[ 1], 5, 0xf61e2562); /* 17 */
    GG(d, a, b, c, x[ 6], 9, 0xc040b340); /* 18 */
    GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
    GG(a, b, c, d, x[ 5], 5, 0xd62f105d); /* 21 */
    GG(d, a, b, c, x[10], 9,  0x2441453); /* 22 */
    GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
    GG(a, b, c, d, x[ 9], 5, 0x21e1cde6); /* 25 */
    GG(d, a, b, c, x[14], 9, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
    GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
    GG(a, b, c, d, x[13], 5, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[ 2], 9, 0xfcefa3f8); /* 30 */
    GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
    GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[ 5], 4, 0xfffa3942); /* 33 */
    HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
    HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[ 1], 4, 0xa4beea44); /* 37 */
    HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
    HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
    HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], 4, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
    HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
    HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
    HH(a, b, c, d, x[ 9], 4, 0xd9d4d039); /* 45 */
    HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II(a, b, c, d, x[ 0], 6, 0xf4292244); /* 49 */
    II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
    II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
    II(a, b, c, d, x[12], 6, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
    II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
    II(a, b, c, d, x[ 8], 6, 0x6fa87e4f); /* 57 */
    II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
    II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[ 4], 6, 0xf7537e82); /* 61 */
    II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
    II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static void
MD5Init(MD5_CTX *context)
{
    context->count[0] = 0;
    context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
}
static void
MD5Update(MD5_CTX *context, MF_U8 *input, MF_U32 inputlen)
{
    MF_U32 i = 0, index = 0, partlen = 0;
    index = (context->count[0] >> 3) & 0x3F;
    partlen = 64 - index;
    context->count[0] += inputlen << 3;
    if (context->count[0] < (inputlen << 3)) {
        context->count[1]++;
    }
    context->count[1] += inputlen >> 29;

    if (inputlen >= partlen) {
        memcpy(&context->buffer[index], input, partlen);
        MD5Transform(context->state, context->buffer);
        for (i = partlen; i + 64 <= inputlen; i += 64) {
            MD5Transform(context->state, &input[i]);
        }
        index = 0;
    } else {
        i = 0;
    }
    memcpy(&context->buffer[index], &input[i], inputlen - i);
}

static void
MD5Final(MD5_CTX *context, MF_U8 digest[16])
{
    MF_U32 index = 0, padlen = 0;
    MF_U8 bits[8];
    index = (context->count[0] >> 3) & 0x3F;
    padlen = (index < 56) ? (56 - index) : (120 - index);
    MD5Encode(bits, context->count, 8);
    MD5Update(context, PADDING, padlen);
    MD5Update(context, bits, 8);
    MD5Encode(digest, context->state, 16);
}

void
MD5string(MF_S8 *in, MF_U8 out[MD5_LEN])
{
    static const MF_S8 hex[] = "0123456789abcdef";
    MD5_CTX context;
    MF_U32 i;
    MF_U8 digest[16];
    MF_U32 len = strlen(in);

    MFinfo("in:%s", in);
    MFinfo("len:%d", len);
    MD5Init(&context);
    MD5Update(&context, (MF_U8 *)in, len);
    MD5Final(&context, digest);

    for (i = 0; i < 16; i++) {
        out[2 * i] = hex[digest[i] >> 4];
        out[2 * i + 1] = hex[digest[i] & 0x0f];
    }
    out[2 * i] = '\0';
    MFinfo("MD5:%s", out);
}

MF_S32
mf_random(MF_S32 min, MF_S32 max)
{
    if (max <= min) {
        return max;
    }

    srand(get_time_us());
    return ((rand() % ((max) - (min))) + (min));
}

MF_S32
mf_random2(MF_S32 min, MF_S32 max, MF_U32 seed)
{
    srand(get_time_us());
    seed += rand();
    return ((seed % ((max) - (min))) + (min));
}

MF_S32
disk_open(const MF_S8 *path, MF_BOOL bO_DIRECT)
{
    if (bO_DIRECT) {
        return open(path, O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
    } else {
        return open(path, O_RDWR | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
    }
}

MF_S32
disk_close(MF_S32 fd)
{
    ms_system("echo 1 > /proc/sys/vm/drop_caches");
    return close(fd);
}

inline MF_S32
disk_read(MF_S32 fd, void *buf, MF_S32 count, off64_t offset)
{
#if 1
    int res = 0;
    if (offset >= 0) {
        res = pread(fd, buf, count, offset);
    } else {
        res = read(fd, buf, count);
    }
    if (res < 0) {
        perror("disk_read\n");
        MFerr("fd = %d conut = %d, offset = %llx res[%d]\n",
              fd, count, offset, res);
    }
    return res;
#else
    int res = 0;
    int diff = 0;
    MF_S32 size = count;
    off64_t offset_n = offset;
    char *readBuf = (char *)buf;

    if (offset >= 0) {
        offset_n = offset & (~(512 - 1));
        if (offset != offset_n) {
            diff = offset - offset_n;
            size = ALIGN_UP(count + diff, 512);
            if (count < 512) {
                readBuf = ms_valloc(size);
            }
        }
        res = pread(fd, readBuf, size, offset_n);
    } else {
        res = read(fd, readBuf, size);
    }
    if (res < 0) {
        perror("disk_read\n");
        MFerr("fd = %d conut = %d, offset = %llx res[%d]\n",
              fd, count, offset, res);
    }

    if (count < 512 || diff > 0) {
        memmove(buf, readBuf + diff, count);
    }

    if (readBuf != buf) {
        ms_free(readBuf);
    }

    return res == size ? count : res;
#endif
}

inline MF_S32
disk_write(MF_S32 fd, const void *buf, MF_S32 count, off64_t offset)
{
    MF_S32 res = 0;
//    printf("========conut =%d, offset = %llx\n", count, offset);
//    printf("++++++++end== = %llx\n",offset+count);
    if (offset >= 0) {
        res = pwrite(fd, buf, count, offset);
    } else {
        res = write(fd, buf, count);
    }

    if (res < 0) {
        res = -errno;
        MFerr("fd = %d conut = %d, offset = %llx res[%d] error[%s]\n",
              fd, count, offset, res, strerror(errno));
    }
    return res;
}

static inline unsigned long long le64_to_int(unsigned char *le64)
{
    return (((unsigned long long)le64[7] << 56) +
            ((unsigned long long)le64[6] << 48) +
            ((unsigned long long)le64[5] << 40) +
            ((unsigned long long)le64[4] << 32) +
            ((unsigned long long)le64[3] << 24) +
            ((unsigned long long)le64[2] << 16) +
            ((unsigned long long)le64[1] << 8) +
            (unsigned long long)le64[0]);
}

MF_S32
disk_find_gpt_sig(MF_S8 *devname, MF_U32 *pSector)
{
    MF_S32 fd = -1, res = MF_FAILURE;
    MF_U32 sector;
    MF_S8 *buf = NULL;
    gpt_header gpth;

    *pSector = 512;
    fd = disk_open(devname, MF_NO);
    if (fd < 0) {
        MFinfo("open fail %s", devname);
        return res;
    }
    if (ioctl(fd, BLKSSZGET, &sector) < 0) {
        sector = 512;
    }
    *pSector = sector;
    buf = ms_malloc(sector);

    lseek64(fd, sector, SEEK_SET);
    if (disk_read(fd, buf, sector, -1) < 0) {
        MFerr("read fail");
        disk_close(fd);
        ms_free(buf);
        return res;
    }
    memcpy(&gpth, buf, sector);

    if (le64_to_int(gpth.signature) == GPT_HEADER_SIGNATURE) {
        MFinfo("find gpt sig %llu", le64_to_int(gpth.signature));
        res = MF_SUCCESS;
    }

    disk_close(fd);
    ms_free(buf);
    return  res;
}

MF_S32
disk_erase_MBR512(MF_S8 *devname)
{
    MF_S32 fd = -1, res = MF_FAILURE;
    MF_U32 sector;
    MF_S8 *buf = NULL;

    fd = disk_open(devname, MF_NO);
    if (fd < 0) {
        MFinfo("open fail %s", devname);
        return res;
    }
    if (ioctl(fd, BLKSSZGET, &sector) < 0) {
        sector = 512;
    }
    buf = ms_malloc(sector);

    lseek64(fd, 0, SEEK_SET);
    if (disk_write(fd, buf, sector, -1) < 0) {
        MFerr("read fail");
        disk_close(fd);
        ms_free(buf);
        return res;
    }
    res = MF_SUCCESS;

    disk_close(fd);
    ms_free(buf);
    return  res;
}

MF_U32
check_sum(void *pData, MF_S32 size)
{
    MF_S8 *pStart = (MF_S8 *)pData;
    MF_U32 sum = 0 ;
    MF_S32 i;

    for (i = 0; i < size; i++) {
        sum += pStart[i];
    }
    //MFinfo("check sum = %d", sum);
    return sum;
}

void
select_usleep(MF_U32 us)
{
    struct timeval tv;

    tv.tv_sec = us / 1000000;
    tv.tv_usec = us % 1000000;
    MF_S32 err;

    do {
        err = select(0, NULL, NULL, NULL, &tv);
    } while (err < 0 && errno == EINTR);
}

MF_U64 inline
get_time_us()
{
    struct timeval stTime;

    gettimeofday(&stTime, NULL);

    return (stTime.tv_sec * 1000000LLU) + stTime.tv_usec;
}

MF_U32 inline
get_time_ms()
{
    struct timeval stTime;

    gettimeofday(&stTime, NULL);

    return  stTime.tv_sec * 1000 + stTime.tv_usec / 1000;
}

void
time_to_string_utc(MF_PTS ntime, MF_S8 stime[32])
{
    struct tm *temp;
    time_t t = (time_t)ntime;

    temp = gmtime(&t);
    snprintf(stime, 32, "%4d-%02d-%02d %02d:%02d:%02d",
             temp->tm_year + 1900, temp->tm_mon + 1, temp->tm_mday,
             temp->tm_hour, temp->tm_min, temp->tm_sec);
}

void
time_to_string_local(MF_PTS ntime, MF_S8 stime[32])
{
    struct tm temp;
    time_t t = (time_t)ntime;

    localtime_r(&t, &temp);
    snprintf(stime, 32, "%4d-%02d-%02d %02d:%02d:%02d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday,
             temp.tm_hour, temp.tm_min, temp.tm_sec);
}

MF_PTS
string_to_time_local(const MF_S8 stime[32])
{
    struct tm tm_;
    if (stime) {
        strptime(stime, "%Y-%m-%d %H:%M:%S", &tm_); //switch to struct tm
        tm_.tm_isdst = -1;
        return mktime(&tm_);
    }
    return 0;
}

// clear space in the end
MF_S8 *rtrim(char *str)
{
    if (str == NULL || *str == '\0') {
        return str;
    }

    MF_S32 len = strlen(str);
    MF_S8 *p = str + len - 1;
    while (p >= str && (*p) == ' ') {
        *p = '\0';
        --p;
    }

    return str;
}

// clear space in the head
MF_S8 *ltrim(MF_S8 *str)
{
    if (str == NULL || *str == '\0') {
        return str;
    }

    MF_S32 len = 0;
    MF_S8 *p = str;
    while (*p != '\0' && (*p) == ' ')    {
        ++p;
        ++len;
    }
    if (len) {
        memmove(str, p, strlen(str) - len + 1);
    }

    return str;
}

MF_S8 *
mf_trim(MF_S8 *str)
{
    str = rtrim(str);
    str = ltrim(str);

    return str;
}

MF_S32
mf_cond_init(MF_COND_T *cond)
{
    return pthread_cond_init(cond, NULL);
}

MF_S32
mf_cond_uninit(MF_COND_T *cond)
{
    return pthread_cond_destroy(cond);
}

MF_S32
mf_cond_wait(MF_COND_T *cond, MF_MUTEX_T *mutex)
{
    return pthread_cond_wait(cond, mutex);
}

MF_S32
mf_cond_timedwait(MF_COND_T *cond, MF_MUTEX_T *mutex, MF_U64 us)
{
    struct timespec outtime;
    struct timeval now;

    gettimeofday(&now, NULL);
    now.tv_usec += us;
    if (now.tv_usec >= 1000000) {
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
    }
    outtime.tv_nsec = now.tv_usec * 1000;
    outtime.tv_sec = now.tv_sec;
    return pthread_cond_timedwait(cond, mutex, &outtime);
}

MF_S32
mf_cond_signal(MF_COND_T *cond)
{
    return pthread_cond_signal(cond);
}

MF_S8 *
mf_time_to_string(MF_PTS time)
{
    static MF_S8 buff[32];

    time_to_string_local(time, buff);

    return buff;
}

void
mf_time_to_string2(MF_PTS time, MF_S8 *buff)
{
    time_to_string_local(time, buff);
}

MF_U32
mf_time_second_to_day(MF_PTS second)
{
    return (second + DAY_SECONDS - 1) / DAY_SECONDS;
}

MF_U32
mf_date_to_second(MF_U32 year, MF_U32 month)
{
    MF_S8 timeStr[64];
    struct tm tm = {0};

    if (month > 12) {
        year += month / 12;
        month = month % 12;
    }
    sprintf(timeStr, "%04d-%02d-01/00:00:00", year, month);
    MF_S8 fmt[] = "%Y-%m-%d/%H:%M:%S";
    strptime(timeStr, fmt, &tm);
    tm.tm_isdst = -1;

    return  mktime(&tm);
}

MF_U32
mf_date_to_day(MF_U32 year, MF_U32 month)
{
    MF_S8 timeStr[64];
    struct tm tm = {0};

    if (month > 12) {
        year += month / 12;
        month = month % 12;
    }
    sprintf(timeStr, "%04d-%02d-01/00:00:00", year, month);
    MF_S8 fmt[] = "%Y-%m-%d/%H:%M:%S";
    strptime(timeStr, fmt, &tm);
    tm.tm_isdst = -1;

    return  mf_time_second_to_day(mktime(&tm));
}

MF_S32
mf_set_task_policy(TASK_HANDLE thread_id, MF_S32 priority)
{
    struct sched_param param;
    MF_S32 policy = SCHED_RR;

    if (thread_id == 0) {
        return MF_FAILURE;
    }

    memset(&param, 0, sizeof(param));

    param.sched_priority = priority;
    if (pthread_setschedparam(thread_id, policy, &param) < 0) {
        perror("pthread_setschedparam");
    }

    pthread_getschedparam(thread_id, &policy, &param);
    if (param.sched_priority != priority) {
        MFerr("set pthread priority[%d] failed", priority);
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}


