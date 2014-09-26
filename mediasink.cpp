#include "mediasink.h"

#include <H264VideoRTPSource.hh>

#include <stdexcept>
#include <sstream>

// Implementation of "StreamMediaSink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define MEDIA_SINK_RECEIVE_BUFFER_SIZE (512000 + FF_INPUT_BUFFER_PADDING_SIZE)

StreamMediaSink* StreamMediaSink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
{
  return new StreamMediaSink(env, subsession, streamId);
}

StreamMediaSink::StreamMediaSink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
: MediaSink(env)
, m_fSubsession(subsession)
, m_idx(0)
, m_avCodec(NULL)
, m_avCodecContext(NULL)
, m_avFrame(NULL)
, m_bmp(NULL)
, m_screen(NULL)
, img_convert_ctx(NULL)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    std::stringstream ss;
    ss << "Could not initialize SDL - " << SDL_GetError();
    throw std::runtime_error(ss.str().c_str());
  }
  m_fStreamId = strDup(streamId);
  m_buffer = new u_int8_t[MEDIA_SINK_RECEIVE_BUFFER_SIZE + 4];

  av_init_packet(&m_avPacket);
  //m_avPacket.flags |= AV_PKT_FLAG_KEY;
  //m_avPacket.pts = m_avPacket.dts = 0;

  m_avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!m_avCodec) {
    throw std::runtime_error("Failed to find H264 ffmpeg codec");
  }

  m_avCodecContext = avcodec_alloc_context3(m_avCodec);
  if (!m_avCodecContext) {
    throw std::runtime_error("Failed to allocate codec context");
  }
  m_avCodecContext->pix_fmt = PIX_FMT_YUV420P;
  //m_avCodecContext->flags |= CODEC_FLAG2_CHUNKS;
  //m_avCodecContext->thread_count = 4;

  if (m_avCodec->capabilities & CODEC_CAP_TRUNCATED) {
    m_avCodecContext->flags |= CODEC_FLAG_TRUNCATED;
  }

  if (avcodec_open2(m_avCodecContext, m_avCodec, NULL) < 0) {
    throw std::runtime_error("Failed to open codec");
  }

  m_avFrame = av_frame_alloc();
  if (!m_avFrame) {
    throw std::runtime_error("Failed to allocate video frame");
  }

  m_screen = SDL_SetVideoMode(m_fSubsession.videoWidth(), m_fSubsession.videoHeight(), 0, 0);
  if (!m_screen) {
    throw std::runtime_error("SDL: could not set video mode - exiting");
  }

  // Allocate a place to put our YUV image on that screen
  m_bmp = SDL_CreateYUVOverlay(m_screen->w, m_screen->h, SDL_YV12_OVERLAY, m_screen);

  if (img_convert_ctx == NULL) {
    int w = m_screen->w;
    int h = m_screen->h;
    img_convert_ctx = sws_getContext(w, h, m_avCodecContext->pix_fmt, w, h, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
  }

  const u_int8_t start_code[] = {0x00, 0x00, 0x00, 0x01};
  u_int8_t idx = 0;
#if 0
  unsigned int n_records = 0;
  const char* sps = subsession.fmtp_spropparametersets();
  envir() << "SPS: " << sps << "\n";
  SPropRecord* pSPropRecord = parseSPropParameterSets(sps, n_records);

  for (int i = 0; i < n_records; ++i) {
    memcpy(&m_buffer[idx], start_code, 4);
    memcpy(&m_buffer[idx + 4], pSPrpoRecord[i].sPropBytes, pSPropBytes[i].sPropLength);
    idx += 4 + pSPropBytes[i].sPropLength;
    m_avPacket.size += 4 + pSPropBytes[i].sPropLength;
  }

  m_avPacket.data = m_buffer;

  int p = 0;
  int l = avcodec_decode_video2(m_avCodecContext, m_avFrame, &p, &m_avPacket);
#endif
  memcpy(&m_buffer[idx], &start_code, 4);
  idx += 4;
  m_fReceiveBuffer = &m_buffer[idx];
}

StreamMediaSink::~StreamMediaSink()
{
  delete[] m_buffer;
  delete[] m_fStreamId;
  if (m_avCodecContext) {
    avcodec_close(m_avCodecContext);
    av_free(m_avCodecContext);
  }
  if (m_avFrame) {
    av_frame_free(&m_avFrame);
  }
}

void StreamMediaSink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
  StreamMediaSink* sink = (StreamMediaSink*) clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
#define DEBUG_PRINT_NPT
#define NO_WRITE_RAW
#define NO_WRITE_JPEG

void StreamMediaSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (m_fStreamId != NULL) {
    envir() << "Stream \"" << m_fStreamId << "\"; ";
  }

  envir() << m_fSubsession.mediumName() << "/" << m_fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";

  if (numTruncatedBytes > 0) {
    envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  }

  char uSecsStr[6 + 1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned) presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int) presentationTime.tv_sec << "." << uSecsStr;

  if (m_fSubsession.rtpSource() != NULL && !m_fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }

#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << m_fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif

  m_avPacket.size = frameSize + 4;
  m_avPacket.data = m_buffer;
  int gotFrame = 0;
  int len = 0;

  while (m_avPacket.size > 0) {
    len = avcodec_decode_video2(m_avCodecContext, m_avFrame, &gotFrame, &m_avPacket);
    if (len < 0) {
      break;
    }
    if (gotFrame) {
      envir() << "Decoded Frame: " << ++m_idx << " Picture Type: " << av_get_picture_type_char(m_avFrame->pict_type) << " Key Frame: " << m_avFrame->key_frame << "\n";
      envir() << "showFrame: " << showFrame() << "\n";

      SDL_PollEvent(&m_event);
      switch (m_event.type) {
        case SDL_QUIT:
          SDL_Quit();
          exit(0);
          break;
        default:
          break;
      }
#if defined(WRITE_RAW)
      if (m_avFrame->key_frame) {
        writeRaw(m_idx);
      }
#endif
#if defined(WRITE_JPEG)
      //if (m_avFrame->pict_type == AV_PICTURE_TYPE_I) {
      writeJPEG(m_idx);
      //}
#endif
    }
    if (m_avPacket.data) {
      m_avPacket.size -= len;
      m_avPacket.data += len;
    }
  }

  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean StreamMediaSink::continuePlaying()
{
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(m_fReceiveBuffer, MEDIA_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
  return True;
}

int StreamMediaSink::writeRaw(int FrameNo)
{
  char file[256] = {0};
  sprintf(file, "frame-%d.data", ++m_idx);
  FILE* fp = fopen(file, "wb");
  if (fp != NULL) {
    for (int i = 0; i < 3; ++i) {
      fwrite(m_avFrame->data[i], m_avFrame->linesize[i], 1, fp);
    }
    fclose(fp);
    return 1;
  }
  return 0;
}

int StreamMediaSink::writeJPEG(int FrameNo)
{
  AVCodecContext *pOCodecCtx = NULL;
  AVCodec *pOCodec = NULL;
  uint8_t *Buffer = NULL;
  FILE *JPEGFile = NULL;
  char JPEGFName[256];

  int BufSiz = avpicture_get_size(AV_PIX_FMT_YUVJ444P, m_avCodecContext->width, m_avCodecContext->height);

  Buffer = new uint8_t[BufSiz + FF_INPUT_BUFFER_PADDING_SIZE];
  if (Buffer == NULL)
    return (0);

  pOCodecCtx = avcodec_alloc_context3(pOCodec);
  if (!pOCodecCtx) {
    free(Buffer);
    return (0);
  }

  pOCodecCtx->bit_rate = m_avCodecContext->bit_rate;
  pOCodecCtx->width = m_avCodecContext->width;
  pOCodecCtx->height = m_avCodecContext->height;
  pOCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ444P;
  pOCodecCtx->codec_id = CODEC_ID_MJPEG;
  pOCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  pOCodecCtx->time_base.num = m_avCodecContext->time_base.num;
  pOCodecCtx->time_base.den = m_avCodecContext->time_base.den;

  pOCodec = avcodec_find_encoder(pOCodecCtx->codec_id);
  if (!pOCodec) {
    free(Buffer);
    return (0);
  }

  if (avcodec_open2(pOCodecCtx, pOCodec, NULL) < 0) {
    free(Buffer);
    return (0);
  }

  pOCodecCtx->mb_lmin = pOCodecCtx->lmin = pOCodecCtx->qmin * FF_QP2LAMBDA;
  pOCodecCtx->mb_lmax = pOCodecCtx->lmax = pOCodecCtx->qmax * FF_QP2LAMBDA;
  pOCodecCtx->flags = CODEC_FLAG_QSCALE;
  pOCodecCtx->global_quality = pOCodecCtx->qmin * FF_QP2LAMBDA;

  m_avFrame->pts = 1;
  m_avFrame->quality = pOCodecCtx->global_quality;
  AVPacket packet;
  av_init_packet(&packet);
  packet.size = BufSiz;
  packet.data = Buffer;

  int image = 0;
  avcodec_encode_video2(pOCodecCtx, &packet, m_avFrame, &image);

  if (image) {
    sprintf(JPEGFName, "%06d.jpg", FrameNo);
    JPEGFile = fopen(JPEGFName, "wb");
    fwrite(packet.data, 1, packet.size, JPEGFile);
    fclose(JPEGFile);
  }

  avcodec_close(pOCodecCtx);
  delete[] Buffer;
  return (packet.size);
}

int StreamMediaSink::showFrame()
{
  int ret = SDL_LockYUVOverlay(m_bmp);

#if 1
  AVPicture pict; // = { { 0 } };

  pict.data[0] = m_bmp->pixels[0];
  pict.data[1] = m_bmp->pixels[2];
  pict.data[2] = m_bmp->pixels[1];

  pict.linesize[0] = m_bmp->pitches[0];
  pict.linesize[1] = m_bmp->pitches[2];
  pict.linesize[2] = m_bmp->pitches[1];

#if 0
  sws_scale(img_convert_ctx, m_avFrame->data, m_avFrame->linesize, 0, m_avCodecContext->height, pict.data, pict.linesize);
#else
  av_picture_copy(&pict, (AVPicture *) m_avFrame, AV_PIX_FMT_YUV420P, m_avCodecContext->width, m_avCodecContext->height);
#endif
#else
  //m_bmp->format = SDL_YV12_OVERLAY;
  //m_bmp->format = SDL_IYUV_OVERLAY;
  //m_bmp->h = m_avFrame->height;
  //m_bmp->w = m_avFrame->width;
  m_bmp->pixels[0] = m_avFrame->data[0];
  m_bmp->pixels[2] = m_avFrame->data[1];
  m_bmp->pixels[1] = m_avFrame->data[2];

  m_bmp->pitches[0] = m_avFrame->linesize[0];
  m_bmp->pitches[2] = m_avFrame->linesize[1];
  m_bmp->pitches[1] = m_avFrame->linesize[2];
#endif
  //AV_PIX_FMT_YUV420P
  // Convert the image into YUV format that SDL uses
  //img_convert(&pict, PIX_FMT_YUV420P, (AVPicture *) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

  SDL_UnlockYUVOverlay(m_bmp);

  m_rect.x = 0;
  m_rect.y = 0;
  m_rect.w = m_avFrame->width;
  m_rect.h = m_avFrame->height;
  ret = SDL_DisplayYUVOverlay(m_bmp, &m_rect);

  return ret;
}
