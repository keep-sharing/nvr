
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


#ifndef TSAR_FRAMEWORK_H
#define TSAR_FRAMEWORK_H


#include "define.h"

struct mod_info {
    char    hdr[LEN_128];
    int     summary_bit;    /* bit set indefi summary */
    int     merge_mode;
    int     stats_opt;
};

struct module {

    char    name[LEN_32];
    char    opt_line[LEN_32];
    char    record[LEN_1M];
    char    usage[LEN_256];
    char    parameter[LEN_256];
    char    print_item[LEN_256];

    struct  mod_info *info;
    int     enable;
    int     spec;
    int     p_item;

    /* private data used by framework*/
    int     n_item;
    int     n_col;
    long    n_record;

    int     pre_flag:4;
    int     st_flag:4;

    U_64   *pre_array;
    U_64   *cur_array;
    double *st_array;
    double *max_array;
    double *mean_array;
    double *min_array;

    /* callback function of module */
    void (*data_collect) (struct module *, char *);
    void (*set_st_record) (struct module *, double *, U_64 *, U_64 *, int);
};


void register_mod_fields(struct module *mod, const char *opt, const char *usage,
        struct mod_info *info, int n_col, void *data_collect, void *set_st_record);
void set_mod_record(struct module *mod, const char *record);
void init_module_fields();
int reload_modules(const char *s_mod);
#ifdef OLDTSAR
void reload_check_modules();
#endif
void load_modules();
void free_modules();
void collect_record();
time_t read_line_to_module_record(char *line);
int  collect_record_stat();
void disable_col_zero();

//for mod register
void mod_percpu_register(struct module *mod);
void mod_pcsw_register(struct module *mod);
void mod_partition_register(struct module *mod);
void mod_nginx_register(struct module *mod);
void mod_ncpu_register(struct module *mod);
void mod_mem_register(struct module *mod);
void mod_lvs_register(struct module *mod);
void mod_load_register(struct module *mod);
void mod_io_register(struct module *mod);
void mod_haproxy_register(struct module *mod);
void mod_cpu_register(struct module *mod);
void mod_apache_register(struct module *mod);
void mod_pernic_register(struct module *mod);
void mod_proc_register(struct module *mod);
void mod_squid_register(struct module *mod);
void mod_swap_register(struct module *mod);
void mod_tcp_register(struct module *mod);
void mod_tcpx_register(struct module *mod);
void mod_traffic_register(struct module *mod);
void mod_udp_register(struct module *mod);

//custom mod
void mod_mscore_register(struct module *mod);

//
int tsar_get_key_value(const char *src, const char *key, char *dst, int len, char eof);

#endif
