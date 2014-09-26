#LIVE_MEDIA_SOURCE_ROOT = ../live
FFMPEG_SOURCE_ROOT = ../ffmpeg.git
FFMPEG_PKG_CONFIG_PRE_OPTIONS = PKG_CONFIG_PATH=$(FFMPEG_SOURCE_ROOT)/doc/examples/pc-uninstalled
LIVE_MEDIA_PKG_CONFIG_PRE_OPTIONS = PKG_CONFIG_PATH=.

#COMPILE_OPTS = -g -m64  -fPIC -I. -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 
COMPILE_OPTS = -g -m64  -fPIC -I. -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 
C_FLAGS = $(COMPILE_OPTS)
C_FLAGS += $(shell $(FFMPEG_PKG_CONFIG_PRE_OPTIONS)  pkg-config --cflags libavcodec)
C_FLAGS += $(shell $(LIVE_MEDIA_PKG_CONFIG_PRE_OPTIONS) pkg-config --cflags live555)

C_FLAGS += $(shell sdl-config --cflags)
CXX_FLAGS = $(C_FLAGS) -Wall -DBSD=1

FFMPEG_LIBS = $(shell $(FFMPEG_PKG_CONFIG_PRE_OPTIONS) pkg-config --libs-only-L libavcodec)
#FFMPEG_LIBS += $(shell $(FFMPEG_PKG_CONFIG_PRE_OPTIONS) pkg-config --libs-only-l libavcodec) 
FFMPEG_LIBS += -lavcodec -lswresample -lavutil -lswscale
LIVE_MEDIA_LIBS = $(shell $(LIVE_MEDIA_PKG_CONFIG_PRE_OPTIONS) pkg-config --libs-only-L live555)
LIVE_MEDIA_LIBS += $(shell $(LIVE_MEDIA_PKG_CONFIG_PRE_OPTIONS) pkg-config --libs-only-l live555)

SDL_LIBS = $(shell sdl-config --libs)
LD_OPTIONS = --discard-none --no-strip-discarded --static
LD_FLAGS = $(LIVE_MEDIA_LIBS) $(FFMPEG_LIBS) $(SDL_LIBS) -L/usr/lib/x86_64-linux-gnu -lXext -lX11 -lz -lpthread -lm -ldl 

SRCS = main.cpp client.cpp mediasink.cpp streamclientstate.cpp
OBJS = $(SRCS:.cpp=.o)

#.c.o:
	#gcc -c $(C_FLAGS) $<

.cpp.o:
	g++ -c $(CXX_FLAGS) $<

all : $(OBJS)
	g++ -g $(OBJS) $(LD_FLAGS) -o client

clean:
	rm $(OBJS) client

