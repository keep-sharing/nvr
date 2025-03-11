#include "msp2p.h"
#include <unistd.h>
#include "socket.h"
#include "rsa.h"

#define SDEBUG(msg, args...) printf("[%s, %d]"msg"\n", __FUNCTION__, __LINE__, ##args)

static int send_keyfile(SOCKET s)
{
	FILE *fp = NULL;
	size_t num = 0;
	char buf[2048] = {0};

	do {
		if ((fp = fopen(FILE_PUB, "r")) == NULL) {
			SDEBUG("fopen error...");
			break;
		}
		if ((num = fread(buf, 1, sizeof(buf), fp)) <= 0) {
			SDEBUG("fread error...");
			break;
		}
		if (socket_send(s, &num, sizeof(num), 0) <= 0) {
			SDEBUG("send error...");
			break;
		}
		if (socket_send(s, buf, num, 0) <= 0) {
			SDEBUG("send file error...");
			break;
		}
		fclose(fp);
		return 0;
	} while (0);
	if (fp)
		fclose(fp);
	return -1;
}

static int recv_keyfile(SOCKET s, size_t size)
{
	FILE *fp = NULL;
	char buf[2048] = {0}, path[256] = {0};

	snprintf(path, sizeof(path), "%s%d", FILE_RMT, s);
	unlink(path);
	do {
		if (socket_recv(s, buf, size, 0) != size) {
			SDEBUG("recv remote file error, size: %d", size);
			break;
		}
		if ((fp = fopen(path, "w")) == NULL) {
			SDEBUG("fopen error...");
			break;
		}
		fwrite(buf, size, 1, fp);
		fclose(fp);
		return 0;
	} while (0);
	if (fp)
		fclose(fp);
	return -1;
}

int p2p_create_keyfile(void)
{
	return rsa_create(FILE_PUB, FILE_PRV);
}

int p2p_serv_switch_keyfile(SOCKET s)
{
	int ret = -1;
	size_t size = 0;

	do {
		if (send_keyfile(s)) {
			SDEBUG("send keyfile error!!!!!");
			break;
		}
		if (socket_recv(s, &size, sizeof(size), 0) < 0) {
			SDEBUG("recv size error....");
			break;
		}
		if (recv_keyfile(s, size) < 0) {
			SDEBUG("recv keyfile error...");
			break;
		}
		ret = 0;
	} while (0);
	return ret;
}

int p2p_cli_switch_keyfile(SOCKET s)
{
	int ret = -1;
	size_t size = 0;

	do {
		if (socket_recv(s, &size, sizeof(size), 0) < 0) {
			SDEBUG("recv size error....");
			break;
		}
		if (recv_keyfile(s, size) < 0) {
			SDEBUG("recv keyfile error...");
			break;
		}
		if (send_keyfile(s)) {
			SDEBUG("send keyfile error!!!!!");
			break;
		}
		ret = 0;
	} while (0);
	return ret;
}

int p2p_send(SOCKET s, void *data, int dsize, void *sbuf, int ssize)
{
	int ensize = 0, osize = 0;
	char path[256] = {0};

	snprintf(path, sizeof(path), "%s%d", FILE_RMT, s);
	ensize = rsa_cal_ensize(dsize);

	if (ensize > ssize) {
		return -1;
	}
	if (rsa_encrypt(path, data, dsize, sbuf, &osize)) {
		return -1;
	}
	return socket_send(s, sbuf, osize, 0);
}

int p2p_recv(SOCKET s, void *data, int size, void *rbuf, int rsize)
{
	int nbytes = 0, osize = 0;
	if ((nbytes = socket_recv(s, data, size, 0)) <= 0) {
		return -1;
	}
	if (rsa_decrypt(FILE_PRV, data, nbytes, rbuf, &osize) < 0) {
		SDEBUG("decrypt error...");
		return -1;
	}
	return osize;
}
