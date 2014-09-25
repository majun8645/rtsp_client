#include <MediaSink.hh>
#include <MediaSession.hh>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <SDL.h>
#include <SDL_thread.h>
}

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class StreamMediaSink
	: public MediaSink
{
public:
	static StreamMediaSink* createNew(UsageEnvironment& env,
		MediaSubsession& subsession, // identifies the kind of data that's being received
		char const* streamId = NULL); // identifies the stream itself (optional)

private:
	StreamMediaSink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
	// called only by "createNew()"
	virtual ~StreamMediaSink();

	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();

private:
    int showFrame();
	int writeRaw(int FrameNo);
	int writeJPEG(int FrameNo);

	u_int8_t* m_buffer;
	u_int8_t* m_fReceiveBuffer;

	MediaSubsession& m_fSubsession;
	char* m_fStreamId;
	u_int32_t m_idx;

	AVCodec* m_avCodec;
	AVCodecContext* m_avCodecContext;
	AVFrame* m_avFrame;
	AVPacket m_avPacket;
  	SDL_Overlay *m_bmp;
  	SDL_Surface *m_screen;
  	SDL_Rect m_rect;
  	SDL_Event m_event;
};
