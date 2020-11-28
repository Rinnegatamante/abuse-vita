TARGET		:= Abuse
SOURCES		:= src src/net src/imlib src/lisp src/ui src/sdl2port src/lol		
INCLUDES	:= src src/imlib src/lisp src/new src/sdl/sdl2port

LIBS = -lSDL2_mixer -lSDL2_ttf -lSDL2_image -lSDL2 -lvita2d -lSceLibKernel_stub -lScePvf_stub \
	-lSceJpegEnc_stub -lSceAppMgr_stub -lSceCtrl_stub -lSceTouch_stub -lSceMotion_stub \
	-lScePromoterUtil_stub -lm -lSceNet_stub -lSceNetCtl_stub -lSceAppUtil_stub -lScePgf_stub \
	-ljpeg -lfreetype -lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz -lSceCamera_stub \
	-lspeexdsp -lmpg123 -lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub -lSceShellSvc_stub \
	-lopusfile -lopus -lSceHttp_stub -lSceAudioIn_stub -lluajit -ldl -ltaihen_stub  -lSceSysmodule_stub \
	-lSceVideodec_stub -lSceShutterSound_stub -lSceSsl_stub -lSceVshBridge_stub -lvorbisfile -lvorbis -logg \
	-lSceHid_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

INCLUDE_DIRS :=	$(foreach dir,$(INCLUDES),-I$(dir))

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -fno-lto -g -Wl,-q -O2 -ftree-vectorize -DVITA \
	-DNO_CHECK -DPACKAGE_NAME="\"Abuse\"" -DPACKAGE_VERSION="\"0.8\"" -DASSETDIR="\"app0:/\"" \
	$(INCLUDE_DIRS) -I$(VITASDK)/$(PREFIX)/include/SDL2 -Wno-class-conversion
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself -s $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
