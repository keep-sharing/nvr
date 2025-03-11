#include "gd32f30x.h"
#include "ms_common.h"

volatile UART_CMD_E rcv_cmd = CMD_NONE;
static uint8_t led_state = 0;
keyInfo_t keyInfo = {0};
unsigned int alarmInstatus;

ms_gpio_t alarmOut[ALARM_OUT_NUM] = {
    {ALARM_OUT1_PORT, ALARM_OUT1_PIN},
    {ALARM_OUT2_PORT, ALARM_OUT2_PIN},
    {ALARM_OUT3_PORT, ALARM_OUT3_PIN},
    {ALARM_OUT4_PORT, ALARM_OUT4_PIN}
};

ms_gpio_t alarmIn[ALARM_IN_NUM] = {
    {ALARM_IN1_PORT, ALARM_IN1_PIN},
    {ALARM_IN2_PORT, ALARM_IN2_PIN},
    {ALARM_IN3_PORT, ALARM_IN3_PIN},
    {ALARM_IN4_PORT, ALARM_IN4_PIN},
    {ALARM_IN5_PORT, ALARM_IN5_PIN},
    {ALARM_IN6_PORT, ALARM_IN6_PIN},
    {ALARM_IN7_PORT, ALARM_IN7_PIN},
    {ALARM_IN8_PORT, ALARM_IN8_PIN},
    {ALARM_IN9_PORT, ALARM_IN9_PIN},
    {ALARM_IN10_PORT, ALARM_IN10_PIN},
    {ALARM_IN11_PORT, ALARM_IN11_PIN},
    {ALARM_IN12_PORT, ALARM_IN12_PIN},
    {ALARM_IN13_PORT, ALARM_IN13_PIN},
    {ALARM_IN14_PORT, ALARM_IN14_PIN},
    {ALARM_IN15_PORT, ALARM_IN15_PIN},
    {ALARM_IN16_PORT, ALARM_IN16_PIN}
};

void ms_led_off(void)
{
    gpio_bit_set(LED_PORT, LED_PIN);
    led_state = 0;
}

void ms_led_on(void)
{
    gpio_bit_reset(LED_PORT, LED_PIN);
    led_state = 1;
}

void ms_led_toggle(void)
{
    if (led_state)
        ms_led_off();
    else
        ms_led_on();
}

unsigned char ms_power_key_state(KEY_STATE state)
{
    if (keyInfo.key_state == state) 
    {
        keyInfo.key_state = KEY_NONE;
        return 1;
    }
    return 0;
}

void ms_power_key_clear(void)
{
    keyInfo.key_state = KEY_NONE;
}

void ms_power_on(void)
{
    gpio_bit_set(POWER_EN_PORT, POWER_EN_PIN);
    ms_led_on();
}

void ms_power_off(void)
{
    gpio_bit_reset(POWER_EN_PORT, POWER_EN_PIN);
    ms_led_off();
    keyInfo.key_state = KEY_NONE;
    keyInfo.shutdown_ready = 0;
}

void ms_shutdown_ready(void)
{
    if (keyInfo.shutdown_ready)
        return;
    keyInfo.shutdown_ready = 1;
    keyInfo.shutdown_time_ms = 0;
}

unsigned char ms_shutdown_is_timeout(void)
{    
    if (keyInfo.shutdown_ready == 0)
        return 0;
    return (keyInfo.shutdown_time_ms >= SHUTDOWN_TIME_MAX);
}

unsigned char alarm_in_trigger(void)
{
    unsigned int i = 0;
    for (i = 0; i < ALARM_IN_NUM; i++) {
        if (gpio_input_bit_get(alarmIn[i].port, alarmIn[i].pin) == 0) {
            return 1;
        }
    }
    return 0;
}

void alarm_in_control(void)
{
    unsigned int tmp = 0, i = 0;
    unsigned char data[] = {0xaf, 0xcc, 0x05, 0xfa,
                            0x01, 0x0d, 0x00, 0x00, 0x00};
    for (i = 0; i < ALARM_IN_NUM; i++) {
        tmp |= (gpio_input_bit_get(alarmIn[i].port, alarmIn[i].pin) << i);
    }
    if (alarmInstatus == tmp) {
        return;
    }
    alarmInstatus = tmp;
    data[6] = tmp & 0xff;
    data[7] = (tmp >> 8) & 0xff;
    uart0_sendstring(data, sizeof(data) / sizeof(data[0]));
}

void ret_cmd_b(void)
{
    unsigned char data[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x03, 0x00};
    uart0_sendstring(data, sizeof(data) / sizeof(data[0]));
}

void ret_cmd_c(void)
{
    unsigned char data[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x05, 0x00};
    uart0_sendstring(data, sizeof(data) / sizeof(data[0]));
}

void alarm_in_test(void)
{
    unsigned int tmp = 0, i = 0;
    unsigned char data[] = {0xaf, 0xcc, 0x05, 0xfa,
                            0x01, 0x0e, 0x00, 0x00, 0x00};
    for (i = 0; i < ALARM_IN_NUM; i++) {
        tmp |= (gpio_input_bit_get(alarmIn[i].port, alarmIn[i].pin) << i);
    }
    data[6] = tmp & 0xff;
    data[7] = (tmp >> 8) & 0xff;
    uart0_sendstring(data, sizeof(data) / sizeof(data[0]));
}