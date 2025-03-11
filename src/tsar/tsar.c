
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



struct statistic statis;
struct configure conf;
struct module   *mods[MAX_MOD_NUM];


void
usage()
{
    int    i;
    struct module *mod;

    fprintf(stderr,
            "Usage: tsar [options]\n"
            "Options:\n"
#ifdef OLDTSAR
            /*used for check alert*/
            "    -check         display last record for alert\n"
            /*end*/
#endif
            "    --check/-C     display last record for alert.example:tsar --check / tsar --check --cpu --io\n"
            "    --watch/-w     display last records in N mimutes. example:tsar --watch 30 / tsar --watch 30 --cpu --io\n"
            "    --cron/-c      run in cron mode, output data to file\n"
            "    --interval/-i  specify intervals numbers, in minutes if with --live, it is in seconds\n"
            "    --list/-L      list enabled modules\n"
            "    --live/-l      running print live mode, which module will print\n"
            "    --file/-f      specify a filepath as input\n"
            "    --ndays/-n     show the value for the past days (default: 1)\n"
            "    --date/-d      show the value for the specify day(n or YYYYMMDD)\n"
            "    --merge/-m     merge multiply item to one\n"
            "    --debug/-D     set debug level INFO DEBUG WARN ERROR FATAL\n"
            "    --stdout/-S     set log stdout on/off \n"
            "    --spec/-s      show spec field data, tsar --cpu -s sys,util\n"
            "    --item/-I      show spec item data, tsar --io -I sda\n"
            "    --msop/-M      MS NVR Operation\n"
            "                   run msop.           example:tsar --msop run\n"
            "                   stop msop.          example:tsar --msop stop\n"
            "                   show msop log.      example:tsar --msop log\n"
            "    --help/-h      help\n");

    fprintf(stderr,
            "Modules Enabled:\n"
           );

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = mods[i];
        fprintf(stderr, "%s\n", mod->usage);
    }

    exit(0);
}


struct option longopts[] = {
    { "cron", no_argument, NULL, 'c' },
    { "check", no_argument, NULL, 'C' },
    { "watch", required_argument, NULL, 'w' },
    { "interval", required_argument, NULL, 'i' },
    { "list", no_argument, NULL, 'L' },
    { "live", no_argument, NULL, 'l' },
    { "file", required_argument, NULL, 'f' },
    { "ndays", required_argument, NULL, 'n' },
    { "date", required_argument, NULL, 'd' },
    { "merge", no_argument, NULL, 'm' },
    { "debug", required_argument, NULL, 'D' },
    { "spec", required_argument, NULL, 's' },
    { "item", required_argument, NULL, 'I' },
    { "msop", required_argument, NULL, 'M' },
    { "stdout", required_argument, NULL, 'S' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0},
};


static void
main_init(int argc, char **argv)
{
    int    opt, oind = 0;
#ifdef OLDTSAR
    /* check option for tsar1.0 */
    if (argc >= 2) {
        if (!strcmp(argv[1], "-check") && argc == 2) {
            conf.running_mode = RUN_CHECK;
            conf.print_mode = DATA_DETAIL;
            conf.print_interval = 60;
            conf.print_tail = 0;
            conf.print_nline_interval = conf.print_interval;
            return;
        }
    }
    /*end*/
#endif
    while ((opt = getopt_long(argc, argv, ":cCw:i:Llf:n:d:s:IMSD:mh", longopts, NULL)) != -1) {
        oind++;
        switch (opt) {
            case 'c':
                conf.running_mode = RUN_CRON;
                break;
            case 'C':
                conf.running_mode = RUN_CHECK_NEW;
                break;
            case 'w':
                conf.running_mode = RUN_WATCH;
                conf.print_nminute = atoi(optarg);
                oind++;
                break;
            case 'i':
                conf.print_interval = atoi(optarg);
                oind++;
                break;
            case 'L':
                conf.running_mode = RUN_LIST;
                break;
            case 'l':
                conf.running_mode = RUN_PRINT_LIVE;
                break;
            case 'f':
                strncpy(conf.output_file_path, optarg, LEN_128);
                break;
            case 's':
                set_special_field(optarg);
                break;
            case 'I':
                set_special_item(optarg);
                break;
            case 'n':
                conf.print_ndays = atoi(optarg);
                oind++;
                break;
            case 'd':
                conf.print_day = atoi(optarg);
                oind++;
                break;
            case 'm':
                conf.print_merge  = MERGE_ITEM;
                break;
            case 'D':
                conf.op_type = 1;
                conf.running_mode = RUN_DEBUG;
                strncat(conf.op_arg, optarg, sizeof(conf.op_arg));
                oind++;
                break;
            case 'S':
                conf.op_type = 2;
                conf.running_mode = RUN_DEBUG;
                strncat(conf.op_arg, optarg, sizeof(conf.op_arg));
                oind++;
                break;
            case 'M':
                conf.running_mode = RUN_MSOP;
                strncat(conf.op_arg, optarg, sizeof(conf.op_arg));
                oind++;
                break;
            case 'h':
                usage();
                break;
            case ':':
                printf("must have parameter\n");
                usage();
                break;
            case '?':
                if (argv[oind] && strstr(argv[oind], "--")) {
                    strncat(conf.output_print_mod, argv[oind], LEN_512 - sizeof(DATA_SPLIT));
                    strcat(conf.output_print_mod, DATA_SPLIT);

                } else {
                    usage();
                }
        }
    }
    /* set default parameter */
    if (!conf.print_ndays) {
        conf.print_ndays = 1;
    }

    if (conf.print_interval <= 0) {
        conf.print_interval = DEFAULT_PRINT_INTERVAL;
    }

    if (RUN_NULL == conf.running_mode) {
        conf.running_mode = RUN_PRINT;

    } else if (conf.running_mode == RUN_CHECK_NEW) {
        conf.print_interval = 60;
        conf.print_tail = 0;
    }

    if (!strlen(conf.output_print_mod)) {
        conf.print_mode = DATA_SUMMARY;

    } else {
        conf.print_mode = DATA_DETAIL;
    }

    strcpy(conf.config_file, DEFAULT_CONF_FILE_PATH);
    if (access(conf.config_file, F_OK)) {
        do_debug(LOG_FATAL, "main_init: can't find tsar.conf");
    }

    if (conf.running_mode == RUN_MSOP) {
        conf.op_restart = 1;
        if (strstr(conf.op_arg, "stop")) {
            conf.op_cmd[0] = '\0';
        } else {
            if (!strstr(conf.op_arg, "run") && !strstr(conf.op_arg, "log")) {
                usage();
            }
            snprintf(conf.op_cmd, sizeof(conf.op_cmd), "msop");
            if (strstr(conf.op_arg, "log")) {
                snprintf(conf.op_cmd + strlen(conf.op_cmd), sizeof(conf.op_cmd) - strlen(conf.op_cmd), "%s", " --log");
                conf.op_restart = 0;
            }

            //level
            snprintf(conf.op_cmd + strlen(conf.op_cmd), sizeof(conf.op_cmd) - strlen(conf.op_cmd), " %s", "&");
        }
    }
}

void
shut_down()
{
    free_modules();

/*
    memset(&conf, 0, sizeof(struct configure));
    memset(mods, 0, sizeof(mods));
    memset(&statis, 0, sizeof(struct statistic));
*/
}


void
running_list()
{
    int    i;
    struct module *mod;

    printf("tsar enable follow modules:\n");

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = mods[i];
        printf("    %s\n", mod->name + 4);
    }
}


void
running_cron()
{
    int    have_collect = 0;
    /* output interface */
    if (strstr(conf.output_interface, "file")) {
        /* output data */
        collect_record();
        output_file();
        have_collect = 1;
    }

    if (strstr(conf.output_interface, "db")) {
        output_db(have_collect);
    }
    if (strstr(conf.output_interface, "nagios")) {
        output_nagios();
    }
    if (strstr(conf.output_interface, "tcp")) {
        output_multi_tcp(have_collect);
    }
}

static void get_runing_mode_str(int mode, char *str, int len)
{
    switch(mode) {
        case RUN_NULL:
            snprintf(str, len, "%s(%d)", "RUN_NULL", mode);
            break;
        case RUN_LIST:
            snprintf(str, len, "%s(%d)", "RUN_LIST", mode);
            break;
        case RUN_CRON:
            snprintf(str, len, "%s(%d)", "RUN_CRON", mode);
            break;
#ifdef OLDTSAR
        case RUN_CHECK:
            snprintf(str, len, "%s(%d)", "RUN_CHECK", mode);
            break;
#endif
        case RUN_CHECK_NEW:
            snprintf(str, len, "%s(%d)", "RUN_CHECK_NEW", mode);
            break;
        case RUN_PRINT:
            snprintf(str, len, "%s(%d)", "RUN_PRINT", mode);
            break;
        case RUN_PRINT_LIVE:
            snprintf(str, len, "%s(%d)", "RUN_PRINT_LIVE", mode);
            break;
        case RUN_WATCH:
            snprintf(str, len, "%s(%d)", "RUN_WATCH", mode);
            break;
        default:
            snprintf(str, len, "%s(%d)", "NONE", mode);
            break;
    }
}


int
main(int argc, char **argv)
{
    char tmp[64] = {0};
    char old[64] = {0};
    char now[64] = {0};
    char cmd[256] = {0};
    
    parse_config_file(DEFAULT_CONF_FILE_PATH);

    load_modules();

    statis.cur_time = time(NULL);

    conf.print_day = -1;

    main_init(argc, argv);

    /*
     * enter running
     */
    get_runing_mode_str(conf.running_mode, tmp, sizeof(tmp));
    //do_debug(LOG_DEBUG, "tsar run %s start.\n", tmp);
    switch (conf.running_mode) {
        case RUN_LIST:
            running_list();
            break;

        case RUN_CRON:
            conf.print_mode = DATA_DETAIL;
            running_cron();
            break;
#ifdef OLDTSAR
            /*for check option*/
        case RUN_CHECK:
            reload_check_modules();
            /* disable module when n_col is zero */
            running_check(RUN_CHECK);
            break;
            /*end*/
#endif
        case RUN_CHECK_NEW:
            if (reload_modules(conf.output_print_mod)) {
                conf.print_mode = DATA_DETAIL;
            };
            /* disable module when n_col is zero */
            disable_col_zero();
            running_check(RUN_CHECK_NEW);
            break;
        case RUN_PRINT:
            /* reload module by output_stdio_mod and output_print_mod*/
            reload_modules(conf.output_stdio_mod);
            reload_modules(conf.output_print_mod);
            if (!statis.real_mod_num) {
                do_debug(LOG_ERR, "enable mod count is 0.\n");
                break;
            }
            /* disable module when n_col is zero */
            disable_col_zero();

            /* set conf.print_nline_interval */
            conf.print_nline_interval = conf.print_interval;
            running_print();
            break;

        case RUN_PRINT_LIVE:
            /* reload module by output_stdio_mod and output_print_mod*/
            reload_modules(conf.output_stdio_mod);
            reload_modules(conf.output_print_mod);
            if (!statis.real_mod_num) {
                do_debug(LOG_ERR, "enable mod count is 0.\n");
                break;
            }
            /* disable module when n_col is zero */
            disable_col_zero();

            running_print_live();
            break;
        case RUN_WATCH:
            /* reload module by output_stdio_mod and output_print_mod*/
            reload_modules(conf.output_stdio_mod);
            reload_modules(conf.output_print_mod);
            if (!statis.real_mod_num) {
                do_debug(LOG_ERR, "enable mod count is 0.\n");
                break;
            }
            /* disable module when n_col is zero */
            disable_col_zero();

            /* set conf.print_nline_interval */
            conf.print_nline_interval = conf.print_interval;
            running_print();
            break;

         case RUN_MSOP:
            if (conf.op_restart) {
                system("killall -9 msop");
            }
            if (strlen(conf.op_cmd)) {
                system(conf.op_cmd);
            }
            break;

         case RUN_DEBUG:
            if (conf.op_type == 1) {
                if (strcasecmp(conf.op_arg, "INFO")
                    && strcasecmp(conf.op_arg, "DEBUG")
                    && strcasecmp(conf.op_arg, "WARN")
                    && strcasecmp(conf.op_arg, "ERROR")
                    && strcasecmp(conf.op_arg, "FATAL")) {
                    fprintf(stderr,
                    "invalid input level\n"
                    "Usage: tsar --debug [level]\n"
                    "example: tsar --debug INFO\n"
                    "         tsar --debug DEBUG\n"
                    "         tsar --debug WARN\n"
                    "         tsar --debug ERROR\n"
                    "         tsar --debug FATAL\n");
                } else {
                    snprintf(old, sizeof(old), "debug_level %s", conf.debugLevelStr);
                    snprintf(now, sizeof(now), "debug_level %s", conf.op_arg);
                    snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' %s", old, now, DEFAULT_CONF_FILE_PATH);
                    system(cmd);
                    printf("change debug_level from %s to %s\n",conf.debugLevelStr, conf.op_arg);
                }
            } else if (conf.op_type == 2) {
                if (strcasecmp(conf.op_arg, "on")
                    && strcasecmp(conf.op_arg, "off")) {
                    fprintf(stderr,
                    "invalid stdout type\n"
                    "Usage: tsar --stdout [opt]\n"
                    "example: tsar --stdout on\n"
                    "         tsar --stdout off\n");
                } else {
                    snprintf(old, sizeof(old), "tsar_log_stdout %s", conf.op_stdout);
                    snprintf(now, sizeof(now), "tsar_log_stdout %s", conf.op_arg);
                    snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' %s", old, now, DEFAULT_CONF_FILE_PATH);
                    system(cmd);
                    printf("change tsar_log_stdout from %s to %s\n",conf.op_stdout, conf.op_arg);
                }
            }
            break;

        default:
            break;
    }

    shut_down();
    //do_debug(LOG_DEBUG, "tsar run %s end.\n", tmp);
    return 0;
}
