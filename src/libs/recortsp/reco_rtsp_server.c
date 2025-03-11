#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#define __USE_GNU
#if HAVE_DLFCN_H
    #include <dlfcn.h>
#endif

#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <sys/prctl.h>
#include <sched.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "osa/osa_que.h"

#include "linkedlists.h"
#include "osa/osa_mutex.h"
#include "osa/osa_thr.h"
#include "net_info.h"
#include "openssl/md5.h"

#include "msstd.h"
#include "recortsp.h"
#include "reco_rtsp_common.h"
#include "reco_rtsp_buffer.h"
#include "ms_rtsp_server_stream.h"
#include "ms_stream_common.h"

#define DAVID_DEBUG                     1
#define BUFINFO_INVALIDID               (~0u)
#define MAX_STREAMS                     3
#define DEFAULT_RTSP_PORT               554
#define DEFAULT_MAX_CONNECTION          (1024)
#define DEFAULT_RTSP_TIMEOUT            90000
#define DEFAULT_RTP_PORT_MIN            6500
#define DEFAULT_RTP_PORT_MAX            8000
//#define RTSP_STATUS_UNAUTHORIZED        401

#define MAX_EAGAIN                      3

#define CODECTYPE_H264                  0
#define CODECTYPE_MPEG4                 1
#define CODECTYPE_MJPEG                 2
#define CODECTYPE_H265                  3
#define BUFFER_SIZE                     1024
#define WEBSOCKET_KEY_MAX_LEN           128
#define AVG_SIZE                        1500
#define MAX_STREAM_CTX                  16

#define ms_http_url_opt                 "opt="
#define ms_http_url_sessionid           "sessionid="
#define GUID                            "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define ms_http_url_auth                "Authorization:"

#define MS_HTTP_BUFF_SIZE               (2*1024*1024)
#define DEFAULT_DETECT_INTERVAL         (60+2)
#define BANDWIDTH_UNIT                  1024
#define MAX_TX_CNT                      5
#define MAX_RTSP_FRAME_BUFF             (5*1024*1024)

#if defined(_HI3536C_)
    #define MAX_RTSP_THREAD                 2
#else
    #define MAX_RTSP_THREAD                 4
#endif

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

typedef enum {
    RTSPSTATE_WAIT_REQUEST = 0,
    RTSPSTATE_READY,
    RTSPSTATE_SEND_REPLY,
    RTSPSTATE_SEND_DATA,/* sending TCP or UDP data */
    RTSPSTATE_SEND_PACKET,
    RTSPSTATE_STOPPING,
    RTSPSTATE_CLOSE
} RTSPSS_STATE;

enum RTSPLowerTransport {
    RTSP_LOWER_TRANSPORT_UDP = 0,           /**< UDP/unicast */
    RTSP_LOWER_TRANSPORT_TCP = 1,           /**< TCP; interleaved in RTSP */
    RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2, /**< UDP/multicast */
    RTSP_LOWER_TRANSPORT_NB,
    RTSP_LOWER_TRANSPORT_HTTP = 8,          /**< HTTP tunneled - not a proper
                                                 transport mode as such,
                                                 only for use via AVOptions */
    RTSP_LOWER_TRANSPORT_CUSTOM = 16,       /**< Custom IO - not a public
                                                 option for lower_transport_mask,
                                                 but set in the SDP demuxer based
                                                 on a flag. */
};

/** RTSP handling */
enum RTSPStatusCode {
    RTSP_STATUS_CONTINUE             = 100,
    RTSP_STATUS_OK                   = 200,
    RTSP_STATUS_CREATED              = 201,
    RTSP_STATUS_LOW_ON_STORAGE_SPACE = 250,
    RTSP_STATUS_MULTIPLE_CHOICES     = 300,
    RTSP_STATUS_MOVED_PERMANENTLY    = 301,
    RTSP_STATUS_MOVED_TEMPORARILY    = 302,
    RTSP_STATUS_SEE_OTHER            = 303,
    RTSP_STATUS_NOT_MODIFIED         = 304,
    RTSP_STATUS_USE_PROXY            = 305,
    RTSP_STATUS_BAD_REQUEST          = 400,
    RTSP_STATUS_UNAUTHORIZED         = 401,
    RTSP_STATUS_PAYMENT_REQUIRED     = 402,
    RTSP_STATUS_FORBIDDEN            = 403,
    RTSP_STATUS_NOT_FOUND            = 404,
    RTSP_STATUS_METHOD               = 405,
    RTSP_STATUS_NOT_ACCEPTABLE       = 406,
    RTSP_STATUS_PROXY_AUTH_REQUIRED  = 407,
    RTSP_STATUS_REQ_TIME_OUT         = 408,
    RTSP_STATUS_GONE                 = 410,
    RTSP_STATUS_LENGTH_REQUIRED      = 411,
    RTSP_STATUS_PRECONDITION_FAILED  = 412,
    RTSP_STATUS_REQ_ENTITY_2LARGE    = 413,
    RTSP_STATUS_REQ_URI_2LARGE       = 414,
    RTSP_STATUS_UNSUPPORTED_MTYPE    = 415,
    RTSP_STATUS_PARAM_NOT_UNDERSTOOD = 451,
    RTSP_STATUS_CONFERENCE_NOT_FOUND = 452,
    RTSP_STATUS_BANDWIDTH            = 453,
    RTSP_STATUS_SESSION              = 454,
    RTSP_STATUS_STATE                = 455,
    RTSP_STATUS_INVALID_HEADER_FIELD = 456,
    RTSP_STATUS_INVALID_RANGE        = 457,
    RTSP_STATUS_RONLY_PARAMETER      = 458,
    RTSP_STATUS_AGGREGATE            = 459,
    RTSP_STATUS_ONLY_AGGREGATE       = 460,
    RTSP_STATUS_TRANSPORT            = 461,
    RTSP_STATUS_UNREACHABLE          = 462,
    RTSP_STATUS_INTERNAL             = 500,
    RTSP_STATUS_NOT_IMPLEMENTED      = 501,
    RTSP_STATUS_BAD_GATEWAY          = 502,
    RTSP_STATUS_SERVICE              = 503,
    RTSP_STATUS_GATEWAY_TIME_OUT     = 504,
    RTSP_STATUS_VERSION              = 505,
    RTSP_STATUS_UNSUPPORTED_OPTION   = 551,
};

enum AVCodecID {
    AV_CODEC_ID_NONE,

    /* video codecs */
    AV_CODEC_ID_MPEG1VIDEO,
    AV_CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
#if FF_API_XVMC
    AV_CODEC_ID_MPEG2VIDEO_XVMC,
#endif /* FF_API_XVMC */
    AV_CODEC_ID_H261,
    AV_CODEC_ID_H263,
    AV_CODEC_ID_RV10,
    AV_CODEC_ID_RV20,
    AV_CODEC_ID_MJPEG,
    AV_CODEC_ID_MJPEGB,
    AV_CODEC_ID_LJPEG,
    AV_CODEC_ID_SP5X,
    AV_CODEC_ID_JPEGLS,
    AV_CODEC_ID_MPEG4,
    AV_CODEC_ID_RAWVIDEO,
    AV_CODEC_ID_MSMPEG4V1,
    AV_CODEC_ID_MSMPEG4V2,
    AV_CODEC_ID_MSMPEG4V3,
    AV_CODEC_ID_WMV1,
    AV_CODEC_ID_WMV2,
    AV_CODEC_ID_H263P,
    AV_CODEC_ID_H263I,
    AV_CODEC_ID_FLV1,
    AV_CODEC_ID_SVQ1,
    AV_CODEC_ID_SVQ3,
    AV_CODEC_ID_DVVIDEO,
    AV_CODEC_ID_HUFFYUV,
    AV_CODEC_ID_CYUV,
    AV_CODEC_ID_H264,
    AV_CODEC_ID_INDEO3,
    AV_CODEC_ID_VP3,
    AV_CODEC_ID_THEORA,
    AV_CODEC_ID_ASV1,
    AV_CODEC_ID_ASV2,
    AV_CODEC_ID_FFV1,
    AV_CODEC_ID_4XM,
    AV_CODEC_ID_VCR1,
    AV_CODEC_ID_CLJR,
    AV_CODEC_ID_MDEC,
    AV_CODEC_ID_ROQ,
    AV_CODEC_ID_INTERPLAY_VIDEO,
    AV_CODEC_ID_XAN_WC3,
    AV_CODEC_ID_XAN_WC4,
    AV_CODEC_ID_RPZA,
    AV_CODEC_ID_CINEPAK,
    AV_CODEC_ID_WS_VQA,
    AV_CODEC_ID_MSRLE,
    AV_CODEC_ID_MSVIDEO1,
    AV_CODEC_ID_IDCIN,
    AV_CODEC_ID_8BPS,
    AV_CODEC_ID_SMC,
    AV_CODEC_ID_FLIC,
    AV_CODEC_ID_TRUEMOTION1,
    AV_CODEC_ID_VMDVIDEO,
    AV_CODEC_ID_MSZH,
    AV_CODEC_ID_ZLIB,
    AV_CODEC_ID_QTRLE,
    AV_CODEC_ID_TSCC,
    AV_CODEC_ID_ULTI,
    AV_CODEC_ID_QDRAW,
    AV_CODEC_ID_VIXL,
    AV_CODEC_ID_QPEG,
    AV_CODEC_ID_PNG,
    AV_CODEC_ID_PPM,
    AV_CODEC_ID_PBM,
    AV_CODEC_ID_PGM,
    AV_CODEC_ID_PGMYUV,
    AV_CODEC_ID_PAM,
    AV_CODEC_ID_FFVHUFF,
    AV_CODEC_ID_RV30,
    AV_CODEC_ID_RV40,
    AV_CODEC_ID_VC1,
    AV_CODEC_ID_WMV3,
    AV_CODEC_ID_LOCO,
    AV_CODEC_ID_WNV1,
    AV_CODEC_ID_AASC,
    AV_CODEC_ID_INDEO2,
    AV_CODEC_ID_FRAPS,
    AV_CODEC_ID_TRUEMOTION2,
    AV_CODEC_ID_BMP,
    AV_CODEC_ID_CSCD,
    AV_CODEC_ID_MMVIDEO,
    AV_CODEC_ID_ZMBV,
    AV_CODEC_ID_AVS,
    AV_CODEC_ID_SMACKVIDEO,
    AV_CODEC_ID_NUV,
    AV_CODEC_ID_KMVC,
    AV_CODEC_ID_FLASHSV,
    AV_CODEC_ID_CAVS,
    AV_CODEC_ID_JPEG2000,
    AV_CODEC_ID_VMNC,
    AV_CODEC_ID_VP5,
    AV_CODEC_ID_VP6,
    AV_CODEC_ID_VP6F,
    AV_CODEC_ID_TARGA,
    AV_CODEC_ID_DSICINVIDEO,
    AV_CODEC_ID_TIERTEXSEQVIDEO,
    AV_CODEC_ID_TIFF,
    AV_CODEC_ID_GIF,
    AV_CODEC_ID_DXA,
    AV_CODEC_ID_DNXHD,
    AV_CODEC_ID_THP,
    AV_CODEC_ID_SGI,
    AV_CODEC_ID_C93,
    AV_CODEC_ID_BETHSOFTVID,
    AV_CODEC_ID_PTX,
    AV_CODEC_ID_TXD,
    AV_CODEC_ID_VP6A,
    AV_CODEC_ID_AMV,
    AV_CODEC_ID_VB,
    AV_CODEC_ID_PCX,
    AV_CODEC_ID_SUNRAST,
    AV_CODEC_ID_INDEO4,
    AV_CODEC_ID_INDEO5,
    AV_CODEC_ID_MIMIC,
    AV_CODEC_ID_RL2,
    AV_CODEC_ID_ESCAPE124,
    AV_CODEC_ID_DIRAC,
    AV_CODEC_ID_BFI,
    AV_CODEC_ID_CMV,
    AV_CODEC_ID_MOTIONPIXELS,
    AV_CODEC_ID_TGV,
    AV_CODEC_ID_TGQ,
    AV_CODEC_ID_TQI,
    AV_CODEC_ID_AURA,
    AV_CODEC_ID_AURA2,
    AV_CODEC_ID_V210X,
    AV_CODEC_ID_TMV,
    AV_CODEC_ID_V210,
    AV_CODEC_ID_DPX,
    AV_CODEC_ID_MAD,
    AV_CODEC_ID_FRWU,
    AV_CODEC_ID_FLASHSV2,
    AV_CODEC_ID_CDGRAPHICS,
    AV_CODEC_ID_R210,
    AV_CODEC_ID_ANM,
    AV_CODEC_ID_BINKVIDEO,
    AV_CODEC_ID_IFF_ILBM,
    AV_CODEC_ID_IFF_BYTERUN1,
    AV_CODEC_ID_KGV1,
    AV_CODEC_ID_YOP,
    AV_CODEC_ID_VP8,
    AV_CODEC_ID_PICTOR,
    AV_CODEC_ID_ANSI,
    AV_CODEC_ID_A64_MULTI,
    AV_CODEC_ID_A64_MULTI5,
    AV_CODEC_ID_R10K,
    AV_CODEC_ID_MXPEG,
    AV_CODEC_ID_LAGARITH,
    AV_CODEC_ID_PRORES,
    AV_CODEC_ID_JV,
    AV_CODEC_ID_DFA,
    AV_CODEC_ID_WMV3IMAGE,
    AV_CODEC_ID_VC1IMAGE,
    AV_CODEC_ID_UTVIDEO,
    AV_CODEC_ID_BMV_VIDEO,
    AV_CODEC_ID_VBLE,
    AV_CODEC_ID_DXTORY,
    AV_CODEC_ID_V410,
    AV_CODEC_ID_XWD,
    AV_CODEC_ID_CDXL,
    AV_CODEC_ID_XBM,
    AV_CODEC_ID_ZEROCODEC,
    AV_CODEC_ID_MSS1,
    AV_CODEC_ID_MSA1,
    AV_CODEC_ID_TSCC2,
    AV_CODEC_ID_MTS2,
    AV_CODEC_ID_CLLC,
    AV_CODEC_ID_MSS2,
    AV_CODEC_ID_VP9,
    AV_CODEC_ID_AIC,
    AV_CODEC_ID_ESCAPE130_DEPRECATED,
    AV_CODEC_ID_G2M_DEPRECATED,
    AV_CODEC_ID_WEBP_DEPRECATED,
    AV_CODEC_ID_HNM4_VIDEO,
    AV_CODEC_ID_HEVC_DEPRECATED,
    AV_CODEC_ID_FIC,
    AV_CODEC_ID_ALIAS_PIX,
    AV_CODEC_ID_BRENDER_PIX_DEPRECATED,
    AV_CODEC_ID_PAF_VIDEO_DEPRECATED,
    AV_CODEC_ID_EXR_DEPRECATED,
    AV_CODEC_ID_VP7_DEPRECATED,
    AV_CODEC_ID_SANM_DEPRECATED,
    AV_CODEC_ID_SGIRLE_DEPRECATED,
    AV_CODEC_ID_MVC1_DEPRECATED,
    AV_CODEC_ID_MVC2_DEPRECATED,
    AV_CODEC_ID_HQX,
    AV_CODEC_ID_TDSC,
    AV_CODEC_ID_HQ_HQA,

    AV_CODEC_ID_BRENDER_PIX = MKBETAG('B', 'P', 'I', 'X'),
    AV_CODEC_ID_Y41P       = MKBETAG('Y', '4', '1', 'P'),
    AV_CODEC_ID_ESCAPE130  = MKBETAG('E', '1', '3', '0'),
    AV_CODEC_ID_EXR        = MKBETAG('0', 'E', 'X', 'R'),
    AV_CODEC_ID_AVRP       = MKBETAG('A', 'V', 'R', 'P'),

    AV_CODEC_ID_012V       = MKBETAG('0', '1', '2', 'V'),
    AV_CODEC_ID_G2M        = MKBETAG(0, 'G', '2', 'M'),
    AV_CODEC_ID_AVUI       = MKBETAG('A', 'V', 'U', 'I'),
    AV_CODEC_ID_AYUV       = MKBETAG('A', 'Y', 'U', 'V'),
    AV_CODEC_ID_TARGA_Y216 = MKBETAG('T', '2', '1', '6'),
    AV_CODEC_ID_V308       = MKBETAG('V', '3', '0', '8'),
    AV_CODEC_ID_V408       = MKBETAG('V', '4', '0', '8'),
    AV_CODEC_ID_YUV4       = MKBETAG('Y', 'U', 'V', '4'),
    AV_CODEC_ID_SANM       = MKBETAG('S', 'A', 'N', 'M'),
    AV_CODEC_ID_PAF_VIDEO  = MKBETAG('P', 'A', 'F', 'V'),
    AV_CODEC_ID_AVRN       = MKBETAG('A', 'V', 'R', 'n'),
    AV_CODEC_ID_CPIA       = MKBETAG('C', 'P', 'I', 'A'),
    AV_CODEC_ID_XFACE      = MKBETAG('X', 'F', 'A', 'C'),
    AV_CODEC_ID_SGIRLE     = MKBETAG('S', 'G', 'I', 'R'),
    AV_CODEC_ID_MVC1       = MKBETAG('M', 'V', 'C', '1'),
    AV_CODEC_ID_MVC2       = MKBETAG('M', 'V', 'C', '2'),
    AV_CODEC_ID_SNOW       = MKBETAG('S', 'N', 'O', 'W'),
    AV_CODEC_ID_WEBP       = MKBETAG('W', 'E', 'B', 'P'),
    AV_CODEC_ID_SMVJPEG    = MKBETAG('S', 'M', 'V', 'J'),
    AV_CODEC_ID_HEVC       = MKBETAG('H', '2', '6', '5'),
#define AV_CODEC_ID_H265 AV_CODEC_ID_HEVC
    AV_CODEC_ID_VP7        = MKBETAG('V', 'P', '7', '0'),
    AV_CODEC_ID_APNG       = MKBETAG('A', 'P', 'N', 'G'),

    /* various PCM "codecs" */
    AV_CODEC_ID_FIRST_AUDIO = 0x10000,     ///< A dummy id pointing at the start of audio codecs
    AV_CODEC_ID_PCM_S16LE = 0x10000,
    AV_CODEC_ID_PCM_S16BE,
    AV_CODEC_ID_PCM_U16LE,
    AV_CODEC_ID_PCM_U16BE,
    AV_CODEC_ID_PCM_S8,
    AV_CODEC_ID_PCM_U8,
    AV_CODEC_ID_PCM_MULAW,
    AV_CODEC_ID_PCM_ALAW,
    AV_CODEC_ID_PCM_S32LE,
    AV_CODEC_ID_PCM_S32BE,
    AV_CODEC_ID_PCM_U32LE,
    AV_CODEC_ID_PCM_U32BE,
    AV_CODEC_ID_PCM_S24LE,
    AV_CODEC_ID_PCM_S24BE,
    AV_CODEC_ID_PCM_U24LE,
    AV_CODEC_ID_PCM_U24BE,
    AV_CODEC_ID_PCM_S24DAUD,
    AV_CODEC_ID_PCM_ZORK,
    AV_CODEC_ID_PCM_S16LE_PLANAR,
    AV_CODEC_ID_PCM_DVD,
    AV_CODEC_ID_PCM_F32BE,
    AV_CODEC_ID_PCM_F32LE,
    AV_CODEC_ID_PCM_F64BE,
    AV_CODEC_ID_PCM_F64LE,
    AV_CODEC_ID_PCM_BLURAY,
    AV_CODEC_ID_PCM_LXF,
    AV_CODEC_ID_S302M,
    AV_CODEC_ID_PCM_S8_PLANAR,
    AV_CODEC_ID_PCM_S24LE_PLANAR_DEPRECATED,
    AV_CODEC_ID_PCM_S32LE_PLANAR_DEPRECATED,
    AV_CODEC_ID_PCM_S24LE_PLANAR = MKBETAG(24, 'P', 'S', 'P'),
    AV_CODEC_ID_PCM_S32LE_PLANAR = MKBETAG(32, 'P', 'S', 'P'),
    AV_CODEC_ID_PCM_S16BE_PLANAR = MKBETAG('P', 'S', 'P', 16),

    /* various ADPCM codecs */
    AV_CODEC_ID_ADPCM_IMA_QT = 0x11000,
    AV_CODEC_ID_ADPCM_IMA_WAV,
    AV_CODEC_ID_ADPCM_IMA_DK3,
    AV_CODEC_ID_ADPCM_IMA_DK4,
    AV_CODEC_ID_ADPCM_IMA_WS,
    AV_CODEC_ID_ADPCM_IMA_SMJPEG,
    AV_CODEC_ID_ADPCM_MS,
    AV_CODEC_ID_ADPCM_4XM,
    AV_CODEC_ID_ADPCM_XA,
    AV_CODEC_ID_ADPCM_ADX,
    AV_CODEC_ID_ADPCM_EA,
    AV_CODEC_ID_ADPCM_G726,
    AV_CODEC_ID_ADPCM_CT,
    AV_CODEC_ID_ADPCM_SWF,
    AV_CODEC_ID_ADPCM_YAMAHA,
    AV_CODEC_ID_ADPCM_SBPRO_4,
    AV_CODEC_ID_ADPCM_SBPRO_3,
    AV_CODEC_ID_ADPCM_SBPRO_2,
    AV_CODEC_ID_ADPCM_THP,
    AV_CODEC_ID_ADPCM_IMA_AMV,
    AV_CODEC_ID_ADPCM_EA_R1,
    AV_CODEC_ID_ADPCM_EA_R3,
    AV_CODEC_ID_ADPCM_EA_R2,
    AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
    AV_CODEC_ID_ADPCM_IMA_EA_EACS,
    AV_CODEC_ID_ADPCM_EA_XAS,
    AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
    AV_CODEC_ID_ADPCM_IMA_ISS,
    AV_CODEC_ID_ADPCM_G722,
    AV_CODEC_ID_ADPCM_IMA_APC,
    AV_CODEC_ID_ADPCM_VIMA_DEPRECATED,
    AV_CODEC_ID_ADPCM_VIMA = MKBETAG('V', 'I', 'M', 'A'),
#if FF_API_VIMA_DECODER
    AV_CODEC_ID_VIMA       = MKBETAG('V', 'I', 'M', 'A'),
#endif
    AV_CODEC_ID_ADPCM_AFC  = MKBETAG('A', 'F', 'C', ' '),
    AV_CODEC_ID_ADPCM_IMA_OKI = MKBETAG('O', 'K', 'I', ' '),
    AV_CODEC_ID_ADPCM_DTK  = MKBETAG('D', 'T', 'K', ' '),
    AV_CODEC_ID_ADPCM_IMA_RAD = MKBETAG('R', 'A', 'D', ' '),
    AV_CODEC_ID_ADPCM_G726LE = MKBETAG('6', '2', '7', 'G'),

    /* AMR */
    AV_CODEC_ID_AMR_NB = 0x12000,
    AV_CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    AV_CODEC_ID_RA_144 = 0x13000,
    AV_CODEC_ID_RA_288,

    /* various DPCM codecs */
    AV_CODEC_ID_ROQ_DPCM = 0x14000,
    AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM,
    AV_CODEC_ID_SOL_DPCM,

    /* audio codecs */
    AV_CODEC_ID_MP2 = 0x15000,
    AV_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    AV_CODEC_ID_AAC,
    AV_CODEC_ID_AC3,
    AV_CODEC_ID_DTS,
    AV_CODEC_ID_VORBIS,
    AV_CODEC_ID_DVAUDIO,
    AV_CODEC_ID_WMAV1,
    AV_CODEC_ID_WMAV2,
    AV_CODEC_ID_MACE3,
    AV_CODEC_ID_MACE6,
    AV_CODEC_ID_VMDAUDIO,
    AV_CODEC_ID_FLAC,
    AV_CODEC_ID_MP3ADU,
    AV_CODEC_ID_MP3ON4,
    AV_CODEC_ID_SHORTEN,
    AV_CODEC_ID_ALAC,
    AV_CODEC_ID_WESTWOOD_SND1,
    AV_CODEC_ID_GSM, ///< as in Berlin toast format
    AV_CODEC_ID_QDM2,
    AV_CODEC_ID_COOK,
    AV_CODEC_ID_TRUESPEECH,
    AV_CODEC_ID_TTA,
    AV_CODEC_ID_SMACKAUDIO,
    AV_CODEC_ID_QCELP,
    AV_CODEC_ID_WAVPACK,
    AV_CODEC_ID_DSICINAUDIO,
    AV_CODEC_ID_IMC,
    AV_CODEC_ID_MUSEPACK7,
    AV_CODEC_ID_MLP,
    AV_CODEC_ID_GSM_MS, /* as found in WAV */
    AV_CODEC_ID_ATRAC3,
#if FF_API_VOXWARE
    AV_CODEC_ID_VOXWARE,
#endif
    AV_CODEC_ID_APE,
    AV_CODEC_ID_NELLYMOSER,
    AV_CODEC_ID_MUSEPACK8,
    AV_CODEC_ID_SPEEX,
    AV_CODEC_ID_WMAVOICE,
    AV_CODEC_ID_WMAPRO,
    AV_CODEC_ID_WMALOSSLESS,
    AV_CODEC_ID_ATRAC3P,
    AV_CODEC_ID_EAC3,
    AV_CODEC_ID_SIPR,
    AV_CODEC_ID_MP1,
    AV_CODEC_ID_TWINVQ,
    AV_CODEC_ID_TRUEHD,
    AV_CODEC_ID_MP4ALS,
    AV_CODEC_ID_ATRAC1,
    AV_CODEC_ID_BINKAUDIO_RDFT,
    AV_CODEC_ID_BINKAUDIO_DCT,
    AV_CODEC_ID_AAC_LATM,
    AV_CODEC_ID_QDMC,
    AV_CODEC_ID_CELT,
    AV_CODEC_ID_G723_1,
    AV_CODEC_ID_G729,
    AV_CODEC_ID_8SVX_EXP,
    AV_CODEC_ID_8SVX_FIB,
    AV_CODEC_ID_BMV_AUDIO,
    AV_CODEC_ID_RALF,
    AV_CODEC_ID_IAC,
    AV_CODEC_ID_ILBC,
    AV_CODEC_ID_OPUS_DEPRECATED,
    AV_CODEC_ID_COMFORT_NOISE,
    AV_CODEC_ID_TAK_DEPRECATED,
    AV_CODEC_ID_METASOUND,
    AV_CODEC_ID_PAF_AUDIO_DEPRECATED,
    AV_CODEC_ID_ON2AVC,
    AV_CODEC_ID_DSS_SP,
    AV_CODEC_ID_FFWAVESYNTH = MKBETAG('F', 'F', 'W', 'S'),
    AV_CODEC_ID_SONIC       = MKBETAG('S', 'O', 'N', 'C'),
    AV_CODEC_ID_SONIC_LS    = MKBETAG('S', 'O', 'N', 'L'),
    AV_CODEC_ID_PAF_AUDIO   = MKBETAG('P', 'A', 'F', 'A'),
    AV_CODEC_ID_OPUS        = MKBETAG('O', 'P', 'U', 'S'),
    AV_CODEC_ID_TAK         = MKBETAG('t', 'B', 'a', 'K'),
    AV_CODEC_ID_EVRC        = MKBETAG('s', 'e', 'v', 'c'),
    AV_CODEC_ID_SMV         = MKBETAG('s', 's', 'm', 'v'),
    AV_CODEC_ID_DSD_LSBF    = MKBETAG('D', 'S', 'D', 'L'),
    AV_CODEC_ID_DSD_MSBF    = MKBETAG('D', 'S', 'D', 'M'),
    AV_CODEC_ID_DSD_LSBF_PLANAR = MKBETAG('D', 'S', 'D', '1'),
    AV_CODEC_ID_DSD_MSBF_PLANAR = MKBETAG('D', 'S', 'D', '8'),

    /* subtitle codecs */
    AV_CODEC_ID_FIRST_SUBTITLE = 0x17000,          ///< A dummy ID pointing at the start of subtitle codecs.
    AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
    AV_CODEC_ID_DVB_SUBTITLE,
    AV_CODEC_ID_TEXT,  ///< raw UTF-8 text
    AV_CODEC_ID_XSUB,
    AV_CODEC_ID_SSA,
    AV_CODEC_ID_MOV_TEXT,
    AV_CODEC_ID_HDMV_PGS_SUBTITLE,
    AV_CODEC_ID_DVB_TELETEXT,
    AV_CODEC_ID_SRT,
    AV_CODEC_ID_MICRODVD   = MKBETAG('m', 'D', 'V', 'D'),
    AV_CODEC_ID_EIA_608    = MKBETAG('c', '6', '0', '8'),
    AV_CODEC_ID_JACOSUB    = MKBETAG('J', 'S', 'U', 'B'),
    AV_CODEC_ID_SAMI       = MKBETAG('S', 'A', 'M', 'I'),
    AV_CODEC_ID_REALTEXT   = MKBETAG('R', 'T', 'X', 'T'),
    AV_CODEC_ID_STL        = MKBETAG('S', 'p', 'T', 'L'),
    AV_CODEC_ID_SUBVIEWER1 = MKBETAG('S', 'b', 'V', '1'),
    AV_CODEC_ID_SUBVIEWER  = MKBETAG('S', 'u', 'b', 'V'),
    AV_CODEC_ID_SUBRIP     = MKBETAG('S', 'R', 'i', 'p'),
    AV_CODEC_ID_WEBVTT     = MKBETAG('W', 'V', 'T', 'T'),
    AV_CODEC_ID_MPL2       = MKBETAG('M', 'P', 'L', '2'),
    AV_CODEC_ID_VPLAYER    = MKBETAG('V', 'P', 'l', 'r'),
    AV_CODEC_ID_PJS        = MKBETAG('P', 'h', 'J', 'S'),
    AV_CODEC_ID_ASS        = MKBETAG('A', 'S', 'S', ' '), ///< ASS as defined in Matroska

    /* other specific kind of codecs (generally used for attachments) */
    AV_CODEC_ID_FIRST_UNKNOWN = 0x18000,           ///< A dummy ID pointing at the start of various fake codecs.
    AV_CODEC_ID_TTF = 0x18000,
    AV_CODEC_ID_BINTEXT    = MKBETAG('B', 'T', 'X', 'T'),
    AV_CODEC_ID_XBIN       = MKBETAG('X', 'B', 'I', 'N'),
    AV_CODEC_ID_IDF        = MKBETAG(0, 'I', 'D', 'F'),
    AV_CODEC_ID_OTF        = MKBETAG(0, 'O', 'T', 'F'),
    AV_CODEC_ID_SMPTE_KLV  = MKBETAG('K', 'L', 'V', 'A'),
    AV_CODEC_ID_DVD_NAV    = MKBETAG('D', 'N', 'A', 'V'),
    AV_CODEC_ID_TIMED_ID3  = MKBETAG('T', 'I', 'D', '3'),
    AV_CODEC_ID_BIN_DATA   = MKBETAG('D', 'A', 'T', 'A'),


    AV_CODEC_ID_PROBE = 0x19000, ///< codec_id is not known (like AV_CODEC_ID_NONE) but lavf should attempt to identify it

    AV_CODEC_ID_MPEG2TS = 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    AV_CODEC_ID_MPEG4SYSTEMS = 0x20001, /**< _FAKE_ codec to indicate a MPEG-4 Systems
                                * stream (only used by libavformat) */
    AV_CODEC_ID_FFMETADATA = 0x21000,   ///< Dummy codec for streams containing only metadata information.

#if FF_API_CODEC_ID
#include "old_codec_ids.h"
#endif
};

typedef struct _frame_head {
    char fin;
    char opcode;
    char mask;
    unsigned long long payload_length;
    char masking_key[4];
} frame_head;

struct reco_frame_node {
    struct reco_frame frame;
    AST_LIST_ENTRY(reco_frame_node) list;
};

struct packet_data_node {
    char *data;
    int size;
    int nSendOut;;
    AST_LIST_ENTRY(packet_data_node) list;
};

//context associated with one connection
struct RTSPContext {
    RTSPSS_STATE state;
    int fd; /* socket file descriptor */

    struct sockaddr_in6 from_addr; /* origin */

    struct pollfd *poll_entry; /* used when polling */
    int64_t timeout;
    uint8_t *buffer_ptr, *buffer_end;
    int64_t start_time;            /* In milliseconds - this wraps fairly often */
    int64_t first_pts;            /* initial pts value */
    int64_t cur_pts;             /* current pts value from the stream in us */
    int64_t cur_frame_duration;  /* duration of the current frame in us */
    int cur_frame_bytes;            /* output frame size, needed to compute
                                  the time at which we send each
                                  packet */

    int  pts_stream_index;        /* stream we choose as clock reference */
    int  belongto;                  //关联的stream id
    char protocol[16];
    char method[16];
    char url[128];
    int  buffer_size;
    uint8_t *buffer;
    int packet_stream_index; /* current stream for output in state machine */

    /* RTSP state specific */
    uint8_t *pb_buffer; /* XXX: use that in all the code */
    int seq; /* RTSP sequence number */

    enum RTSPLowerTransport rtp_protocol;
    char session_id[32]; /* session id */

    /* RTP/TCP specific */
    uint8_t *packet_buffer, *packet_buffer_ptr, *packet_buffer_end;

    int  authed;
    char nonce[33];
    char resp[33];
    int  got_key_frame;
    int  exit;      //是否退出该ctx
    int  detector;  //侦测器时间
    int  ctxstart;  //开始连接时间

    /*for http send eagain */
    int eagain;
    int nHttpSendFlag;
    char reset_iframe_flag;

    int nWebSocketFlag;
    int nWebSocketHead;
    char websocket_req_key[WEBSOCKET_KEY_MAX_LEN];
    char websocket_resp_key[WEBSOCKET_KEY_MAX_LEN];
    char websocket_buffer[BUFFER_SIZE];

    /*rtsp over http*/
    int rtsp_over_http; //RTSP_OVER_HTTP_REQ
    int rtsp_tunnelled;
    char x_sessioncookie[WEBSOCKET_KEY_MAX_LEN];
    char pHttpBaseBuff[BUFFER_SIZE];

    /*http send*/
    AST_LIST_HEAD_NOLOCK(datalist, packet_data_node) datalist;
    OSA_MutexHndl data_mutex;
    int data_cnt;
    int data_size;

    int remote_permission;
    char ip[64];
    char mac[64];

    struct RTSPContext *origin;//rtp流的起源ctx
    struct RTSPContext *ms_c;
    struct RTSPContext *rtsp_c;
    AST_LIST_ENTRY(RTSPContext) list;
};

struct reco_rtsp_stream {
    char name[128];             //stream name
    int exec;
    int id;                     //stream id
    int state;                  //state
    int nb_streams;
    int bandwidth;
    int ctxcnt;                 //连接到该stream的所有ctx
    int frame_cnt;
    int video_stream_index;
    int audio_stream_index;
    int data_stream_index;

    int is_multicast;           //multicast specific
    int multicast_port;         //first port used for multicast
    int multicast_ttl;
    uint8_t reset_flag;

    //帧数据处理
    OSA_ThrHndl   tid;
    OSA_MutexHndl mutex;
    OSA_MutexHndl ctx_mutex;

    struct in_addr multicast_ip;
    struct rtsp_server_args desc;
    struct RTSPContext *ctxs[MAX_STREAM_CTX];

    AST_LIST_HEAD_NOLOCK(framelist, reco_frame_node) framelist;
    AST_LIST_ENTRY(reco_rtsp_stream) list;
};

struct reco_rtsp_handle {
    int nInit;
    void *sock;
};

static int global_init = 0;
static int send_on_key  = 0;
static int rtp_port_min = DEFAULT_RTP_PORT_MIN;
static int rtp_port_max = DEFAULT_RTP_PORT_MAX;
static int nb_max_connections = DEFAULT_MAX_CONNECTION;
static int rtsp_connection_timeout = DEFAULT_RTSP_TIMEOUT;

static int max_bandwidth = MAX_SEND_BAND_WIDTH;
static int max_conn = MAX_RTSP_SERVER_CONN;
static int global_rtsp_port = DEFAULT_RTSP_PORT;

static OSA_MutexHndl global_ctxmutex;

static struct rtsp_netinfo global_rtsp_net;
static struct rtsp_user_info g_user_info[RTSP_MAX_USER];
static struct sockaddr_in6 global_rtsp_addr;
static struct reco_rtsp_handle rtspServer;
static struct access_filter g_rtsp_access_filter;

extern CbRtspServerReq cb_rtsp_server_req;

static int global_srv_exec = 0;
static OSA_ThrHndl global_srv_tid[MAX_RTSP_THREAD];
static int g_framelist_cnt[MAX_CAMERA];
static int g_framelist_size[MAX_CAMERA];
OSA_MutexHndl g_framelist_mutex[MAX_CAMERA];

static int g_rtsp_thread[MAX_RTSP_THREAD];
typedef struct ms_frame_node {
    MS_RTP_FRAME *frame;
    void *handle;
    AST_LIST_ENTRY(ms_frame_node) list;
} MS_FRAME_NODE;
AST_LIST_HEADS_NOLOCK_STATIC(msframelist, ms_frame_node, MAX_CAMERA);

static void set_http_packet_header(RTSP_SERVER_STREAM *stream, http_frame_packet_header_t *header, struct reco_frame *frame)
{
    header->boundary = HTTP_HEADER_BOUNDARY;
    header->version = HTTP_HEADER_VERSION;
    header->header_size = sizeof(http_frame_packet_header_t);

    header->ch = frame->ch;
    header->strm_type = frame->strm_type;
    header->size = frame->size;

    if (header->strm_type == ST_VIDEO) {
        header->codec_type = frame->codec_type;
        header->frame_type = frame->frame_type;
        header->width = frame->width;
        header->height = frame->height;
        header->sample = stream->samplerate ? stream->samplerate : 8000;
        header->audio_codec = stream->originAcodeType;
    }

    if (header->strm_type == ST_AUDIO) {
        header->codec_type = stream->originVcodeType;
        header->frame_type = frame->frame_type ? frame->frame_type : 0;
        header->width = stream->width ? stream->width : 320;
        header->height = stream->height ? stream->height : 240;
        header->sample = frame->sample ? frame->sample : 8000;
        if (stream->isPlayback) {
            header->audio_codec = frame->codec_type;
        } else {
            header->audio_codec = stream->originAcodeType;
        }
    }
    
    header->time_usec = frame->time_usec;
    return ;
}

static void *ms_framelist_insert(void *hdl, struct reco_frame *frame)
{
    if (!hdl || !frame) {
        rtsp_server_log(TSAR_WARN, "framelist insert params is err.");
        return NULL;
    }
    int chnid = frame->ch;
    RTSP_SERVER_STREAM *handle = NULL;

    if (chnid < 0 || chnid >= MAX_CAMERA) {
        rtsp_server_log(TSAR_WARN, "framelist insert chnid is err.");
        return NULL;
    }

    handle = (RTSP_SERVER_STREAM *)hdl;
    if (!handle->state) {
        rtsp_server_log(TSAR_WARN, "framelist state is err.");
        return NULL;
    }

    if (g_framelist_size[chnid] + frame->size > MAX_RTSP_FRAME_BUFF) {
        //rtsp_server_log(TSAR_WARN, "chnid:%d framelist size is more than limit.", chnid);
        return NULL;
    }

    MS_FRAME_NODE *pNode = rtsp_buffer_calloc(1, sizeof(MS_FRAME_NODE));
    if (!pNode) {
        rtsp_server_log(TSAR_WARN, "framelist insert calloc is err.");
        return NULL;
    }

    pNode->handle = hdl;
    pNode->frame = (MS_RTP_FRAME *)rtsp_buffer_calloc(1, sizeof(MS_RTP_FRAME));
    if (!pNode->frame) {
        rtsp_buffer_free(pNode, sizeof(MS_FRAME_NODE));
        rtsp_server_log(TSAR_WARN, "frame insert calloc is err.");
        return NULL;
    }

    pNode->frame->chnid = frame->ch;
    pNode->frame->strmType = ms_translate_strm_type(frame->strm_type);
    pNode->frame->frameType = frame->frame_type;
    pNode->frame->time_usec = frame->time_usec;
    pNode->frame->time_upper = frame->time_upper;
    pNode->frame->time_lower = frame->time_lower;

    pNode->frame->nHttpHeaderSize = sizeof(http_frame_packet_header_t);
    pNode->frame->httpHeader = (http_frame_packet_header_t *)rtsp_buffer_calloc(1, sizeof(http_frame_packet_header_t));

#if 0
    if (frame->ch == 0 && frame->stream_format == 0) {
        rtsp_server_log(TSAR_DEBUG, "[david debug] ch:%d frameType:%d size:%d", frame->ch, pNode->frame->frameType, frame->size);
    }
#endif
    if (!pNode->frame->httpHeader) {
        rtsp_buffer_free(pNode->frame, sizeof(MS_RTP_FRAME));
        rtsp_buffer_free(pNode, sizeof(MS_FRAME_NODE));
        rtsp_server_log(TSAR_WARN, "frame httpHeader calloc is err.");
        return NULL;
    }
    set_http_packet_header(handle, pNode->frame->httpHeader, frame);

    pNode->frame->nDataSize = frame->size;
    pNode->frame->data = rtsp_buffer_malloc(pNode->frame->nDataSize);
    memcpy(pNode->frame->data, frame->data, pNode->frame->nDataSize);

    OSA_mutexLock(&g_framelist_mutex[chnid]);
    g_framelist_cnt[chnid]++;
    g_framelist_size[chnid] += pNode->frame->nDataSize;
    g_framelist_size[chnid] += pNode->frame->nHttpHeaderSize;
    AST_LIST_INSERT_TAIL(&msframelist[chnid], pNode, list);
    OSA_mutexUnlock(&g_framelist_mutex[chnid]);

    return pNode;
}

static MS_FRAME_NODE *ms_framelist_popfront(int chnid)
{
    MS_FRAME_NODE *node = NULL;
    OSA_mutexLock(&g_framelist_mutex[chnid]);
    if (AST_LIST_EMPTY(&msframelist[chnid])) {
        OSA_mutexUnlock(&g_framelist_mutex[chnid]);
        return node;
    }

    node = AST_LIST_FIRST(&msframelist[chnid]);
    if (node) {
        g_framelist_cnt[chnid]--;
        g_framelist_size[chnid] -= node->frame->nDataSize;
        g_framelist_size[chnid] -= node->frame->nHttpHeaderSize;
        AST_LIST_REMOVE(&msframelist[chnid], node, list);
    }
    OSA_mutexUnlock(&g_framelist_mutex[chnid]);

    return node;
}

static void ms_framelist_free(MS_FRAME_NODE *node)
{
    if (!node) {
        return ;
    }

    if (node->frame->data) {
        rtsp_buffer_free(node->frame->data, node->frame->nDataSize);
    }

    if (node->frame->httpHeader) {
        rtsp_buffer_free(node->frame->httpHeader, node->frame->nHttpHeaderSize);
    }

    if (node->frame) {
        rtsp_buffer_free(node->frame, sizeof(MS_RTP_FRAME));
    }

    rtsp_buffer_free(node, sizeof(MS_FRAME_NODE));
    return ;
}

static void ms_framelist_clear_ex(void *hdl)
{
    MS_FRAME_NODE *node = NULL;
    int chnid;

    for (chnid = 0; chnid < MAX_CAMERA; chnid++) {
        OSA_mutexLock(&g_framelist_mutex[chnid]);
        AST_LIST_TRAVERSE_SAFE_BEGIN(&msframelist[chnid], node, list) {
            if (!node) {
                break;
            }
            if (node->handle == hdl) {
                g_framelist_cnt[chnid]--;
                g_framelist_size[chnid] -= node->frame->nDataSize;
                g_framelist_size[chnid] -= node->frame->nHttpHeaderSize;
                AST_LIST_REMOVE(&msframelist[chnid], node, list);
                ms_framelist_free(node);
            }
        }
        AST_LIST_TRAVERSE_SAFE_END
        OSA_mutexUnlock(&g_framelist_mutex[chnid]);
    }

    return;
}

static void ms_framelist_clear()
{
    int chnid = 0;
    for (chnid = 0; chnid < MAX_CAMERA; chnid++) {
        while (1) {
            MS_FRAME_NODE *node = ms_framelist_popfront(chnid);
            if (!node) {
                break;
            }
            ms_framelist_free(node);
        }
    }

    return ;
}

static int ms_set_thread_name(const char *name)
{
    if (!name) {
        return -1;
    }
    prctl(PR_SET_NAME, name, 0, 0, 0);
    return 0;
}

static void *thread_rtsp_server_main(void *param)
{
    int chnid = 0;
    int chns = 0;
    int threadId = *(int *)param;
    char pThreadName[64] = {0};
    MS_FRAME_NODE *node = NULL;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(threadId, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("set thread affinity failed.\n");
    }

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0) {
        rtsp_server_log(TSAR_ERR, "block sigpipe error\n");
    }
    snprintf(pThreadName, sizeof(pThreadName), "rsrv_main_%d", threadId);
    ms_set_thread_name(pThreadName);

    while (global_srv_exec) {
        if (chnid == MAX_CAMERA) {
            if (chns == 0) {
                usleep(10000);
            }
            chnid = 0;
            chns = 0;
        }

        if (chnid % MAX_RTSP_THREAD != threadId) {
            chnid++;
            continue;
        }

        //pop data
        node = ms_framelist_popfront(chnid);
        if (!node) {
            chnid++;
            continue;
        }

        //Send data.
        ms_rtsp_handle_senddata(node->handle, node->frame);

        //free data
        ms_framelist_free(node);

        chns++;
        chnid++;
    }

    OSA_thrExit(0);
    return NULL;
}

static void rtsp_generate_md5_str(const unsigned char *in, int insize, char *out, int outsize)
{
    unsigned char buf[16] = {0};

    MD5(in, insize, buf);
    snprintf(out, outsize, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8],
             buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}

static void rtsp_compute_digest_response(const char *cmd, const char *username, const char *password,
                                    const char *realm, const char *url, const char *nonce, char *respbuf, int bufsize)
{
    //The "response" field is computed as:
    //md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))
    //msprintf("[david debug] cmd:%s username:%s password:%s", cmd, username, password);
    //msprintf("[david debug] realm:%s url:%s nonce:%s", realm, url, password);
    char ha1Buf[33];
    unsigned ha1DataLen = strlen(username) + 1 + strlen(realm) + 1 + strlen(password);
    unsigned char *ha1Data = (unsigned char *)malloc(ha1DataLen + 1);
    sprintf((char *)ha1Data, "%s:%s:%s", username, realm, password);
    rtsp_generate_md5_str(ha1Data, ha1DataLen, ha1Buf, sizeof(ha1Buf));
    free(ha1Data);

    unsigned ha2DataLen = strlen(cmd) + 1 + strlen(url);
    unsigned char *ha2Data = (unsigned char *)malloc(ha2DataLen + 1);
    sprintf((char *)ha2Data, "%s:%s", cmd, url);

    char ha2Buf[33];
    rtsp_generate_md5_str(ha2Data, ha2DataLen, ha2Buf, sizeof(ha2Buf));
    free(ha2Data);

    unsigned digestDataLen = 33 + strlen(nonce) + 33;
    unsigned char *digestData = (unsigned char *)malloc(digestDataLen + 1);
    sprintf((char *)digestData, "%s:%s:%s", ha1Buf, nonce, ha2Buf);
    rtsp_generate_md5_str(digestData, digestDataLen, respbuf, bufsize);

    free(digestData);
}

static int parse_authorization_header(char *buf, char *username, char *realm,
                                      char *nonce, char *uri, char *response)
{
    const char *prefix = "Digest ";
    int len = strlen(prefix);

    while (1) {
        if (*buf == '\0') {
            return -1;    // not found
        }
        if (strncasecmp(buf, prefix, len) == 0) {
            break;
        }
        ++buf;
    }

    char *fields = buf + len;
    while (*fields == ' ') {
        ++fields;
    }
    char parameter[128] = {0};
    char value[128] = {0};

    while (1) {
        value[0] = '\0';
        if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
            sscanf(fields, "%[^=]=\"\"", parameter) != 1) {
            break;
        }
        if (strcmp(parameter, "username") == 0) {
            strcpy(username, value);
        } else if (strcmp(parameter, "realm") == 0) {
            strcpy(realm, value);
        } else if (strcmp(parameter, "nonce") == 0) {
            strcpy(nonce, value);
        } else if (strcmp(parameter, "uri") == 0) {
            strcpy(uri, value);
        } else if (strcmp(parameter, "response") == 0) {
            strcpy(response, value);
        }

        fields += strlen(parameter) + 2 /*="*/ + strlen(value) + 1 /*"*/;
        while (*fields == ',' || *fields == ' ') {
            ++fields;
        }
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') {
            break;
        }
    }

    return 0;
}

static int rtsp_access_filter(const char *ip)
{
    int ret;
    char mac[64] = {0};
    char *tmpIp = (char *)ip;

    ret = check_client_allow(&g_rtsp_access_filter, tmpIp, mac, sizeof(mac));
    rtsp_server_log(TSAR_INFO, "rtsp access filter deny. ip:%s mac:%s ret:%d\n", ip, mac, ret);
    return ret;
}

static int rtsp_user_permissions(char *auth, char *nonce, char *cmd, struct MsRtspPermissions *perm)
{
    int ret = -1;
    int i;
    char tmpusername[64] = {0};
    char tmprealm[64] = {0};
    char tmpnonce[33] = {0};
    char tmpuri[128] = {0};
    char tmpresponse[33] = {0};
    char resp[33];

    if (parse_authorization_header(auth, tmpusername, tmprealm, tmpnonce, tmpuri, tmpresponse) != 0) {
        return -2;
    }
    
    if (tmpusername[0] == 0 || tmprealm[0] == 0 ||
        strcmp(tmprealm, MS_RTSP_SERVER) != 0 || tmpnonce[0] == 0 ||
        tmpuri[0] == 0 || tmpresponse[0] == 0) {
        return -3;
    }

    if (nonce && strcmp(tmpnonce, nonce) != 0) {
        return -4;
    }  

    snprintf(perm->username, sizeof(perm->username), "%s", tmpusername);
    for (i = 0; i < RTSP_MAX_USER; i++) {
        if (g_user_info[i].username[0] == '\0') {
            continue;
        }
        if (strcmp(tmpusername, g_user_info[i].username)) {
            continue;
        }
        rtsp_compute_digest_response(cmd,  g_user_info[i].username, g_user_info[i].password_ex, tmprealm, tmpuri, tmpnonce, resp, sizeof(resp));
        if (!strcmp(resp, tmpresponse)) {
            ret = 0;
        } else {
            rtsp_compute_digest_response(cmd,  g_user_info[i].username, g_user_info[i].password, tmprealm, tmpuri, tmpnonce, resp, sizeof(resp));
            if (!strcmp(resp, tmpresponse)) {
                ret = 0;
            }
        }

        if (!ret) {
            perm->userLevel = g_user_info[i].type;
            snprintf(perm->liveviewMask, sizeof(perm->liveviewMask), "%s", g_user_info[i].remoteLiveviewChannel);
            perm->liveviewAudio = g_user_info[i].remoteLiveviewAudio;
            snprintf(perm->playbackMask, sizeof(perm->playbackMask), "%s", g_user_info[i].remotePlaybackChannel);
            perm->playbackAudio = g_user_info[i].remotePlaybackAudio;
        }
        break;
    }
    
    return ret;
}

void reco_rtsp_server_init()
{
    if (global_init) {
        return;
    }

    int i = 0;
    global_init = 1;
    struct rtsp_msstream_conf conf;

    OSA_mutexCreate(&global_ctxmutex);
    for (i = 0; i < MAX_CAMERA; i++) {
        g_framelist_cnt[i] = 0;
        g_framelist_size[i] = 0;
        OSA_mutexCreate(&g_framelist_mutex[i]);
    }
    memset(&g_user_info, 0, sizeof(struct rtsp_user_info)*RTSP_MAX_USER);
    memset(&rtspServer, 0, sizeof(struct reco_rtsp_handle));
    memset(&conf, 0, sizeof(struct rtsp_msstream_conf));
    conf.port = global_rtsp_port;
    conf.bandwidth = max_bandwidth;
    conf.ms_rp_malloc = msrtsp_buffer_malloc;
    conf.ms_rp_calloc = msrtsp_buffer_calloc;
    conf.ms_rp_free = msrtsp_buffer_free;
    conf.ms_rp_permissions = rtsp_user_permissions;
    conf.ms_rp_access_filter = rtsp_access_filter;
    conf.ms_rp_print = reco_rtsp_debug;
    
    rtspServer.sock = create_rtsp_server_task(&conf);
    if (rtspServer.sock) {
        rtspServer.nInit = 1;
    }

    global_srv_exec = 1;
    for (i = 0; i < MAX_RTSP_THREAD; i++) {
        g_rtsp_thread[i] = i;
        OSA_thrCreate(&global_srv_tid[i], thread_rtsp_server_main, 1, 32 * 1024, &g_rtsp_thread[i]);
    }

    return ;
}

void reco_rtsp_server_uninit()
{
    //退出线程
    if (!global_init) {
        return;
    }

    int i = 0;
    global_init = 0;

    if (global_srv_exec) {
        global_srv_exec = 0;

        for (i = 0; i < MAX_RTSP_THREAD; i++) {
            OSA_thrJoin(&global_srv_tid[i]);
        }
    }

    ms_framelist_clear();

    if (rtspServer.nInit) {
        rtspServer.nInit = 0;
        destory_rtsp_server_task(rtspServer.sock);
    }

    rtsp_buffer_uninit();
    for (i = 0; i < MAX_CAMERA; i++) {
        OSA_mutexDelete(&g_framelist_mutex[i]);
    }
    OSA_mutexDelete(&global_ctxmutex);

    return ;
}

void reco_rtsp_server_set(struct reco_rtsp_server_conf *conf)
{
    if (conf == NULL) {
        rtsp_server_log(TSAR_ERR, "rtp conf is null !!!");
        return;
    }

    memset(&g_user_info, 0, sizeof(struct rtsp_user_info)*RTSP_MAX_USER);
    memset(global_rtsp_addr.sin6_addr.s6_addr, 0, sizeof(global_rtsp_addr.sin6_addr.s6_addr));

    send_on_key = 0;
    rtp_port_min = DEFAULT_RTP_PORT_MIN;
    rtp_port_max = DEFAULT_RTP_PORT_MAX;
    global_rtsp_addr.sin6_port = ntohs(DEFAULT_RTSP_PORT);
    nb_max_connections = DEFAULT_MAX_CONNECTION;
    rtsp_connection_timeout = DEFAULT_RTSP_TIMEOUT;

    memcpy(&global_rtsp_net, &conf->rtsp_net, sizeof(struct rtsp_netinfo));

    if (conf->bindaddr[0] && inet_addr(conf->bindaddr) != INADDR_NONE) {
        inet_pton(AF_INET6, conf->bindaddr, &global_rtsp_addr.sin6_addr);
    }

    if (conf->server_port > 0 && conf->server_port < 65536) {
        global_rtsp_port = conf->server_port;
        global_rtsp_addr.sin6_port = ntohs(conf->server_port);
    }

    if (conf->max_connections > 0) {
        nb_max_connections = conf->max_connections;
    }

    if (conf->connection_timeout > 0) {
        rtsp_connection_timeout = conf->connection_timeout;
    }

    send_on_key = conf->send_on_key;
    if (conf->rtp_port_min > 0 && conf->rtp_port_min < conf->rtp_port_max) {
        rtp_port_min = conf->rtp_port_min;
    }

    if (conf->rtp_port_max > conf->rtp_port_min && conf->rtp_port_max < 65536) {
        rtp_port_max = conf->rtp_port_max;
    }

    if (conf->max_bitrate > 0) {
        max_bandwidth = conf->max_bitrate;
    }

    if (conf->max_conn > 0) {
        max_conn = conf->max_conn;
    }

    cb_rtsp_server_req = (CbRtspServerReq)conf->cb_user_req;

    rtsp_buffer_init(max_bandwidth);
    reco_rtsp_server_init();

    return ;
}

int reco_rtsp_server_set_user(struct rtsp_user_info user_info[RTSP_MAX_USER])
{
    OSA_mutexLock(&global_ctxmutex);
    memcpy(g_user_info, user_info, sizeof(struct rtsp_user_info)*RTSP_MAX_USER);
    OSA_mutexUnlock(&global_ctxmutex);

    return 0;
}

void *reco_rtsp_server_create(struct rtsp_server_args *desc)
{
    if (!desc) {
        return NULL;
    }
    static int id = 0;
    RTSP_SERVER_STREAM node;

    memset(&node, 0, sizeof(RTSP_SERVER_STREAM));
    snprintf(node.name, sizeof(node.name), "%s", desc->name);

    node.id = id++;
    node.nInit = 1;
    node.state = 1;
    ms_com_mutex_init(&node.hdlMutex);

    node.chnid = desc->chnid;
    node.originVcodeType = desc->codec;
    node.vCodeType = ms_translate_forward(ST_VIDEO, desc->codec);
    node.bandWidth = desc->bandwidth;
    node.width = desc->width;
    node.height = desc->height;
    node.nb_streams |= 1 << DEFAULT_STREAM_VID;

    node.originAcodeType = desc->acodec;
    node.aCodeType = ms_translate_forward(ST_AUDIO, desc->acodec);
    node.samplerate = desc->samplerate;
    if (node.aCodeType >= 0 && node.samplerate > 0) {
        node.nb_streams |= 1 << DEFAULT_STREAM_AUD;
    }

    node.exSize = desc->extradata_size;
    if (node.exSize > sizeof(node.extraData)) {
        node.exSize = sizeof(node.extraData);
    }
    memcpy(node.extraData, desc->extradata, node.exSize);

    node.nb_streams |= 1 << DEFAULT_STREAM_MED;
    node.ctxCnts = 0;
    node.arg = desc->arg;
    node.cb_server_state = desc->cb_server_state;

    node.maxDataSize = desc->maxDataSize;

    if (strstr(desc->name, "pb_")) {
        node.isPlayback = 1;
    } else {
        node.isPlayback = 0;
    }
    
    return ms_streamlist_insert(&node);
}

int reco_rtsp_server_destroy(void *hdl)
{
    RTSP_SERVER_STREAM *stream;
    
    if (!hdl) {
        return -1;
    }
    stream = (RTSP_SERVER_STREAM *)hdl;
    stream->state = 0;
    ms_framelist_clear_ex(hdl);
    ms_streamlist_exchange(stream);
    return 0;
}

int reco_rtsp_server_senddata(void *hdl, struct reco_frame *frame)
{
    if (!global_init) {
        return -1;
    }

    ms_framelist_insert(hdl, frame);

    return 0;
}

void reco_rtsp_server_showstreams()
{
    int chnid = 0;
    int allframesize = 0;

    for (chnid = 0; chnid < MAX_CAMERA; chnid++) {
        if (!g_framelist_cnt[chnid]) {
            continue;
        }
        allframesize += g_framelist_size[chnid];
        rtsp_server_log(TSAR_DEBUG, "[david debug] chnid:%d framelist cnt:%d size:%d", chnid, g_framelist_cnt[chnid] , g_framelist_size[chnid]);
    }
    rtsp_server_log(TSAR_DEBUG, "[david debug] all frame size:%d", allframesize);
    
    ms_streamlist_show();
    rtsp_buffer_show();

    return ;
}

int reco_rtsp_server_ctx_count(void *hdl)
{
    if (hdl == NULL) {
        return 1;
    }

    return 0;
}

int reco_rtsp_update_access_filter(struct access_filter *conf)
{
    memcpy(&g_rtsp_access_filter, conf, sizeof(struct access_filter));
    ms_rtsp_update_filter();
    
    return 0;
}

void reco_rtsp_server_show_channel(char *name)
{
    ms_stream_channel_show(name);
    return ;
}

void reco_rtsp_set_multicast(int enable, char *ip)
{
    return ms_rtsp_set_multicast(enable, ip);
}

int reco_rtsp_server_set_bandwidth(void *hdl, int bandwidth)
{
    RTSP_SERVER_STREAM *stream = (RTSP_SERVER_STREAM *)hdl;
    return ms_rtsp_server_update_bandwidth(stream, bandwidth);
}
