#ifndef __MS_COMMAND_HANDLER_H
#define __MS_COMMAND_HANDLER_H

#define USART_BUFFER_SIZE 128

typedef struct {    /* 接收队列 */
    unsigned char front;   /* 队头 */
    unsigned char rear;    /* 队尾 */
    unsigned char RawDataBuff[USART_BUFFER_SIZE]; /* 接收缓冲区 */
} __Rcv_Queue;


typedef struct {
    unsigned char id;
    void (*handler)(unsigned short value);
} MS_CMD_TABLE;

typedef enum {
    MS_HEAD = 0,
    MS_VALUE,
    MS_CHECKSUM,
    MS_CMD_NONE
} MS_FREAM_FIELD;

unsigned char ms_data_out_queue(__Rcv_Queue *Queue);
unsigned char ms_data_enter_queue(__Rcv_Queue *Queue, unsigned char x);
void ms_receive_data_handler(__Rcv_Queue *queue);

#endif
