#ifndef LIBXD_XD_COMMON_H
#define LIBXD_XD_COMMON_H

#include <stdarg.h>
#include <stddef.h>
#if defined(HAVE_STDINT_H) || __GNUC__ >= 4 || _MSC_VER > 1500
    #include <stdint.h>  // for int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t;
#else
    #if defined(_MSC_VER)
        typedef signed __int8       int8_t;
        typedef unsigned __int8     uint8_t;
        typedef signed __int16      int16_t;
        typedef unsigned __int16    uint16_t;
        typedef signed __int32      int32_t;
        typedef unsigned __int32    uint32_t;
        typedef signed __int64      int64_t;
        typedef unsigned __int64    uint64_t;
    #else
    #endif
#endif

#if 0
    #define VLD_FORCE_ENABLE
    #include <vld.h>
#endif


///
/// @{
///

/// @defgroup xdcommon-changes 变更日志
/// @{
///
/// @par 3.1.18 (2014/3/21)
///   - 增加 BLOCK_FLAG_AUDIO_FRAME 及 BLOCK_FLAG_VIDEO_FRAME 定义
///
/// @par 3.1.17 (2014/2/21)
///   - 增加 BLOCK_FLAG_FRAGMENTIZED 及 BLOCK_FLAG_BEGIN_OF_FRAME 定义
///
/// @par 3.1.16 (2014/01/13)
///   - 增加 AV_FOURCC_XDMI 宏
///
/// @par 3.1.15 (2013/10/29)
///   - 增加 PCM a-law/u-law, PCM 16 FourCC
///
/// @par 3.1.14 (2013/7/25)
///   - 接口增加，无法与 v3.0 版二进制文件保持兼容性，估子版本号升级
///   - 增加网络接口参数获取及设定系列接口
///
/// @par 3.0.13 (2013/7/15)
///   - 修正 XD_StreamBlockDuplicate() 容量检测的兼容性问题
///
/// @par 3.0.12 (2013/2/2)
///   - Windows 平台下日志文件修改为以 _SH_DENYNO 方式打开
///
/// @par 3.0.11 (2013/2/1)
///   - 修正创建日志文件失败时导致程序崩溃的问题
///
/// @par 3.0.10 (2013/1/30)
///   - 增加 #XD_LogOnlyLocalTime, #XD_LogLocalDateTime 由于枚举编号已发生变化，所以用 <= 3.0.9 版的程序需要重新编译
///   - 增加 XD_LogSetEOL(), XD_LogGetEOL()
///   - 修改内置换行符格式，Windows 下默认为: "r\n", Linux 下默认为: "\n"
///
/// @par 3.0.9 (2013/1/29)
///   - 修正 XD_StreamBlockDuplicate() 可能会导致程序崩溃的问题
///
/// @par 3.0.8 (2013/1/27)
///   - 修改 XD_Log() 线程安全机制，现在默认开启线程安全
///
/// @par 3.0.7 (2013/1/21)
///   - 增加 XD_StrDup()
///   - 增加 XD_LogSetOutputFileW(), XD_LogGetOutputFileW()
///   - 增加 XD_NetProbeHwAddrByIPv4(), XD_NetProbeHwAddrByIPv4W()
///   - 增加 XD_NetProbeHwAddrByIPv6(), XD_NetProbeHwAddrByIPv6W()
///   - 增加 #XD_STREAM_BLOCK_MAX_SIZE
///   - XD_StreamBlockAlloc(), XD_StreamBlockRealloc() 将不允许创建大于 #XD_STREAM_BLOCK_MAX_SIZE 的数据块
///   - XD_StreamBlockAlloc(), XD_StreamBlockRealloc(), XD_StreamBlockAppend() 出错时将会有日志信息输出
///   - 修改 MAKE_FOURCC() 为 XD_MakeFourCC()
///
/// @par 3.0.6 (2013/1/19)
///   - 增加 XD_FileZip(), XD_FileUnzip()
///   - 增加 XD_LogSetAutoArchive(), XD_LogGetAutoArchive()
///   - 增加 XD_LogSetMaxArchives(), XD_LogGetMaxArchives()
///   - 增加 XD_LogSetAutoRotate(), XD_LogGetAutoRotate()
///   - 增加 XD_LogSetRotateSize(), XD_LogGetRotateSize()
///   - 增加 XD_LogFlush()
///
/// @par 3.0.5 (2013/1/18)
///   - 修正 msvc2008 支持
///
/// @par 3.0.4 (2013/1/16)
///   - 修改 XD_GetFileSystemSpaceInfoA() 为 XD_FileSystemGetSpaceInfoA()
///   - 修改 XD_GetFileSystemSpaceInfoW() 为 XD_FileSystemGetSpaceInfoW()
///   - 修改 XD_GetFileSystemSpaceInfoU8() 为 XD_FileSystemGetSpaceInfoU8()
///   - 废弃 XD_GetFileSystemSpaceInfoA(), XD_GetFileSystemSpaceInfoW(), XD_GetFileSystemSpaceInfoU8()
///
/// @par 3.0.3 (2013/1/14)
///   - 增加 \ref xdcommon-log 接口
///
/// @par 3.0.2 (2013/1/11)
///   - 增加 XD_GetFileSystemSpaceInfoA(), XD_GetFileSystemSpaceInfoW(), XD_GetFileSystemSpaceInfoU8() 三个用于获取文件系统空间信息的接口
///
/// @par 3.0.1 (2013/1/10)
///   - 增加 XD_DbgPrintf() 调试信息打印宏函数
///   - 增加 XD_PrintfA(), XD_PrintfW() 打印函数用于向控制台或调试信息缓冲区输出内容
///
/// @par 3.0 (2013/1/6)
///   - 初始版本建立
///
/// @}
///

/// @addtogroup xdcommon-macros 宏定义
/// @{
///

#if defined(_MSC_VER) && !defined(XD_STATIC_LIB)
    #ifdef XD_EXPORTS
        #define XD_API __declspec(dllexport)
    #else
        #define XD_API __declspec(dllimport)
    #endif
#else
    #define XD_API
#endif

#define XD_DECL ///< 函数调用方式

#if defined(_MSC_VER)
    #define XD_DEPRECATED __declspec(deprecated) ///< 已经废弃的接口
#else
    #define XD_DEPRECATED __attribute__((deprecated)) ///< 已经废弃的接口
#endif

#define XD_MakeFourCC( a, b, c, d ) \
    ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
      | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )

#define AV_FOURCC_NULL  XD_MakeFourCC('N', 'U', 'L', 'L')
#define AV_FOURCC_AAC   XD_MakeFourCC('m', 'p', '4', 'a')
#define AV_FOURCC_JPEG  XD_MakeFourCC('J', 'P', 'E', 'G')
#define AV_FOURCC_H264  XD_MakeFourCC('a', 'v', 'c', '1')
#define AV_FOURCC_HKMI  XD_MakeFourCC('H', 'K', 'M', 'I')
#define AV_FOURCC_MP2P  XD_MakeFourCC('M', 'P', '2', 'P')
#define AV_FOURCC_MP2T  XD_MakeFourCC('M', 'P', '2', 'T')
#define AV_FOURCC_PCMA  XD_MakeFourCC('P', 'C', 'M', 'A')
#define AV_FOURCC_PCMU  XD_MakeFourCC('P', 'C', 'M', 'U')
#define AV_FOURCC_S16B  XD_MakeFourCC('S', '1', '6', 'B')
#define AV_FOURCC_S16L  XD_MakeFourCC('S', '1', '6', 'L')
#define AV_FOURCC_XDMI  XD_MakeFourCC('X', 'D', 'M', 'I')
#define AV_FOURCC_H265  XD_MakeFourCC('H', '2', '6', '5')

/// 将数值 v 向上对齐到 x
#define XD_AlignedUpTo(v, x)  (((v)+((x)-1)) & (~((x)-1)))



/// 生成 32 位版本号
#define XD_MakeVersion(a, b, c) (a<<24|b<<16|c)

/// 当前定义版本号
#define LIBXD_COMMON_VERSION XD_MakeVersion(3,1,18)

/// 定义流数据块最大尺寸
#define XD_STREAM_BLOCK_MAX_SIZE 16777216

/// @}
///

#ifdef __cplusplus
extern "C" {
#endif



/// @addtogroup xdcommon-structs 数据结构
/// @{
///

#ifndef XD_STREAMBLOCK_TYPE_DEFINED
#define XD_STREAMBLOCK_TYPE_DEFINED
#define BLOCK_FLAG_DISCONTINUITY        0x0001      ///< 非连续块
#define BLOCK_FLAG_TYPE_I               0x0002      ///< I帧
#define BLOCK_FLAG_TYPE_P               0x0004      ///< P帧
#define BLOCK_FLAG_TYPE_B               0x0008      ///< B帧
#define BLOCK_FLAG_TYPE_PB              0x0010      ///< P/B帧
#define BLOCK_FLAG_HEADER               0x0020      ///< 含有头部信息
#define BLOCK_FLAG_END_OF_FRAME         0x0040      ///< 帧结束
#define BLOCK_FLAG_NO_KEYFRAME          0x0080      ///< 非关键帧
#define BLOCK_FLAG_END_OF_SEQUENCE      0x0100      ///< 序列结束
#define BLOCK_FLAG_CLOCK                0x0200      ///< 参考时钟
#define BLOCK_FLAG_SCRAMBLED            0x0400      ///< 已加扰或受扰
#define BLOCK_FLAG_PREROLL              0x0800      ///< 已经解码但未显示
#define BLOCK_FLAG_CORRUPTED            0x1000      ///< 数据损坏或丢失
#define BLOCK_FLAG_TOP_FIELD_FIRST      0x2000      ///< 顶场在前
#define BLOCK_FLAG_BOTTOM_FIELD_FIRST   0x4000      ///< 底场在先
#define BLOCK_FLAG_FRAGMENTIZED         0x8000      ///< 碎片化的数据，非完整帧
#define BLOCK_FLAG_BEGIN_OF_FRAME       0x10000     ///< 帧起始
#define BLOCK_FLAG_AUDIO_FRAME          0x20000     ///< 音频帧
#define BLOCK_FLAG_VIDEO_FRAME          0x40000     ///< 视频帧

// 前置声明
/// 数据流输出的结构体
struct XD_StreamBlock;
typedef struct XD_StreamBlock XD_StreamBlock;

/// 流数据块资源释放回调函数
/// @param [in] block       已分配的流数据块指针
/// @return 无返回值.
typedef void (*XD_StreamBlockDeallocator)(XD_StreamBlock *block);

/// 流数据块
#pragma pack(8)
struct XD_StreamBlock {
    XD_StreamBlock *p_next;         ///< 后续数据块

    uint8_t        *p_buffer;       ///< 负载数据起始位置
    uint32_t        i_buffer;       ///< 负载数据长度

    uint8_t        *p_start;        ///< 缓冲区起始位置
    uint32_t        i_size;         ///< 缓冲区长度

    uint32_t        i_flags;        ///< 数据包类型，是I帧，P帧
    uint32_t        i_nb_samples;   ///< 音频样本数量
    uint32_t        i_track;        ///< 数据轨道
    uint32_t        i_codec;        ///< 编码类型FOURCC('a','b','c','d')

    int64_t         i_pts;          ///< 数据包时戳
    int64_t         i_dts;          ///< 解码时戳
    int64_t         i_length;       ///< 持续时长
    XD_StreamBlockDeallocator pf_release;   ///< 数据块释放回调
};
#pragma pack()
#endif // XD_STREAMBLOCK_TYPE_DEFINED
/// @}
///



#ifdef __cplusplus
}
#endif

/// @}
///

#endif // LIBXD_XD_COMMON_H
