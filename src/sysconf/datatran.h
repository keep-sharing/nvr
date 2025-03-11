#ifndef _SYS_DT_H_
#define _SYS_DT_H_
//data translate
#include "msdefs.h"
#include "msdb.h"
#include "syscfg.h"

int netDbToInternal(struct network *dbCfg, NETWORKCFG *interNetCfg);

int accessDbToInternal(struct network_more *accessDb, ACCESSCFG *accessCfg);

int emailDbToInternal(struct email *emailDb, MAILCFG *mailcfg);

#endif//_SYS_DT_H_