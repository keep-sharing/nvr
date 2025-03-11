一、安装emcc环境，需要64位ubuntu系统
# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk

# Fetch the latest version of the emsdk (not needed the first time you clone)
git pull

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
./emsdk activate latest

# Activate PATH and other environment variables in the current terminal
source ./emsdk_env.sh

二、编译ffmpeg
ffmpeg源码直接从git上导出，目前用的是release/4.4版本

CPPFLAGS="-D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600" \
emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --prefix=/usr/hrz/aswm/demo/webfile/js/ffmpeg-wasm/dist --enable-cross-compile --target-os=none --arch=x86_64 --cpu=generic --enable-gpl --enable-version3 --disable-avdevice --disable-swresample --disable-postproc --disable-avfilter --disable-avformat --disable-parsers --disable-everything --disable-programs --disable-ffplay --disable-ffprobe --disable-asm --disable-doc --disable-devices --disable-hwaccels --disable-bsfs --disable-debug --disable-iconv --disable-xlib --disable-zlib --disable-sdl2 --disable-bzlib --enable-small --disable-indevs --disable-outdevs --disable-encoders --disable-decoders --disable-decoder=h263 --enable-ffmpeg --enable-static --disable-shared --enable-lto  --enable-decoder=pcm_mulaw --enable-decoder=pcm_alaw --enable-decoder=adpcm_ima_smjpeg --enable-decoder=aac --enable-decoder=hevc --enable-decoder=h264 --enable-avformat --disable-pthreads
make && make install

三、编译生成wasm文件
执行build_decoder_wasm.sh脚本，注意ffmepg依赖库的路径对应


四、相关文件说明
1、disk
编译ffmpeg后生成的依赖库，
dist\include\linkedlists.h 是从mshn分支从拷贝过来的，提供解码所需链表相关接口

2、build_decoder_wasm.sh
脚本文件，一键emcc生成libffmpeg.js和libffmpeg.wasm

3、decoder.c
用于生成wasm文件的对应c语言代码，主要是移植插件解码h265的接口

4、ffmpeg源码直接从git上导出，由于没有对源码做修改，因此不上传源码，目前用的是release/4.4版本

5、libffmpeg.js  libffmpeg.wasm
emcc后生成的前端文件
