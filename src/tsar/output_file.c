
/*
 * (C) 2010-2011 Alibaba Group Holding Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "tsar.h"


void
output_file()
{
    int         i, ret = 0, n = 0;
    FILE       *fp = NULL;
    char        detail[LEN_1M] = {0};
    char        s_time[LEN_32] = {0};
    struct      module *mod;
    static char line[LEN_10M] = {0};
    struct tm temp;

    if (!(fp = fopen(conf.output_file_path, "a"))) {
        do_debug(LOG_FATAL, "output_file: can't open or create data file = %s  err=%d\n", conf.output_file_path, errno);
    }
    setbuf(fp, NULL);
    
	localtime_r((long*)&statis.cur_time, &temp);
	snprintf(s_time, sizeof(s_time), "%04d-%02d-%02d %02d:%02d:%02d", 
		temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec);
    strcat(line, s_time);

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = mods[i];
        if (mod->enable && strlen(mod->record)) {
            /* save collect data to output_file */
            n = snprintf(detail, LEN_1M, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            if (n >= LEN_1M - 1) {
                do_debug(LOG_FATAL, "mod %s lenth is overflow %d\n", mod->name, n);
            }
	    /* one for \n one for \0 */
            if (strlen(line) + strlen(detail) >= LEN_10M - 2) {
                do_debug(LOG_FATAL, "tsar.data line lenth is overflow line %d detail %d\n", strlen(line), strlen(detail));
            }
            strcat(line, detail);
            ret = 1;
        }
    }
    strcat(line, "\n");

    if (ret) {
        if (fputs(line, fp) < 0) {
            do_debug(LOG_ERR, "write line error\n");
        }
    }
    if (fclose(fp) < 0) {
        do_debug(LOG_FATAL, "fclose error:%s", strerror(errno));
    }
}
