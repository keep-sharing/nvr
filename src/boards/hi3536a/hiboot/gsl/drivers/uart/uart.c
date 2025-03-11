/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <platform.h>
#include <types.h>
#include <common.h>
#include <lib.h>
#include "securecutil.h"
#include "uart.h"

#define IN
#define OUT

#define XSTART  0xFE 		/* START frame head byte */
#define XFILE   0xFE		/* FILE frame head byte */
#define XDATA   0xDA		/* DATA frame head byte */
#define XEOT    0xED		/* EOT frame head byte */

#define ACK     0xAA		/* UE response byte ACK */
#define NAK     0x55		/* UE response byte NAK */

#define LOAD_RAM    0x01	/* file transfer type: RAM init file */
#define LOAD_USB    0x02	/* fiel transfer type: USB load file */

#define XUSER_DATA_LENGTH   1024	/* Data length of data frame */

#define FRAME_HEAD          0		/* The head of frame */
#define FRAME_LENGTH_FILE   13		/* The length of file frame(start from 0) */
#define FRAME_LENGTH_DATA   (XUSER_DATA_LENGTH + 4)	/* The length of standard data frame(start from 0) */
#define FRAME_LENGTH_EOT    4		/* The length of standard EOT frame(start from 0) */

#define PROC_BYTE_HEAD          0	/* The offset of frame head byte */
#define PROC_BYTE_SEQ           1	/* The offset of seq byte */
#define PROC_BYTE_SEQF          2	/* The offset of inverse seq byte */
#define PROC_BYTE_FILE_TYPE     3	/* The offset of file type byte */
#define PROC_BYTE_FILE_LENG0    4	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG1    5	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG2    6	/* The offset of file length byte */
#define PROC_BYTE_FILE_LENG3    7	/* The offset of file length byte */
#define PROC_BYTE_FILE_ADDR0    8	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR1    9	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR2    10	/* The offset of file addr byte */
#define PROC_BYTE_FILE_ADDR3    11	/* The offset of file addr byte */

#define OCR_STATUS_OK       0x01	/* protocol receive status: no error */
#define OCR_STATUS_ERROR    0x02	/* protocol receive status: frame error */
#define OCR_STATUS_SKIP     0x03	/* protocol receive status: data error, need to clear */
#define OCR_STATUS_FIN      0x04	/* The file has been download completely */

#define CRC16_LEN		2
#define BIT_CNT_4	4
#define BIT_CNT_8	8

#ifndef CFG_EDA_VERIFY
#define PROC_TIME_OUT       10  /* timeout threshold  ms */
#else
#define PROC_TIME_OUT       1   /* for eda */
#endif

typedef struct {
	uint32_t  file_type;		/* File type(RAM/USB) */
	uint32_t  file_address;	/* The write addr for downloaded file */
	uint32_t  file_length;	/* The length for downloaded file */
	uint32_t  total_frame;	/* The total frame number for the file */
	uint32_t  count_frame;	/* Current frame number been transfered */
	uint8_t   exp_frame_idx;	/* The next frame index to expect */

	void (*SubFunction)(void);	/* The function pointer for the downloaded file */
} data_handle_t;

unsigned int uart_inited;

typedef struct {
	uint16_t frame_crc;
	uint16_t calc_crc;
	uint16_t len;
	uint8_t  seq;
	uint8_t *write_addr;
} frame_info;

static uint16_t cal_crc_perbyte(IN uint8_t byte, IN uint16_t crc)
{
	uint8_t  da;
	const uint16_t crc_ta[16] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
				     0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
				    };

	da = ((uint8_t)(crc >> BIT_CNT_8)) >> BIT_CNT_4;
	crc <<= BIT_CNT_4;
	crc ^= (uint16_t)crc_ta[da ^ (byte >> BIT_CNT_4)];
	da = ((uint8_t)(crc >> BIT_CNT_8)) >> BIT_CNT_4;
	crc <<= BIT_CNT_4;
	crc ^= (uint16_t)crc_ta[da ^ (byte & 0x0f)];

	return (crc);
}

static void uart_mux_pad_ctrl_set()
{
	reg_set(REG_AHB_IO_CFG_BASE + UART0_RXD_IOCFG_OFST, 0x1101);
	reg_set(REG_AHB_IO_CFG_BASE + UART0_TXD_IOCFG_OFST, 0x1011);
}

void uart_init()
{
	uart_mux_pad_ctrl_set();
	serial_init();
	uart_inited = 1;
}

void uart_deinit()
{
	serial_deinit();
	uart_inited = 0;
}

void uart_reset()
{
	/* clear the FIFO of uart controller */
	while (serial_tstc())

		(void)serial_getc();
}

static bool calc_check_crc(frame_info *frame, uint32_t pos, uint8_t cr)
{
	if (pos <= frame->len - CRC16_LEN) {
		/* calculated crc */
		frame->calc_crc = cal_crc_perbyte(cr, frame->calc_crc);
	} else {
		/* received crc */
		frame->frame_crc |= (uint16_t)(cr << (BIT_CNT_8 * (frame->len - pos)));
		if (pos == frame->len)
			/* compare calcuated crc and received crc in the last */
			return (frame->calc_crc == frame->frame_crc);
	}

	return TRUE;
}

/* flag_frame: file frame and eot frame */
#define FLAG_FRAME 2

static int parse_file_frame(data_handle_t *data_handle, frame_info *frame,
			uint32_t pos, uint8_t cr)
{
	switch (pos) {
	case PROC_BYTE_HEAD:
		frame->len = FRAME_LENGTH_FILE;
		break;

	case PROC_BYTE_SEQ:
		frame->seq = cr;
		if (frame->seq != 0)
			return -1;
		break;

	case PROC_BYTE_SEQF:
		if (cr != (uint8_t) ~frame->seq)
			return -1;
		break;

	case PROC_BYTE_FILE_TYPE:
		if ((cr == LOAD_RAM) || (cr == LOAD_USB))
			data_handle->file_type = cr;
		else
			return -1;
		break;

	case PROC_BYTE_FILE_LENG0:
		data_handle->file_length = 0; /* reset file_length */
	case PROC_BYTE_FILE_LENG1:
	case PROC_BYTE_FILE_LENG2:
	case PROC_BYTE_FILE_LENG3:
		/* obtain the length of file, Byte */
		data_handle->file_length |= cr << (BIT_CNT_8 * ((PROC_BYTE_FILE_LENG3) - pos));
		break;

	case PROC_BYTE_FILE_ADDR0:
		data_handle->file_address = 0; /* reset file_address */
	case PROC_BYTE_FILE_ADDR1:
	case PROC_BYTE_FILE_ADDR2:
	case PROC_BYTE_FILE_ADDR3:
		/* obtain the address which file should be located in, Byte */
		data_handle->file_address |= cr << (BIT_CNT_8 * ((PROC_BYTE_FILE_ADDR3) - pos));
		break;

	default:
		break;
	}

	if (!calc_check_crc(frame, pos, cr))
		return -1;

	if (pos == frame->len) {
		/* be careful when the file's length is the integer times of XUSER_DATA_LENGTH */
		data_handle->total_frame = (data_handle->file_length - 1) / XUSER_DATA_LENGTH + FLAG_FRAME;
		/* the file should be executed after download is complete. */
		data_handle->SubFunction = (void *)(uintptr_t)data_handle->file_address;
		/* the index of frame. */
		data_handle->count_frame = 1;
		/* I think, it equals to count_frame. */
		data_handle->exp_frame_idx = 1;
	}
	return 0;
}

/* data frame mata data: PROC_BYTE_HEAD + BYTE_SEQ + BYTE_SEQF + CRC16_LEN */
#define DATA_FRAME_MATA_DATA 5

static uint16_t calc_frame_len(data_handle_t *data_handle)
{
	/* the length of the last data frame is
	   (file length - number of received data frame * length of data frame) +
	   length of the package of the frame */
	/* the length of the rest is FRAME_LENGTH_DATA */
	return (data_handle->count_frame == (data_handle->total_frame - 1)) ?
	       DATA_FRAME_MATA_DATA - 1 + (data_handle->file_length - (data_handle->count_frame - 1) * XUSER_DATA_LENGTH) :
	       FRAME_LENGTH_DATA;
}

static bool parse_data_frame(data_handle_t *data_handle, frame_info *frame, uint32_t pos,
			     uint8_t cr)
{
	switch (pos) {
	case PROC_BYTE_HEAD:
		frame->len = calc_frame_len(data_handle);
		break;

	case PROC_BYTE_SEQ:
		frame->seq = cr;
		if (frame->seq != data_handle->exp_frame_idx) {
			/* trace back */
			if ((data_handle->count_frame > 1) &&
			    (frame->seq == data_handle->exp_frame_idx - 1)) {
				data_handle->count_frame--;
				data_handle->exp_frame_idx--;
				frame->len = calc_frame_len(data_handle);
			} else {
				return -1;
			}
		}

		/* obtain the address which frame should be located in */
		frame->write_addr = (uint8_t *)(uintptr_t)(data_handle->file_address +
				    (data_handle->count_frame - 1) * XUSER_DATA_LENGTH);
		break;

	case PROC_BYTE_SEQF:
		if (cr != (uint8_t) ~(frame->seq))
			return -1;
		break;

	default:
		/* locate frame */
		if (pos <= (frame->len - CRC16_LEN))
			*(frame->write_addr++) = cr;
		break;
	}

	if (!calc_check_crc(frame, pos, cr))
		return -1;

	if (pos == frame->len) {
		/* increase the index of frame */
		data_handle->count_frame++;
		data_handle->exp_frame_idx++;
	}
	return 0;
}

static bool parse_eot_frame(data_handle_t *data_handle, frame_info *frame, uint32_t pos, uint8_t cr)
{
	switch (pos) {
	case PROC_BYTE_HEAD:
		frame->len = FRAME_LENGTH_EOT;
		break;

	case PROC_BYTE_SEQ:
		frame->seq = cr;
		if (frame->seq != data_handle->exp_frame_idx)
			return -1;
		break;

	case PROC_BYTE_SEQF:
		if (cr != (uint8_t) ~frame->seq)
			return -1;
		break;

	default:
		break;
	}

	if (!calc_check_crc(frame, pos, cr))
		return -1;

	/* count_frame of EOT frame should equal to total_frame */
	if ((pos == frame->len) && (data_handle->count_frame != data_handle->total_frame))
		return -1;

	return 0;
}

static int uart_wait_data(int status)
{
	uint32_t timer_out = PROC_TIME_OUT * timer_get_divider();

	/* initialize timer controller */
	timer_start();

	/* does uart controller have any data? */
	while (!serial_tstc()) {
		/* time out. */
		if (timer_get_val() >= timer_out) {
			if (status == OCR_STATUS_ERROR)
				return OCR_STATUS_ERROR;
			else
				return OCR_STATUS_SKIP;
		}
	}

	return	OCR_STATUS_OK;
}

uint8_t uart_hand_head_frame(IN data_handle_t *data_handle, uint8_t  *frame_type,
			uint32_t pos, int *status)
{
	errno_t err;
	uint8_t  cr;

	cr = (uint8_t)serial_getc();
	if (pos != FRAME_HEAD)
		return cr;

	if (data_handle == NULL || frame_type == NULL)
		return 0;

	*frame_type = cr;

	/* check the first byte of the whole process. */
	if ((data_handle->count_frame == 0) && (*frame_type != XFILE) && (*frame_type != XSTART)) {
		if (status == NULL)
			return 0;
		*status = OCR_STATUS_SKIP;
		return cr;
	}

	if ((*frame_type != XFILE) && (*frame_type != XSTART))
		return cr;

	err = memset_s(data_handle, sizeof(data_handle_t),
			0, sizeof(data_handle_t));
	if (err != EOK)
		return 0;

	return cr;
}

static int uart_proc_loop(IN data_handle_t *data_handle)
{
	uint8_t cr;
	uint8_t frame_type = 0;
	uint32_t pos = FRAME_HEAD;

	int status = OCR_STATUS_OK;
	frame_info frame;

	if (memset_s(&frame, sizeof(frame_info), 0, sizeof(frame_info)) != EOK)
		return -1;

	while (1) {
		if (uart_wait_data(status) != OCR_STATUS_OK)
			return -1;

		cr = uart_hand_head_frame(data_handle, &frame_type, pos, &status);
		/*
		   If status equals to SKIP/ERROR, clean the whole frame by reading out
		   and discarding all datas except FILE/START frams. It shall not end untill
		   timing out
		 */
		if ((status == OCR_STATUS_SKIP) || (status == OCR_STATUS_ERROR)) {
			/*
			   When receiving FILE/START frames, every byte will be checked.
			   Thus the sender can send START frames without interval.
			   for bug:
			   see the comments in function self_boot_main.
			 */
			if (data_handle->count_frame == 0) {
				status = OCR_STATUS_OK;
				pos = 0;
			} else  {
				pos++;
			}
			continue;
		}

		switch (frame_type) {
		/* FILE frame, or START frame */
		case XFILE:
			if (parse_file_frame(data_handle, &frame, pos, cr))
				status = OCR_STATUS_ERROR;
			break;
		case XDATA:
			if (parse_data_frame(data_handle, &frame, pos, cr))
				status = OCR_STATUS_ERROR;
			break;
		case XEOT:
			if (parse_eot_frame(data_handle, &frame, pos, cr))
				status = OCR_STATUS_ERROR;
			else if (pos == frame.len)
				status = OCR_STATUS_FIN;
			break;
		default:
			status = OCR_STATUS_SKIP;
		}

		if (pos == frame.len)
			return status;

		/* increase the index within the frame */
		pos++;
	}

	return status;
}

int copy_from_uart(const void *dest, size_t count)
{
	data_handle_t data_handle;
	int32_t status;
	int32_t error = 0;
	int32_t count_err = 0;
	errno_t err;

	err = memset_s(&data_handle, sizeof(data_handle_t), 0, sizeof(data_handle_t));
	if (err != EOK)
		return ERROR;

	while (1) {
		status = uart_proc_loop(&data_handle);

		switch (status) {
		case OCR_STATUS_OK:
			serial_putc((uint8_t)ACK);
			if (data_handle.file_length != (uint32_t)count)
				count_err = 1;

			if (data_handle.count_frame == 1)
				data_handle.file_address = (uint32_t)(uintptr_t)dest;
			break;

		case OCR_STATUS_FIN:
			serial_putc((uint8_t)ACK);
			if (count_err)
				return ERROR;
			return OK;

		case OCR_STATUS_ERROR:
			serial_putc((uint8_t)NAK);
			break;

		default:
			++error;

			if (error > 0xff)
				return ERROR;

			break;
		}
	}
}
