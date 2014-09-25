LIVE_MEDIA_SOURCE_ROOT = ../live
FFMPEG_SOURCE_ROOT = ../ffmpeg.git

ifdef LIVE_MEDIA_SOURCE_ROOT
LIVE_MEDIA_INCLUDES = -I$(LIVE_MEDIA_SOURCE_ROOT)/liveMedia/include
LIVE_MEDIA_INCLUDES += -I$(LIVE_MEDIA_SOURCE_ROOT)/BasicUsageEnvironment/include
LIVE_MEDIA_INCLUDES += -I$(LIVE_MEDIA_SOURCE_ROOT)/UsageEnvironment/include
LIVE_MEDIA_INCLUDES += -I$(LIVE_MEDIA_SOURCE_ROOT)/groupsock/include
LIVE_MEDIA_INCLUDES += -I$(LIVE_MEDIA_SOURCE_ROOT)/BasicUsageEnvironment/include
else
	LIVE_MEDIA_INCLUDES = $(shell pkg-config --cflags-only-L libMedia)
endif

ifdef FFMPEG_SOURCE_ROOT
FFMPEG_INCLUDES += -I$(FFMPEG_SOURCE_ROOT)/libavcodec
FFMPEG_INCLUDES += -I$(FFMPEG_SOURCE_ROOT)/libavutil
else
	FFMPEG_INCLUDES = $(shell pkg-config --cflags-only-L libavcodec)
endif

COMPILE_OPTS = -g -m64  -fPIC -I. -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 
C_FLAGS = $(LIVE_MEDIA_INCLUDES) $(FFMPEG_INCLUDES) $(COMPILE_OPTS)
ifndef FFMPEG_SOURCE_ROO
C_FLAGS += $(shell pkg-config --cflags libavcodec)
endif
C_FLAGS += $(shell sdl-config --cflags)
CXX_FLAGS = $(C_FLAGS) -Wall -DBSD=1

ifndef FFMPEG_SOURCE_ROOT
FFMPEG_LIBS = -L$(FFMPEG_SOURCE_ROOT)/libavcodec -lavcodec -lx264 -lm 
FFMPEG_LIBS += -L$(FFMPEG_SOURCE_ROOT)/libswresample -lswresample
FFMPEG_LIBS += -L$(FFMPEG_SOURCE_ROOT)/libavutil -lavutil
else
FFMPEG_LIBS = $(shell pkg-config --libs-only-L libavcodec) $(shell pkg-config --libs-only-l libavcodec) 
endif

ifdef LIVE_MEDIA_SOURCE_ROOT
  LIVE_MEDIA_LIBS = -L$(LIVE_MEDIA_SOURCE_ROOT)/liveMedia -lliveMedia
  LIVE_MEDIA_LIBS += -L$(LIVE_MEDIA_SOURCE_ROOT)/groupsock -lgroupsock
  LIVE_MEDIA_LIBS += -L$(LIVE_MEDIA_SOURCE_ROOT)/BasicUsageEnvironment -lBasicUsageEnvironment
  LIVE_MEDIA_LIBS += -L$(LIVE_MEDIA_SOURCE_ROOT)/UsageEnvironment -lUsageEnvironment
else
  LIVE_MEDIA_LIBS = $(shell pkg-config --libs libliveMedia)
endif

LD_OPTIONS = --discard-none --no-strip-discarded  
LD_FLAGS = -lpthread -socket -ldl -lstdc++ $(LIVE_MEDIA_LIBS) $(FFMPEG_LIBS) $(shell sdl-config --libs)

SRCS = main.cpp client.cpp mediasink.cpp streamclientstate.cpp
OBJS = $(SRCS:.cpp=.o)

#.c.o:
	#gcc -c $(C_FLAGS) $<

.cpp.o:
	g++ -c $(CXX_FLAGS) $<

all : $(OBJS)
	ld -g $(OBJS) $(LD_FLAGS) -o client

