#include "mssp_utils.h"
#include "md5.h"

void switch_from_raw_packet(const u_char *packet, char values[][128])
{
    int m = 0, n = 0;

    u_char *data = (u_char *)packet;
    data = (u_char *)packet;

    while(*data != '\0')
	{
        if(*data == ';')
		{
			m++;
            data++;
            n = 0;
            continue;
        }
        values[m][n++] = *data;
        data++;
    }
}

static void generate_md5_str(const char *in, int insize, char *out, int outsize)
{
	unsigned char buf[16] = {0};

	MD5((const unsigned char *)in, (size_t)insize, buf);
	snprintf(out, outsize, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		buf[0], buf[1], buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],
		buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]);
}

void get_random_md5str(char *dest, int dstsize)
{
	struct
	{
		struct timeval ts;
		unsigned int counter;
	} seeddata;
	static unsigned counter = 0;

	gettimeofday(&seeddata.ts, NULL);
	seeddata.counter = ++counter;
	generate_md5_str((const char *)&seeddata, sizeof(seeddata), dest, dstsize);
	int i = 0;

	for (i = 0; i < dstsize; i++) 
	{
		if (dest[i] == '\0' || dest[i] == ';')
			dest[i] = 0xff;
	}
}
