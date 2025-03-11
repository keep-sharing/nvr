#include "tsar.h"

/*
 * Structure for CPU infomation.
 */
struct stats_cpu {
    unsigned long long cpu_user;
    unsigned long long cpu_nice;
    unsigned long long cpu_sys;
    unsigned long long cpu_idle;
    unsigned long long cpu_iowait;
    unsigned long long cpu_steal;
    unsigned long long cpu_hardirq;
    unsigned long long cpu_softirq;
    unsigned long long cpu_guest;
    unsigned long long cpu_number;
};

#define STATS_CPU_SIZE (sizeof(struct stats_cpu))

static char *cpu_usage = "    --cpu               CPU share (user, system, interrupt, nice, & idle)";
static int   cpu_quota = 1;

static void compute_cpu_data(double st_array[], struct stats_cpu *pre, struct stats_cpu *cur)
{
    U_64 pre_array[10];
    U_64 cur_array[10];
    char *max_cpu;
    U_64 pre_total, cur_total;
    int i, j;
    
    pre_array[0] = pre->cpu_user;
    pre_array[1] = pre->cpu_sys;
    pre_array[2] = pre->cpu_iowait;
    pre_array[3] = pre->cpu_hardirq;
    pre_array[4] = pre->cpu_softirq;
    pre_array[5] = pre->cpu_idle;
    pre_array[6] = pre->cpu_nice;
    pre_array[7] = pre->cpu_steal;
    pre_array[8] = pre->cpu_guest;
    pre_array[9] = pre->cpu_number;

    cur_array[0] = cur->cpu_user;
    cur_array[1] = cur->cpu_sys;
    cur_array[2] = cur->cpu_iowait;
    cur_array[3] = cur->cpu_hardirq;
    cur_array[4] = cur->cpu_softirq;
    cur_array[5] = cur->cpu_idle;
    cur_array[6] = cur->cpu_nice;
    cur_array[7] = cur->cpu_steal;
    cur_array[8] = cur->cpu_guest;
    cur_array[9] = cur->cpu_number;

    //get cpu number for cpushare
    max_cpu = getenv("SIGMA_MAX_CPU_QUOTA");
    cpu_quota = max_cpu ? atoi(max_cpu) / 100 : 1;
    pre_total = cur_total = 0;

    for (i = 0; i < 9; i++) {
        if(cur_array[i] < pre_array[i]){
            for(j = 0; j < 9; j++)
                st_array[j] = -1;
            return;
        }
        pre_total += pre_array[i];
        cur_total += cur_array[i];
    }

    /* no tick changes, or tick overflows */
    if (cur_total <= pre_total) {
        for(j = 0; j < 9; j++)
            st_array[j] = -1;
        return;
    }

    /* set st record */
    for (i = 0; i < 9; i++) {
        /* st_array[5] is util, calculate it late */
        if((i != 5) && (cur_array[i] >= pre_array[i]))
            st_array[i] = (cur_array[i] - pre_array[i]) * 100.0 / (cur_total - pre_total);
    }

    /* util = 100 - idle - iowait - steal */
    if (cur_array[5] >= pre_array[5]) {
        st_array[5] = 100.0 - (cur_array[5] - pre_array[5]) * 100.0 / (cur_total - pre_total) - st_array[2] - st_array[7];
    }
    if (cpu_quota > 1) {
        for (i = 0; i < 9; i++) {
            st_array[i] = st_array[i] * cur_array[9] / cpu_quota;
        }
    }

    st_array[9] = cur_array[9];
    return;
}

static void
read_cpu_stats(struct module *mod)
{
    FILE   *fp, *ncpufp;
    char   *max_cpu;
    char    line[LEN_4096];
    char    buf[LEN_4096];
    struct  stats_cpu st_cpu[2];
    int i;
    double st_array[10];

    memset(buf, 0, LEN_4096);
    memset(&st_cpu[0], 0, sizeof(struct stats_cpu) * 2);
    //get cpu number for cpushare
    max_cpu = getenv("SIGMA_MAX_CPU_QUOTA");
    cpu_quota = max_cpu ? atoi(max_cpu) / 100 : 1;

    //read twice and set value

    for (i = 0; i < 2; i++) {
        if ((fp = fopen(STAT, "r")) == NULL) {
            return;
        }
        while (fgets(line, LEN_4096, fp) != NULL) {
            if (!strncmp(line, "cpu ", 4)) {
                /*
                 * Read the number of jiffies spent in the different modes
                 * (user, nice, etc.) among all proc. CPU usage is not reduced
                 * to one processor to avoid rounding problems.
                 */
                sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &st_cpu[i].cpu_user,
                        &st_cpu[i].cpu_nice,
                        &st_cpu[i].cpu_sys,
                        &st_cpu[i].cpu_idle,
                        &st_cpu[i].cpu_iowait,
                        &st_cpu[i].cpu_hardirq,
                        &st_cpu[i].cpu_softirq,
                        &st_cpu[i].cpu_steal,
                        &st_cpu[i].cpu_guest);
            }
        }

        /* get cpu number */
        if ((ncpufp = fopen("/proc/cpuinfo", "r")) == NULL) {
            fclose(fp);
            return;
        }
        while (fgets(line, LEN_4096, ncpufp)) {
            if (!strncmp(line, "processor\t:", 11))
                st_cpu[i].cpu_number++;
        }
        fclose(ncpufp);
        fclose(fp);
        if (!i) {
            sleep(1);
        }
    }

    compute_cpu_data(st_array, &st_cpu[0], &st_cpu[1]);
    sprintf(buf, "%.2f,%.2f,%.2f,%.2f,%.2f",
            /* the store order is not same as read procedure */
            st_array[0],
            st_array[1],
            st_array[2],
            st_array[4],
            st_array[5]);
    set_mod_record(mod, buf);
}

static void
read_cpu_stats_bak(struct module *mod)
{
    FILE   *fp, *ncpufp;
    char   *max_cpu;
    char    line[LEN_4096];
    char    buf[LEN_4096];
    struct  stats_cpu st_cpu;

    memset(buf, 0, LEN_4096);
    memset(&st_cpu, 0, sizeof(struct stats_cpu));
    //get cpu number for cpushare
    max_cpu = getenv("SIGMA_MAX_CPU_QUOTA");
    cpu_quota = max_cpu ? atoi(max_cpu) / 100 : 1;
    //unsigned long long cpu_util;
    if ((fp = fopen(STAT, "r")) == NULL) {
        return;
    }
    while (fgets(line, LEN_4096, fp) != NULL) {
        if (!strncmp(line, "cpu ", 4)) {
            /*
             * Read the number of jiffies spent in the different modes
             * (user, nice, etc.) among all proc. CPU usage is not reduced
             * to one processor to avoid rounding problems.
             */
            sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &st_cpu.cpu_user,
                    &st_cpu.cpu_nice,
                    &st_cpu.cpu_sys,
                    &st_cpu.cpu_idle,
                    &st_cpu.cpu_iowait,
                    &st_cpu.cpu_hardirq,
                    &st_cpu.cpu_softirq,
                    &st_cpu.cpu_steal,
                    &st_cpu.cpu_guest);
        }
    }

    /* get cpu number */
    if ((ncpufp = fopen("/proc/cpuinfo", "r")) == NULL) {
        fclose(fp);
        return;
    }
    while (fgets(line, LEN_4096, ncpufp)) {
        if (!strncmp(line, "processor\t:", 11))
            st_cpu.cpu_number++;
    }
    fclose(ncpufp);

    /* cpu_util =  */
    /*      st_cpu.cpu_user + st_cpu.cpu_sys + */
    /*      st_cpu.cpu_hardirq + st_cpu.cpu_softirq; */

    sprintf(buf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",
            /* the store order is not same as read procedure */
            st_cpu.cpu_user,
            st_cpu.cpu_sys,
            st_cpu.cpu_iowait,
            st_cpu.cpu_hardirq,
            st_cpu.cpu_softirq,
            st_cpu.cpu_idle,
            st_cpu.cpu_nice,
            st_cpu.cpu_steal,
            st_cpu.cpu_guest,
            st_cpu.cpu_number);

    set_mod_record(mod, buf);
    if (fclose(fp) < 0) {
        return;
    }
}

static void
set_cpu_record(struct module *mod, double st_array[],
    U_64 pre_array[], U_64 cur_array[], int inter)
{
    int    i, j;
    char  *max_cpu;
    U_64   pre_total, cur_total;

    return;

    //get cpu number for cpushare
    max_cpu = getenv("SIGMA_MAX_CPU_QUOTA");
    cpu_quota = max_cpu ? atoi(max_cpu) / 100 : 1;

    pre_total = cur_total = 0;

    for (i = 0; i < 9; i++) {
        if(cur_array[i] < pre_array[i]){
            for(j = 0; j < 9; j++)
                st_array[j] = -1;
            return;
        }
        pre_total += pre_array[i];
        cur_total += cur_array[i];
    }

    /* no tick changes, or tick overflows */
    if (cur_total <= pre_total) {
        for(j = 0; j < 9; j++)
            st_array[j] = -1;
        return;
    }

    /* set st record */
    for (i = 0; i < 9; i++) {
        /* st_array[5] is util, calculate it late */
        if((i != 5) && (cur_array[i] >= pre_array[i]))
            st_array[i] = (cur_array[i] - pre_array[i]) * 100.0 / (cur_total - pre_total);
    }

    /* util = 100 - idle - iowait - steal */
    if (cur_array[5] >= pre_array[5]) {
        st_array[5] = 100.0 - (cur_array[5] - pre_array[5]) * 100.0 / (cur_total - pre_total) - st_array[2] - st_array[7];
    }
    if (cpu_quota > 1) {
        for (i = 0; i < 9; i++) {
            st_array[i] = st_array[i] * cur_array[9] / cpu_quota;
        }
    }

    st_array[9] = cur_array[9];
    /* util = user + sys + hirq + sirq + nice */
    //st_array[5] = st_array[0] + st_array[1] + st_array[3] + st_array[4] + st_array[6];
}

static struct mod_info cpu_info[] = {
    {"  user", SUMMARY_BIT,  0,  STATS_NULL},
    {"   sys", DETAIL_BIT,  0,  STATS_NULL},
    {"  wait", DETAIL_BIT,  0,  STATS_NULL},
    //{"  hirq", DETAIL_BIT,  0,  STATS_NULL},
    {"  sirq", DETAIL_BIT,  0,  STATS_NULL},
    {"  util", SUMMARY_BIT,  0,  STATS_NULL},
    //{"  nice", HIDE_BIT,  0,  STATS_NULL},
    //{" steal", HIDE_BIT,  0,  STATS_NULL},
    //{" guest", HIDE_BIT,  0,  STATS_NULL},
    //{"  ncpu", HIDE_BIT,  0,  STATS_NULL},
};

void
mod_cpu_register(struct module *mod)
{
    register_mod_fields(mod, "--cpu", cpu_usage, cpu_info, 10, read_cpu_stats, set_cpu_record);
}
