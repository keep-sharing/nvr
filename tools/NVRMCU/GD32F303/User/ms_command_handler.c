#include "gd32f30x.h"
#include "ms_command_handler.h"
#include "sys_config.h"
#include "ms_common.h"

extern __Rcv_Queue ms_uart0;

unsigned char ms_data_enter_queue(__Rcv_Queue *queue, unsigned char x)
{
    if (queue->front == (queue->rear + 1) % USART_BUFFER_SIZE) {
        return 0;  /* 队满 */
    } else {
        queue->RawDataBuff[queue->rear] = x;
        queue->rear = (queue->rear + 1) % USART_BUFFER_SIZE;
        return 1;
    }
}

unsigned char ms_data_out_queue(__Rcv_Queue *queue)
{
    unsigned char tmp;

    if (queue->front == queue->rear) {
        return 0;  /* 队空 */
    } else {
        tmp = queue->RawDataBuff[queue->front];
        queue->front = (queue->front + 1) % USART_BUFFER_SIZE;
        return tmp;
    }
}

unsigned char is_queue_empty(__Rcv_Queue queue)
{
    return (queue.front == queue.rear ? 1 : 0);
}

extern MS_FREAM_FIELD parse_status;
static void ms_cmd_handler(unsigned char cmd, unsigned char *data)
{
    switch (cmd) {
    case CMD_A:
        break;
    case CMD_B:
        break;
    case CMD_C:
        break;
    case CMD_D:
        break;
    case CMD_E:
        break;
    case CMD_F:
        break;
    case CMD_ALARM_OUT:
        gpio_bit_write(alarmOut[data[5]].port, alarmOut[data[5]].pin, (bit_status)(data[4]));
        break;
    case CMD_ALARM_IN:
        break;
    case CMD_TEST_ALARM_IN:
        alarm_in_test();
        break;
    default:
        break;
    }
}

void ms_receive_data_handler(__Rcv_Queue *queue)
{
    static unsigned char idx = 0;
    static unsigned char tmpRawData, rawData, length;
    static unsigned char data[32];
    static unsigned int checkSum;
    while (!is_queue_empty(*queue)) {
        rawData = ms_data_out_queue(queue);
        if (queue == &ms_uart0) {
            switch (parse_status) {
                case MS_HEAD:
                    if ((length + rawData) == 0xFF) {
                        data[0] = length;
                        data[1] = rawData;
                        checkSum = rawData + length;
                        parse_status = MS_VALUE;
                    } else {
                        length = rawData;
                    }
                    break;
                case MS_VALUE:
                    data[2 + idx] = rawData;
                    checkSum += rawData;
                    ++idx;
                    if (idx == length - 1) {
                        parse_status = MS_CHECKSUM;
                    }
                    break;
                case MS_CHECKSUM:
                    if ((checkSum & 0xFF) == rawData) {
                        rcv_cmd = data[3];
                        ms_cmd_handler(data[3], data);
                    }
                    idx = 0;
                    parse_status = MS_CMD_NONE;
                    break;
                default:
                    break;
            }

            if (((tmpRawData << 8) | rawData) == 0xAFCC) {
                parse_status = MS_HEAD;
                idx = 0;
            }
            tmpRawData = rawData;
        }
    }
}

