һ����װemcc��������Ҫ64λubuntuϵͳ
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

��������ffmpeg
ffmpegԴ��ֱ�Ӵ�git�ϵ�����Ŀǰ�õ���release/4.4�汾

CPPFLAGS="-D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600" \
emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --prefix=/usr/hrz/aswm/demo/webfile/js/ffmpeg-wasm/dist --enable-cross-compile --target-os=none --arch=x86_64 --cpu=generic --enable-gpl --enable-version3 --disable-avdevice --disable-swresample --disable-postproc --disable-avfilter --disable-avformat --disable-parsers --disable-everything --disable-programs --disable-ffplay --disable-ffprobe --disable-asm --disable-doc --disable-devices --disable-hwaccels --disable-bsfs --disable-debug --disable-iconv --disable-xlib --disable-zlib --disable-sdl2 --disable-bzlib --enable-small --disable-indevs --disable-outdevs --disable-encoders --disable-decoders --disable-decoder=h263 --enable-ffmpeg --enable-static --disable-shared --enable-lto  --enable-decoder=pcm_mulaw --enable-decoder=pcm_alaw --enable-decoder=adpcm_ima_smjpeg --enable-decoder=aac --enable-decoder=hevc --enable-decoder=h264 --enable-avformat --disable-pthreads
make && make install

������������wasm�ļ�
ִ��build_decoder_wasm.sh�ű���ע��ffmepg�������·����Ӧ


�ġ�����ļ�˵��
1��disk
����ffmpeg�����ɵ������⣬
dist\include\linkedlists.h �Ǵ�mshn��֧�ӿ��������ģ��ṩ��������������ؽӿ�

2��build_decoder_wasm.sh
�ű��ļ���һ��emcc����libffmpeg.js��libffmpeg.wasm

3��decoder.c
��������wasm�ļ��Ķ�Ӧc���Դ��룬��Ҫ����ֲ�������h265�Ľӿ�

4��ffmpegԴ��ֱ�Ӵ�git�ϵ���������û�ж�Դ�����޸ģ���˲��ϴ�Դ�룬Ŀǰ�õ���release/4.4�汾

5��libffmpeg.js  libffmpeg.wasm
emcc�����ɵ�ǰ���ļ�
