#include <common.h>
#include <linux/io.h>
#include <hi3536a_gpio.h>

/* GPIO control registers */
static GPIO_REG_T  *hi_gio_reg[HI_GPIO_GROUP_NUM];
static VU32 hi_gio_offset[HI_GPIO_GROUP_NUM];

extern int mstype_read(uchar off, uchar cnt, char *output);

int gpio_init(void)
{
    int i;
    VU32 *setPadCtrlReg42;
    for (i = 0; i < HI_GPIO_GROUP_NUM; i++) {
        hi_gio_offset[i] = HI_GPIO_OFFSET * i;
    }
    // ioremap
    for (i = 0; i < HI_GPIO_GROUP_NUM; i++) {
        hi_gio_reg[i] = (GPIO_REG_T *)(HI_GPIO_BASE + hi_gio_offset[i]);
    }
    return 0;
}

int gpio_deinit(void)
{
    int i;
    for (i = HI_GPIO_GROUP_NUM - 1; i >= 0; i--) {
        iounmap((void *)hi_gio_reg[i]);
    }
    return 0;
}

int gpio_mode(int gpio, GIO_MODE_E mode)
{
    VU32 group = MS_GROUP(gpio);
    VU32 bit = MS_BIT(gpio);

    if (group >= HI_GPIO_GROUP_NUM) {
        __E("Group out of range! \n");
        return -1;
    }
    if (bit >= HI_GPIO_BIT_NUM) {
        __E("Bit out of range! \n");
        return -1;
    }
    // Disable IRQ
    hi_gio_reg[group]->MIS &= ~(1 << bit);

    if (mode == GIO_MODE_OUTPUT) {
        hi_gio_reg[group]->DIR |= (GIO_MODE_OUTPUT << bit);
    } else {
        hi_gio_reg[group]->DIR &= ~(GIO_MODE_OUTPUT << bit);
    }

    return 0;
}

int gpio_read(int gpio)
{
    VU32 group = MS_GROUP(gpio);
    VU32 bit = MS_BIT(gpio);
    VU32 addr = MS_PADDR(gpio);

    if (group >= HI_GPIO_GROUP_NUM) {
        __E("Group out of range! \n");
        return -1;
    }
    if (bit >= HI_GPIO_BIT_NUM) {
        __E("Bit out of range! \n");
        return -1;
    }

    // Disable IRQ
    hi_gio_reg[group]->MIS &= ~(1 << bit);
    // Set directory input
    hi_gio_reg[group]->DIR &= ~(GIO_MODE_OUTPUT << bit);
    // Set data
    if (hi_gio_reg[group]->DATA[addr] & (1 << bit)) {
        return 1;
    }

    return 0;
}


int gpio_write(int gpio, int lv)
{
    VU32 group = MS_GROUP(gpio);
    VU32 bit = MS_BIT(gpio);
    VU32 addr = MS_PADDR(gpio);

    __D("IO:(%d-%d), paddr:%d value:%d", group, bit, addr, lv);
    if (group >= HI_GPIO_GROUP_NUM) {
        __E("Group out of range! \n");
        return -1;
    }
    if (bit >= HI_GPIO_BIT_NUM) {
        __E("Bit out of range! \n");
        return -1;
    }

    // Disable IRQ
    hi_gio_reg[group]->MIS &= ~(1 << bit);
    // Set directory output
    hi_gio_reg[group]->DIR |= (GIO_MODE_OUTPUT << bit);
    // Set data
    hi_gio_reg[group]->DATA[addr] = (lv << bit);

    return 0;
}

void m_delay(int n)
{
    while (n --) {
        udelay(1000);
    }

}

typedef struct part_info_s {
    char         name[32];
    unsigned int size;
    unsigned int version;
    unsigned int date;

} part_info_t;

int tolower(int c)
{
    if ((c >= 'A') && (c <= 'Z')) {
        return c + ('a' - 'A');
    }
    return c;
}

int htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= 'A' && s[i] <= 'F'); ++i) {
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}

void buzz(int timeout)
{
    gpio_write(BUZZ_CTL, BUZZ_ON);
    while (timeout--) {
        m_delay(100);
    }
    gpio_write(BUZZ_CTL, BUZZ_OFF);
}

void gpio_setting(void)
{
    buzz(2);
    gpio_write(STAT_GREEN_LED, LED_ON);
}

void restore_setting(void)
{
    run_command("mmc erase.part cfg", 0);
    printf("restore factory done ... \n");
    buzz(10);
}

int switch_setting(void)
{
    char *bootargs;
    char *bootcmd;
    char *p;

    bootargs = env_get("bootargs");
    if (!bootargs) {
        printf("Get bootargs fail!\n");
        return 0;
    }

    bootcmd = env_get("bootcmd");
    if (!bootcmd) {
        printf("Get bootcmd fail!\n");
        return 0;
    }

    if ((p = strstr(bootargs, "mmcblk0p6")) != NULL) {
        *(p + 8) = '7';
    } else if ((p = strstr(bootargs, "mmcblk0p7")) != NULL) {
        *(p + 8) = '6';
    } else {
        printf("bootargs is error\n");
        return 0;
    }


    if ((p = strstr(bootcmd, "ker1")) != NULL) {
        *(p + 3) = '2';
    } else if ((p = strstr(bootcmd, "ker2")) != NULL) {
        *(p + 3) = '1';
    } else {
        printf("bootcmd is error\n");
        return 0;
    }
    run_command("saveenv", 0);
    printf("switch fs_ker done ... \n");
    buzz(20);
    return 0;
}

int factory_reset(void)
{
    unsigned char i = 0;
    if (gpio_read(RESET_KEY) != RESET_KEY_PRESSED) {
        return 0;
    } else {
        printf("The reset key is pressed !\n");
        for (i = 0; i < 30; i ++) {
            if (gpio_read(RESET_KEY) == RESET_KEY_PRESSED) {
                if (i == 7) {
                    restore_setting();
                } else if (i == 26) {
                    switch_setting();
                    break;
                } else {
                    m_delay(500);
                    continue;
                }
            } else {
                break;
            }
        }
    }
    return 0;
}

