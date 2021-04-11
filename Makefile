TARGET		:= Abuse
TITLE		:= ABUSEVITA
SOURCES		:= src src/net src/imlib src/lisp src/ui src/sdl2port src/lol		
INCLUDES	:= src src/imlib src/lisp src/new src/sdl/sdl2port

LIBS = -lSDL2_mixer -lSDL2_ttf -lSDL2_image -lSDL2 -lvita2d -lSceLibKernel_stub -lScePvf_stub \
	-lSceAppMgr_stub -lSceCtrl_stub -lSceTouch_stub -lSceMotion_stub \
	-lm -lSceNet_stub -lSceNetCtl_stub -lSceAppUtil_stub -lScePgf_stub \
	-ljpeg -lfreetype -lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lspeexdsp -lmpg123 -lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub \
	-lSceHttp_stub -lSceAudioIn_stub -lSceSysmodule_stub -lSceSsl_stub \
	-lvorbisfile -lvorbis -logg -lSceHid_stub -lmikmod -lFLAC

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

INCLUDE_DIRS :=	$(foreach dir,$(INCLUDES),-I$(dir))

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -fno-lto -g -Wl,-q -O3 -DVITA -fno-optimize-sibling-calls \
	-DNO_CHECK -DPACKAGE_NAME="\"Abuse\"" -DPACKAGE_VERSION="\"0.8\"" -DASSETDIR="\"app0:/data/\"" \
	$(INCLUDE_DIRS) -I$(VITASDK)/$(PREFIX)/include/SDL2 -Wno-class-conversion
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -s $< data/eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) -d ATTRIBUTE2=12 "$(TARGET)" data/sce_sys/param.sfo

	#------------ Comment this if you don't have 7zip ------------------
	7z a -tzip ./$(TARGET).vpk -r ./data/sce_sys ./data/data ./data/eboot.bin
	#-------------------------------------------------------------------

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
