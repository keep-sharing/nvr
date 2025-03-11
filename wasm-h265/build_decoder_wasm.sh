rm libffmpeg.wasm libffmpeg.js
export TOTAL_MEMORY=67108864
export EXPORTED_FUNCTIONS="[ \
    '_initDecoder', \
    '_uninitDecoder', \
    '_sendData', \
    '_decodeOnePacket', \
	'_setDecoderLimit', \
    '_main',
    '_malloc',
    '_free'
]"

echo "Running Emscripten..."
emcc decoder.c dist/lib/libavformat.a dist/lib/libavcodec.a dist/lib/libavutil.a dist/lib/libswscale.a \
    -O3 \
    -I "dist/include" \
    -s WASM=1 \
    -s TOTAL_MEMORY=${TOTAL_MEMORY} \
    -s EXPORTED_FUNCTIONS="${EXPORTED_FUNCTIONS}" \
    -s EXPORTED_RUNTIME_METHODS="['addFunction']" \
    -s RESERVED_FUNCTION_POINTERS=14 \
    -s ALLOW_MEMORY_GROWTH=1 \
	-s ASSERTIONS=1 \
    -o libffmpeg.js

echo "Finished Build"
