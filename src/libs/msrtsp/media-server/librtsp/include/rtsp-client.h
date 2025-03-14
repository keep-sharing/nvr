#ifndef _rtsp_client_h_
#define _rtsp_client_h_

#include "rtsp-header-transport.h"
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum rtsp_state_t
{
    RTSP_INIT = 0,
    RTSP_ANNOUNCE = 1,
    RTSP_RECORD = 2,
    RTSP_DESCRIBE = 3,
    RTSP_SETUP = 4,
    RTSP_PLAY = 5,
    RTSP_PAUSE = 6,
    RTSP_TEARDWON = 7,
    RTSP_OPTIONS = 8,
    RTSP_GET_PARAMETER = 9,
    RTSP_SET_PARAMETER = 10,
};

typedef struct rtsp_client_t rtsp_client_t;

// seq=232433;rtptime=972948234
struct rtsp_rtp_info_t
{
	const char* uri;
	uint32_t seq;	// uint16_t
	uint32_t time;	// uint32_t
};

struct rtsp_client_handler_t
{
	///network implementation
	///@return >0-sent bytes, <0-error
	int (*send)(void* param, const char* uri, const void* req, size_t bytes);
	///create rtp/rtcp port 
	int (*rtpport)(void* param, int media, unsigned short *rtp); // udp only(rtp%2=0 and rtcp=rtp+1), rtp=0 if you want to use RTP over RTSP(tcp mode)

	/// rtsp_client_announce callback only
	int (*onannounce)(void* param);
	/// option reply
	int (*onoption)(void* param);
	/// call rtsp_client_setup
	int (*ondescribe)(void* param, const char* sdp);

	int (*onsetup)(void* param);
	int (*onplay)(void* param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale, const struct rtsp_rtp_info_t* rtpinfo, int count); // play
    int (*onrecord)(void* param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale, const struct rtsp_rtp_info_t* rtpinfo, int count); // record
	int (*onpause)(void* param);
	int (*onteardown)(void* param);

	void (*onrtp)(void* param, uint8_t channel, const void* data, uint16_t bytes);
};

/// @param[in] param user-defined parameter
/// @param[in] usr RTSP auth username(optional)
/// @param[in] pwd RTSP auth password(optional)
rtsp_client_t* rtsp_client_create(const char* uri, const char* usr, const char* pwd, const struct rtsp_client_handler_t *handler, void* param);

void rtsp_client_destroy(rtsp_client_t* rtsp);

/// input server reply
/// @param[in] data server response message
/// @param[in] bytes data length in byte
int rtsp_client_input(rtsp_client_t* rtsp, const void* data, size_t bytes, int *dosleep, uint64_t *timeout);

/// find RTSP response header
/// @param[in] name header name
/// @return header value, NULL if not found.
/// NOTICE: call in rtsp_client_handler_t callback only
const char* rtsp_client_get_header(rtsp_client_t* rtsp, const char* name);
/// rtsp options (optional)
int rtsp_client_options(struct rtsp_client_t *rtsp, const char* commands);

/// rtsp get_parameter (optional)
int rtsp_client_get_parameter(struct rtsp_client_t *rtsp, int media, const char* parameter);

/// rtsp describe (optional)
int rtsp_client_describe(struct rtsp_client_t* rtsp);

/// rtsp setup
/// @param[in] uri media resource uri
/// @param[in] sdp resource info. it can be null, sdp will get by describe command
/// @return 0-ok, -EACCESS-auth required, try again, other-error.
int rtsp_client_setup(rtsp_client_t* rtsp, const char* sdp);

/// stop and close session(TearDown)
/// call onclose on done
/// @return 0-ok, other-error.
int rtsp_client_teardown(rtsp_client_t* rtsp);

/// play session(PLAY)
/// call onplay on done
/// @param[in] npt PLAY range parameter [optional, NULL is acceptable]
/// @param[in] speed PLAY scale+speed parameter [optional, NULL is acceptable]
/// @return 0-ok, other-error.
/// Notice: if npt and speed is null, resume play only
int rtsp_client_play(rtsp_client_t* rtsp, const uint64_t *npt, const float *speed);

/// pause session(PAUSE)
/// call onpause on done
/// @return 0-ok, other-error.
/// use rtsp_client_play(rtsp, NULL, NULL) to resume play
int rtsp_client_pause(rtsp_client_t* rtsp);

/// announce server sdp
/// @return 0-ok, other-error.
int rtsp_client_announce(rtsp_client_t* rtsp, const char* sdp);

/// record session(publish)
/// call onrecord on done
/// @param[in] npt RECORD range parameter [optional, NULL is acceptable]
/// @param[in] scale RECORD scale parameter [optional, NULL is acceptable]
/// @return 0-ok, other-error.
/// Notice: if npt and scale is null, resume record only
int rtsp_client_record(struct rtsp_client_t *rtsp, const uint64_t *npt, const float *scale);

/// SDP API
int rtsp_client_media_count(rtsp_client_t* rtsp);
const struct rtsp_header_transport_t* rtsp_client_get_media_transport(rtsp_client_t* rtsp, int media);
const char* rtsp_client_get_media_encoding(rtsp_client_t* rtsp, int media);
const char* rtsp_client_get_media_fmtp(rtsp_client_t* rtsp, int media);
int rtsp_client_get_media_payload(rtsp_client_t* rtsp, int media);
int rtsp_client_get_media_rate(rtsp_client_t* rtsp, int media); // return 0 if unknown rate
const char* rtsp_client_get_media_describption(struct rtsp_client_t *rtsp, int media);
int rtsp_client_get_media_extradata(struct rtsp_client_t *rtsp, int media, char *dst, int dstlen);

uint32_t rtsp_client_get_methodmask(struct rtsp_client_t *rtsp);
int rtsp_client_over_https_data(struct rtsp_client_t *rtsp, const char* host, const char* x_session, const char* path);
int rtsp_client_over_https_command(struct rtsp_client_t *rtsp, const char* host, const char* x_session, const char* path);

#if defined(__cplusplus)
}
#endif
#endif /* !_rtsp_client_h_ */
