gui:
	$(QT_PATH)/bin/qmake ./msgui.pro -spec $(QT_PATH)/mkspecs/qws/linux-arm-his-g++
	$(QT_PATH)/bin/rcc -binary ./resource/resource.qrc -o ./gui.rcc
	make -j4
gui-debug:
	$(QT_PATH)/bin/qmake ./msgui.pro -spec $(QT_PATH)/mkspecs/qws/linux-arm-his-g++ CONFIG+=debug
	$(QT_PATH)/bin/rcc -binary ./resource/resource.qrc -o ./gui.rcc
	make -j4
clean:
	make clean
