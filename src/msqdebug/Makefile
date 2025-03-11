PROJECT_DIR := $(shell pwd)/../../

msqdebug:
	@echo "----hi3536c----"
	-mkdir -p ../build-msqdebug-hi3536c
	cd ../build-msqdebug-hi3536c && $(PROJECT_DIR)/tools/hi3536c/QTLibs/bin/qmake ../msqdebug/msqdebug.pro -spec $(PROJECT_DIR)/tools/hi3536c/QTLibs/mkspecs/qws/linux-arm-his-g++
	cd ../build-msqdebug-hi3536c && make -j4
	cp ../build-msqdebug-hi3536c/msqdebug $(PROJECT_DIR)/targets/hi3536c/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3536c/app/bin/
	@echo "----hi3536g----"
	-mkdir -p ../build-msqdebug-hi3536g
	cd ../build-msqdebug-hi3536g && $(PROJECT_DIR)/tools/hi3536g/QTLibs/bin/qmake ../msqdebug/msqdebug.pro -spec $(PROJECT_DIR)/tools/hi3536g/QTLibs/mkspecs/qws/linux-arm-his-g++
	cd ../build-msqdebug-hi3536g && make -j4
	cp ../build-msqdebug-hi3536g/msqdebug $(PROJECT_DIR)/targets/hi3536g/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3536g/app/bin/
	@echo "----hi3798----"
	-mkdir -p ../build-msqdebug-hi3798
	cd ../build-msqdebug-hi3798 && $(PROJECT_DIR)/tools/hi3798/QTLibs/bin/qmake ../msqdebug/msqdebug.pro -spec $(PROJECT_DIR)/tools/hi3798/QTLibs/mkspecs/qws/linux-arm-his-g++
	cd ../build-msqdebug-hi3798 && make -j4
	cp ../build-msqdebug-hi3798/msqdebug $(PROJECT_DIR)/targets/hi3798/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3798/app/bin/
	@echo "----nt98323----"
	-mkdir -p ../build-msqdebug-nt98323
	cd ../build-msqdebug-nt98323 && $(PROJECT_DIR)/tools/nt98323/QTLibs/bin/qmake ../msqdebug/msqdebug.pro -spec $(PROJECT_DIR)/tools/nt98323/QTLibs/mkspecs/qws/linux-arm-his-g++
	cd ../build-msqdebug-nt98323 && make -j4
	cp ../build-msqdebug-nt98323/msqdebug $(PROJECT_DIR)/targets/nt98323/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/nt98323/app/bin/

install:
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3536c/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3536g/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/hi3798/app/bin/
	cp msqdebug.help $(PROJECT_DIR)/targets/nt98323/app/bin/
	
clean:
	cd ../build-msqdebug-hi3536c && make clean
	cd ../build-msqdebug-hi3536g && make clean
	cd ../build-msqdebug-hi3798 && make clean
