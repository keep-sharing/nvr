#ifndef _rtp_header_h_
#define _rtp_header_h_

#include <stdint.h>

#define RTP_VERSION 2 // RTP version field must equal 2 (p66)
#if 0
//RFC3550 5.1 RTP Fixed Header Fields(p12)
typedef struct _rtp_header_t
{
	uint32_t v:2;		/* protocol version */
	uint32_t p:1;		/* padding flag */
	uint32_t x:1;		/* header extension flag */
	uint32_t cc:4;		/* CSRC count */
	uint32_t m:1;		/* marker bit */
	uint32_t pt:7;		/* payload type */
	uint32_t seq:16;	/* sequence number */
	uint32_t timestamp; /* timestamp */
	uint32_t ssrc;		/* synchronization source */
} rtp_header_t;
#else
//little endian
typedef struct _rtp_header_t
{
	uint8_t cc:4;		/* CSRC count */
	uint8_t x:1;		/* header extension flag */
	uint8_t p:1;		/* padding flag */
	uint8_t v:2;		/* protocol version */
	uint8_t pt:7;		/* payload type */
	uint8_t m:1;		/* marker bit */
	uint16_t seq;	/* sequence number */
	uint32_t timestamp; /* timestamp */
	uint32_t ssrc;		/* synchronization source */
} rtp_header_t;
#endif
#if 1
#define RTP_V(v)	((v >> 30) & 0x03) /* protocol version */
#define RTP_P(v)	((v >> 29) & 0x01) /* padding flag */
#define RTP_X(v)	((v >> 28) & 0x01) /* header extension flag */
#define RTP_CC(v)	((v >> 24) & 0x0F) /* CSRC count */
#define RTP_M(v)	((v >> 23) & 0x01) /* marker bit */
#define RTP_PT(v)	((v >> 16) & 0x7F) /* payload type */
#define RTP_SEQ(v)	((v >> 00) & 0xFFFF) /* sequence number */
#else
#define RTP_V(v)	(v & 0xC0000000) /* protocol version */
#define RTP_P(v)	(v & 0x02000000) /* padding flag */
#define RTP_X(v)	(v & 0x01000000) /* header extension flag */
#define RTP_CC(v)	(v & 0x00F00000) /* CSRC count */
#define RTP_M(v)	(v & 0x00800000) /* marker bit */
#define RTP_PT(v)	(v & 0x007F0000) /* payload type */
#define RTP_SEQ(v)	(v & 0x0000FFFF) /* sequence number */
#endif

#endif /* !_rtp_header_h_ */
