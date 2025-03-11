#!/bin/bash

start_time=$(date +%s)

ROOT="/share"
VERSION=$(cat src/msgui/mscore/mssysconf.h | sed -n '/MS_HI_NVR_VERSION/s#.*"\(.*\)"#\1#p')
TIME=$(date "+%Y%m%d_%H%M%S")
PLATFORM=$1

TARGET="$ROOT/${TIME}_${VERSION}"

for PLATFORM in nt98323 hi3536c hi3536g
do
	echo "make $PLATFORM"
	mkdir -p $TARGET/$PLATFORM/symbols
	make $PLATFORM && make distclean && make gui-clean
	make apps && make gui && make all
	cp ./targets/$PLATFORM/firmware/*$VERSION $TARGET/$PLATFORM

	echo "wait for backup symbols..."
	find temp/$PLATFORM -name "*.so" | xargs -n1 -I {} cp {} $TARGET/$PLATFORM/symbols
	cp temp/$PLATFORM/gui/mscore $TARGET/$PLATFORM/symbols
	cd $TARGET/$PLATFORM
	tar -zcf symbols.tar.gz symbols
	rm -r $TARGET/$PLATFORM/symbols
	cd -
done

end_time=$(date +%s)
cost_time=$[ $end_time - $start_time ]
echo "build $1 took $(($cost_time/60))min $(($cost_time%60))s"
echo "build finished: $TARGET"
