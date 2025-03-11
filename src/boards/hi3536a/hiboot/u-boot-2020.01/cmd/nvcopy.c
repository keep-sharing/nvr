// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <stdlib.h>
#include <search.h>
#include <env_internal.h>

#define ENV_MAX_LEN 4096

static int env_copy(const char *name, char *addr)
{
	struct env_entry e = {0};
	struct env_entry *ep = NULL;
	e.key = name;
	e.data = NULL;
	hsearch_r(e, ENV_FIND, &ep, &env_htab, H_HIDE_DOT);
	if (ep == NULL)
		return -1;
	int ret = 0;
	size_t len = strlen(ep->data) + strlen("=") + strlen(ep->key) + 1;
	if (len > ENV_MAX_LEN) {
		printf("argc len should less than %d.\n", ENV_MAX_LEN);
		return -1;
	}

	char *buf = (char *)malloc(len);
	if (buf == NULL) {
		printf("malloc buf failed.\n");
		return -1;
	}
	(void)memset_s(buf, len, 0, len);

	if (strcat_s(buf, len, ep->key) < 0) {
		printf("strcat failed.\n");
		free(buf);
		return -1;
	}

	if (strcat_s(buf, len, "=") < 0) {
		printf("strcat failed.\n");
		free(buf);
		return -1;
	}

	if (strcat_s(buf, len, ep->data) < 0) {
		printf("strcat failed.\n");
		free(buf);
		return -1;
	}

	printf("%s\n", buf);
	if (memcpy_s(addr, len, buf, len) != EOK) {
		printf("memcpy_s failed.\n");
		free(buf);
		return -1;
	}

	free(buf);

	return 0;
}

static int do_env_copy(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 3)
		return CMD_RET_USAGE;

	char *name = (char *)argv[1];
	char *addr = (char *)(uintptr_t)simple_strtoul(argv[2], NULL, 16); /* 2: addr argc, 16: base */

	if ((name == NULL) || (addr == NULL)) {
		printf("invalid arg!\n");
		return CMD_RET_USAGE;
	}

	int ret = env_copy(name, addr);
	if (ret != 0) {
		printf("copy env failed!\n");
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	envcopy,  5,  0,  do_env_copy,
	"[envcopy name addr] copy env value to specified addr",
	"copy env value to specified addr\n"
);
