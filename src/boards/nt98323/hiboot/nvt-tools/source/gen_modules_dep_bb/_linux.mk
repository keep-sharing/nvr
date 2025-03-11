#--------- COMPILER SETTING ----------------
AR = /usr/bin/ar
CC = /usr/bin/cc
CPP = /usr/bin/g++
CXX = /usr/bin/g++
LD = /usr/bin/ld
LDD = /usr/bin/ldd
NM = /usr/bin/nm
OBJCOPY = /usr/bin/objcopy
OBJDUMP = /usr/bin/objdump
STRIP = /usr/bin/strip
INCLUDE_DIR = /usr/include
LIBRARY_DIR = /usr/lib
#--------- END OF COMPILER SETTING ----------------

#--------- ENVIRONMENT SETTING --------------------
INCLUDES	= -I$(INCLUDE_DIR)
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces
COMPILE_OPTS	= $(INCLUDES) -I. -O2
C_FLAGS		= $(COMPILE_OPTS) $(WARNING)
CPPFLAGS	= $(C_FLAGS)
CFLAGS		= $(C_FLAGS) -Wstrict-prototypes
LD_FLAGS	= -L$(LIBRARY_DIR) -lstdc++
#--------- END OF ENVIRONMENT SETTING -------------

#--------- Compiling -------------------
# BINTYPE = APP or LIB
BIN_TYPE = APP

ifeq ($(BIN_TYPE),LIB)
EXPORT_HEADER = $(MAIN_NAME).h
endif

ifeq ($(BIN_TYPE),APP)
BIN = $(MAIN_NAME)
else ifeq ($(BIN_TYPE),LIB)
BIN = lib$(MAIN_NAME).a
endif

HEADER = $(shell find . -name "*.h")

OBJ = $(patsubst %.c,%.o,$(filter %.c, $(SRC))) $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRC)))

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c *.cpp */*.cpp)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"
else

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ)  $(LD_FLAGS) -g#charlie 
	$(NM) -n $@ > $@.sym
	#//charlie $(STRIP) $@
	$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp $(HEADER)
	$(CXX) $(CPPFLAGS) -c $< -o $@

ifeq ($(BIN_TYPE),APP)
clean:
	rm -vf $(BIN) $(OBJ_C) $(OBJ_CPP) $(BIN) *.sym *.o *.a *.so*
	rm -vf ./release/$(BIN)

install: $(BIN) 
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>" 
	cp $(BIN) ../../

else ifeq ($(BIN_TYPE),LIB)

clean:
	rm -vf $(BIN) $(OBJ_C) $(OBJ_CPP) $(BIN) *.sym *.o *.a *.so*
	rm -vf ./release/lib/$(BIN) 
	
install: $(BIN) $(EXPORT_HEADER)
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	cp $(BIN) ../../
endif
endif
