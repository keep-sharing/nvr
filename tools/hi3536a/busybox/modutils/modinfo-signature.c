/*
 * Show Linux Kernel Module Signature information
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2020.
 *
 * Author: Wang Lei
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#include "libbb.h"
#include "modutils.h"
#include "modinfo-signature.h"

/* sig_id TYPE from linux kernel */
enum {
	PKEY_ID_PGP,
	PKEY_ID_X509,
	PKEY_ID_PKCS7,
	PKEY_ID_TYPE__LAST
};

const char* pkey_ids[PKEY_ID_TYPE__LAST] = {
	"PGP",
	"X509",
	"PKCS#7"
};

struct module_sig {
	uint8_t algo;
	uint8_t hash;
	uint8_t id_type;
	uint8_t signer_len;
	uint8_t key_id_len;
	uint8_t __pad[3];
	uint32_t sig_len;
};

#define MODULE_SIG_MAGIC "~Module signature appended~\n"
#define OUTPUT_PREFIX_WIDTH 16

/*
 * Print pkey_id if signature is existing with right format.
 *
 * Return TRUE if signature existing with right format or no signature, FALSE otherwise.
 *
 * Parameters:
 * 	module - module file buffer
 * 	len - size of buffer 
 *
 * Structure of Module signature:
 * 	Module Raw data
 * 	signature	
 * 	module_sig structure
 * 	MODULE_SIG_STRING
 */
bool print_mod_sig_id(const char *module, size_t len)
{
	const size_t magic_len = sizeof(MODULE_SIG_MAGIC) - 1;
	struct module_sig *sig = NULL;
	int n;

	if ((len > magic_len) && !memcmp(module + len - magic_len, MODULE_SIG_MAGIC, magic_len)) {
		len -= magic_len;
		if (len < sizeof(struct module_sig)) 		
			return FALSE;
		len -= sizeof(struct module_sig);
		sig = (struct module_sig *)(module + len);
		if (sig->id_type >= PKEY_ID_TYPE__LAST) {
			return FALSE;
		} else {
			n = printf("sig_id:");
			while (n++ < OUTPUT_PREFIX_WIDTH)
				bb_putchar(' ');
			printf("%s\n", pkey_ids[sig->id_type]);
		}
	}

	return TRUE;
}

