// RFC7798 RTP Payload Format for High Efficiency Video Coding (HEVC)

#include "rtp-packet.h"
#include "rtp-queue.h"
#include "rtp-payload-internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "app-log.h"

/*
0               1
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|F|    Type   |  LayerId  | TID |
+-------------+-----------------+

Forbidden zero(F) : 1 bit
NAL unit type(Type) : 6 bits
NUH layer ID(LayerId) : 6 bits
NUH temporal ID plus 1 (TID) : 3 bits
*/

#define H265_TYPE(v) ((v >> 1) & 0x3f)

#define FU_START(v) (v & 0x80)
#define FU_END(v)	(v & 0x40)
#define FU_NAL(v)	(v & 0x3F)

/**
 * Table 7-3: NAL unit type codes
 */
enum HEVCNALUnitType {
    HEVC_NAL_TRAIL_N    = 0,
    HEVC_NAL_TRAIL_R    = 1,
    HEVC_NAL_TSA_N      = 2,
    HEVC_NAL_TSA_R      = 3,
    HEVC_NAL_STSA_N     = 4,
    HEVC_NAL_STSA_R     = 5,
    HEVC_NAL_RADL_N     = 6,
    HEVC_NAL_RADL_R     = 7,
    HEVC_NAL_RASL_N     = 8,
    HEVC_NAL_RASL_R     = 9,
    HEVC_NAL_BLA_W_LP   = 16,
    HEVC_NAL_BLA_W_RADL = 17,
    HEVC_NAL_BLA_N_LP   = 18,
    HEVC_NAL_IDR_W_RADL = 19,
    HEVC_NAL_IDR_N_LP   = 20,
    HEVC_NAL_CRA_NUT    = 21,
    HEVC_NAL_VPS        = 32,
    HEVC_NAL_SPS        = 33,
    HEVC_NAL_PPS        = 34,
    HEVC_NAL_AUD        = 35,
    HEVC_NAL_EOS_NUT    = 36,
    HEVC_NAL_EOB_NUT    = 37,
    HEVC_NAL_FD_NUT     = 38,
    HEVC_NAL_SEI_PREFIX = 39,
    HEVC_NAL_SEI_SUFFIX = 40,
};

static const uint8_t start_sequence[] = { 0, 0, 0, 1 };
static const int start_sequence_size = sizeof(start_sequence);
#define H265_FRAGMENT_FLAG_START 1
#define H265_FRAGMENT_FLAG_END 0
//wyl add for record start slice
#define H265_FIND_FIRST_SLICE 1
#define H265_WITHOUT_FIRST_SLICE 0

struct rtp_decode_h265_t
{
	struct rtp_payload_t handler;
	void* cbparam;

//	uint16_t seq; // rtp seq

	uint8_t* ptr;
	int size, capacity;

	int8_t flags;
	uint8_t fragment_flags;
	uint8_t find_start_slice;
	int using_donl_field;

	ms_rtp_queue_t *queue;
};

static int rtp_h265_rtp_packet_parser(void *param, void *packet);
static void* rtp_h265_unpack_create(struct rtp_payload_t *handler, void* param)
{
	struct rtp_decode_h265_t *unpacker;
	unpacker = (struct rtp_decode_h265_t *)calloc(1, sizeof(*unpacker));
	if (!unpacker)
		return NULL;

	memcpy(&unpacker->handler, handler, sizeof(unpacker->handler));
	unpacker->cbparam = param;
	unpacker->flags = -1;
	unpacker->fragment_flags = 0;
	unpacker->find_start_slice = H265_WITHOUT_FIRST_SLICE;

	//handle rtp queue sequential
	struct ms_rtp_packet_parser_t rtp_parser;
	rtp_parser.rtp_parser = rtp_h265_rtp_packet_parser;
	unpacker->queue  = ms_rtp_queue_create(&rtp_parser, unpacker);
	if(!unpacker->queue)
	{
		free(unpacker);
		return NULL;
	}
	
	return unpacker;
}

static void rtp_h265_unpack_destroy(void* p)
{
	struct rtp_decode_h265_t *unpacker;
	unpacker = (struct rtp_decode_h265_t *)p;

	if (unpacker->ptr)
		free(unpacker->ptr);
	if(unpacker->queue)
		ms_rtp_queue_destroy(unpacker->queue);	
#if defined(_DEBUG) || defined(DEBUG)
	memset(unpacker, 0xCC, sizeof(*unpacker));
#endif
	free(unpacker);
}

// 4.4.2. Aggregation Packets (APs) (p25)
/*
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          RTP Header                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      PayloadHdr (Type=48)     |           NALU 1 DONL         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           NALU 1 Size         |            NALU 1 HDR         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                         NALU 1 Data . . .                     |
|                                                               |
+     . . .     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               |  NALU 2 DOND  |            NALU 2 Size        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          NALU 2 HDR           |                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+            NALU 2 Data        |
|                                                               |
|         . . .                 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
static int rtp_h265_unpack_ap(struct rtp_decode_h265_t *unpacker, const uint8_t* ptr, int bytes, uint32_t timestamp)
{
	int n;
	int len;

	n = unpacker->using_donl_field ? 4 : 2;
	for (bytes -= 2; bytes > n; bytes -= len + n)
	{
		ptr += n - 2; // skip DON
		len = nbo_r16(ptr);
		if (len + n > bytes)
		{
			//assert(0);
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_LOST;
			unpacker->size = 0;
			return -EINVAL; // error
		}

		//assert(H265_TYPE(ptr[n]) >= 0 && H265_TYPE(ptr[n]) <= 40);
		unpacker->handler.packet(unpacker->cbparam, ptr + 2, len, timestamp, unpacker->flags);
		unpacker->flags = 0;
		unpacker->size = 0;

		ptr += len + 2; // next NALU
	}

	return 1; // packet handled
}

// 4.4.3. Fragmentation Units (p29)
/*
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     PayloadHdr (Type=49)      |    FU header  |  DONL (cond)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|  DONL (cond)  |                                               |
|-+-+-+-+-+-+-+-+                                               |
|                           FU payload                          |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|   FuType  |
+---------------+
*/
static int rtp_h265_unpack_fu(struct rtp_decode_h265_t *unpacker, const uint8_t* ptr, int bytes, uint32_t timestamp)
{
	int n;
	uint8_t fuheader;
	uint8_t fu_nal;
	//wyl modify for hk-ipc one frame have many slice and one slice in whole rtp frame 
	uint8_t first_slice_segment_in_pic_flag;

	n = unpacker->using_donl_field ? 4 : 2;
	if (bytes < n + 1 /*FU header*/)
		return -EINVAL;

	if (unpacker->size + bytes - n - 1 + 2 + start_sequence_size /*NALU*/ > unpacker->capacity)
	{
		void* p = NULL;
		size_t size = unpacker->size + bytes + 256000 + 2 + start_sequence_size;
        if (size >= RTP_PAYLOAD_MAX_LEN) {
			unpacker->flags = 0;
			unpacker->size = 0;
			unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
			return 0;
		}
		p = realloc(unpacker->ptr, size);
		if (!p)
		{
			// set packet lost flag
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_LOST;
			unpacker->size = 0;
			return -ENOMEM;
		}
		unpacker->ptr = (uint8_t*)p;
		unpacker->capacity = size;
	}

	fuheader = ptr[2];
	fu_nal = FU_NAL(fuheader);
	if (FU_START(fuheader))
	{
		//handle more slice in different rtp packet
		if(fu_nal >= HEVC_NAL_VPS && fu_nal <= HEVC_NAL_SEI_SUFFIX)
		{
			if(unpacker->find_start_slice == H265_FIND_FIRST_SLICE)
			{
				unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
				unpacker->flags = 0;
				unpacker->size = 0;
				unpacker->fragment_flags = H265_FRAGMENT_FLAG_END;
				unpacker->find_start_slice = H265_WITHOUT_FIRST_SLICE;
			}
		}
		else if(fu_nal <= HEVC_NAL_RASL_R || (fu_nal>= HEVC_NAL_BLA_W_LP && fu_nal <= HEVC_NAL_CRA_NUT))
		{
			if(bytes > 3)
			{
				first_slice_segment_in_pic_flag = ptr[3] >> 7;
				if(first_slice_segment_in_pic_flag)
				{
					if(unpacker->find_start_slice == H265_FIND_FIRST_SLICE)
					{
						unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
						unpacker->flags = 0;
						unpacker->size = 0;
						unpacker->fragment_flags = H265_FRAGMENT_FLAG_END;
					}
					unpacker->find_start_slice = H265_FIND_FIRST_SLICE;
				}			
			}
		}
		memcpy(&unpacker->ptr[unpacker->size], start_sequence, start_sequence_size);
		unpacker->size += start_sequence_size; 		
		// NAL unit type byte
		unpacker->ptr[unpacker->size] = fu_nal << 1;
		unpacker->ptr[unpacker->size+1] = 1;
		unpacker->size += 2;
		if(fu_nal == HEVC_NAL_IDR_W_RADL)//I frame
		{
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_IDR;
		}
		unpacker->fragment_flags = H265_FRAGMENT_FLAG_START;
	}
	else
	{
		if (unpacker->fragment_flags == H265_FRAGMENT_FLAG_END)//lost fu_start packet
		{
			app_log_printf(LOG_INFO, "======lost h265 fu_start rtp packet====");
			memcpy(&unpacker->ptr[unpacker->size], start_sequence, start_sequence_size);
			unpacker->size += start_sequence_size; 
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_LOST;	
			// NAL unit type byte
			unpacker->ptr[unpacker->size] = fu_nal << 1;
			unpacker->ptr[unpacker->size+1] = 1;
			unpacker->size += 2; 
			if(fu_nal == HEVC_NAL_IDR_W_RADL)//I frame
			{
				unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_IDR;
			}
			unpacker->fragment_flags = H265_FRAGMENT_FLAG_START;
		}
	}

	if (bytes > n + 1)
	{
		memmove(unpacker->ptr + unpacker->size, ptr + n + 1, bytes - n - 1);
		unpacker->size += bytes - n - 1;
	}

	//wyl modify for hk-ipc one frame have many slice and one slice in whole rtp packets 
	/*if (FU_END(fuheader))
	{
		unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
		unpacker->flags = 0;
		unpacker->fragment_flags = H265_FRAGMENT_FLAG_END;
		unpacker->size = 0;		
	}*/

	return 1; // packet handled
}

static int rtp_h265_unpack_single_nal(struct rtp_decode_h265_t *unpacker, const uint8_t* ptr, int bytes, uint32_t timestamp)
{
	int nal_type = 0;
	uint8_t first_slice_segment_in_pic_flag;
	if (unpacker->size + bytes + start_sequence_size /*start_sequence size*/ > unpacker->capacity)
	{
		void* p = NULL;
		int size = unpacker->size + bytes + 128000 + start_sequence_size;
		if (size >= RTP_PAYLOAD_MAX_LEN) {
			unpacker->flags = 0;
			unpacker->size = 0;
			unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
			return 0;
		}
		p = realloc(unpacker->ptr, size);
		if (!p)
		{
			// set packet lost flag
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_LOST;
			unpacker->size = 0;
			return -ENOMEM; // error
		}
		unpacker->ptr = (uint8_t*)p;
		unpacker->capacity = size;
	}
	nal_type = H265_TYPE(ptr[0]);
	if(nal_type >= HEVC_NAL_VPS && nal_type <= HEVC_NAL_SEI_SUFFIX)
	{
		if(unpacker->find_start_slice == H265_FIND_FIRST_SLICE)
		{
			unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
			unpacker->flags = 0;
			unpacker->size = 0; 
			unpacker->fragment_flags = H265_FRAGMENT_FLAG_END;
			unpacker->find_start_slice = H265_WITHOUT_FIRST_SLICE;
		}
	}
	else if(nal_type <= HEVC_NAL_RASL_R || (nal_type>= HEVC_NAL_BLA_W_LP && nal_type <= HEVC_NAL_CRA_NUT))
	{
		if(bytes > 2)
		{
			first_slice_segment_in_pic_flag = ptr[2] >> 7;
			if(first_slice_segment_in_pic_flag)
			{
				if(unpacker->find_start_slice == H265_FIND_FIRST_SLICE)
				{
					unpacker->handler.packet(unpacker->cbparam, unpacker->ptr, unpacker->size, timestamp, unpacker->flags);
					unpacker->flags = 0;
					unpacker->size = 0;
					unpacker->fragment_flags = H265_FRAGMENT_FLAG_END;
				}
				unpacker->find_start_slice = H265_FIND_FIRST_SLICE;
			}
		}
		if(nal_type == HEVC_NAL_IDR_W_RADL)
		{
			unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_IDR;
		}
	}
	memmove(&unpacker->ptr[unpacker->size], start_sequence, start_sequence_size);
	unpacker->size += start_sequence_size; 
	memmove(&unpacker->ptr[unpacker->size], ptr, bytes);
	unpacker->size += bytes; 	
	return 1;
}

static int rtp_h265_rtp_packet_parser(void *param, void *packet)
{
	int nal;
	const uint8_t* ptr;	
	struct rtp_packet_t *pkt = NULL;
	struct rtp_decode_h265_t *unpacker;

	unpacker = (struct rtp_decode_h265_t *)param;
	pkt = (struct rtp_packet_t *)packet;
	if(!unpacker ||!pkt)
		return -EINVAL;	
	
	ptr = (const uint8_t*)pkt->payload;
	nal = H265_TYPE(ptr[0]);
	//lid = ((ptr[0] & 0x01) << 5) | ((ptr[1] >> 3) & 0x1f);
	//tid = ptr[1] & 0x07;
	//assert(0 == lid && 0 != tid);

	if (nal > 50)
		return 0; // packet discard, Unsupported (HEVC) NAL type

	switch (nal)
	{
	case 48: // aggregated packet (AP) - with two or more NAL units
		return rtp_h265_unpack_ap(unpacker, ptr, pkt->payloadlen, pkt->rtp.timestamp);

	case 49: // fragmentation unit (FU)
		return rtp_h265_unpack_fu(unpacker, ptr, pkt->payloadlen, pkt->rtp.timestamp);

	case 50: // TODO: 4.4.4. PACI Packets (p32)
	case HEVC_NAL_RASL_N:
		return 0; // packet discard

	case 32: // video parameter set (VPS)
	case 33: // sequence parameter set (SPS)
	case 34: // picture parameter set (PPS)
	case 39: // supplemental enhancement information (SEI)
	default: // 4.4.1. Single NAL Unit Packets (p24)
		return rtp_h265_unpack_single_nal(unpacker, ptr, pkt->payloadlen, pkt->rtp.timestamp);
		/*unpacker->handler.packet(unpacker->cbparam, ptr, pkt.payloadlen, pkt.rtp.timestamp, unpacker->flags);
		unpacker->flags = 0;
		unpacker->size = 0;
		return 1;*/ // packet handled
	}	
}

static int rtp_h265_unpack_input(void* p, const void* packet, int bytes)
{
	struct rtp_packet_t *pkt = NULL;
	struct rtp_decode_h265_t *unpacker;

	unpacker = (struct rtp_decode_h265_t *)p;
	if (!unpacker)
		return -EINVAL;
	unpacker->handler.get_rtp_packet_ptr(unpacker->cbparam, (void **)&pkt);
	if (!pkt || 0 != rtp_packet_deserialize(pkt, packet, bytes))
		return -EINVAL;

	if(pkt->rtp.pt == 112 && pkt->rtp.x && pkt->extlen)
	{
		if(pkt->extension)
			unpacker->handler.packet(unpacker->cbparam, pkt->extension, pkt->extlen, pkt->rtp.timestamp, RTP_PAYLOAD_FLAG_PACKET_MS_METADATA);
		return 1;
	}
	else if(pkt->payloadlen < (unpacker->using_donl_field ? 4 : 2))
	{
		return -EINVAL;
	}

	/*if (-1 == unpacker->flags)
	{
		unpacker->flags = 0;
		unpacker->seq = (uint16_t)(pkt->rtp.seq - 1); // disable packet lost
	}

	if ((uint16_t)pkt->rtp.seq != (uint16_t)(unpacker->seq + 1))
	{
		unpacker->flags |= RTP_PAYLOAD_FLAG_PACKET_LOST;
		//unpacker->size = 0; // discard previous packets
		app_log_printf(LOG_INFO, "lost packet %d", pkt->rtp.seq - unpacker->seq - 1);
	}
	unpacker->seq = (uint16_t)pkt->rtp.seq;*/
	return ms_rtp_queue_parse_rtppacket(unpacker->queue, pkt);

}

struct rtp_payload_decode_t *rtp_h265_decode()
{
	static struct rtp_payload_decode_t unpacker = {
		rtp_h265_unpack_create,
		rtp_h265_unpack_destroy,
		rtp_h265_unpack_input,
	};

	return &unpacker;
}
