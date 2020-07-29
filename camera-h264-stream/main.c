#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

void transcode1(AVFrame *srcFrame) {
  AVFrame *frame;
  frame = av_frame_alloc();
  if (!frame) {
    printf("Could not allocate video frame\n");
    return;
  }
  /*
  int buffSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, srcFrame->width,
                                          srcFrame->height, 16);
  printf("buffSize:%d\n", buffSize);
  uint8_t *buff = (uint8_t *)av_malloc(buffSize);
  if (av_image_fill_arrays(frame->data, frame->linesize, buff,
                           AV_PIX_FMT_YUV420P, srcFrame->width,
                           srcFrame->height, 16) < 0) {
    printf("av_image_alloc error\n");
    return;
  }
  */
  // set up YV12 pixel array (12 bits per pixel)
  uint8_t *yPlane, *uPlane, *vPlane;
  size_t yPlaneSz, uvPlaneSz;
  int uvPitch;
  AVPicture pict;
  yPlaneSz = srcFrame->width * srcFrame->height;
  uvPlaneSz = srcFrame->width * srcFrame->height / 4;
  uvPitch = srcFrame->width / 2;
  yPlane = (uint8_t *)malloc(yPlaneSz);
  uPlane = (uint8_t *)malloc(uvPlaneSz);
  vPlane = (uint8_t *)malloc(uvPlaneSz);
  pict.data[0] = yPlane;
  pict.data[1] = uPlane;
  pict.data[2] = vPlane;
  pict.linesize[0] = srcFrame->width;
  pict.linesize[1] = uvPitch;
  pict.linesize[2] = uvPitch;

  int i = 0;
  for (i = 0; i < srcFrame->width; i++) {
    if (i % 16 == 0) {
      printf("\n");
    }
    printf("%02x", pict.data[0][i]);
  }

  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(srcFrame->width, srcFrame->height, AV_PIX_FMT_YUYV422,
                          srcFrame->width, srcFrame->height, AV_PIX_FMT_YUV420P,
                          SWS_BICUBIC, NULL, NULL, NULL);
  sws_scale(swsCtx, (const uint8_t *const *)srcFrame->data, srcFrame->linesize,
            0, srcFrame->height, pict.data, pict.linesize);

  printf("transcode AV_PIX_FMT_NONE:%d, format: %d\n", AV_PIX_FMT_NONE,
         frame->format);
  printf("pkt_size: %d, width:%d, height:%d\n", srcFrame->pkt_size,
         srcFrame->width, srcFrame->height);
  printf("transcode pkt_size: %d, width:%d, height:%d\n", frame->pkt_size,
         frame->width, frame->height);
  for (i = 0; i < srcFrame->width; i++) {
    if (i % 16 == 0) {
      printf("\n");
    }
    printf("%02x", pict.data[0][i]);
  }
  sws_freeContext(swsCtx);
  av_frame_free(&frame);
}

void encode(AVFrame *frame) {
  if (av_frame_get_buffer(frame, 0) < 0) {
    printf("av_frame_get_buffer error pFrameYUV\n");
    return;
  }
  if (av_frame_make_writable(frame) < 0) {
    printf("av_frame_make_writable error\n");
    return;
  }

  AVPacket *p264Packet;
  p264Packet = av_packet_alloc();
  if (!p264Packet)
    return;
  printf("size: %d\n", p264Packet->size);

  AVCodecContext *p264CodecCtx;
  AVCodec *p264Codec;

  p264Codec = avcodec_find_encoder_by_name("libx264");
  if (!p264Codec) {
    fprintf(stderr, "Codec not found\n");
    return;
  }
  p264CodecCtx = avcodec_alloc_context3(p264Codec);
  if (!p264CodecCtx) {
    printf("Codec not found.\n");
    return;
  }
  /* put sample parameters */
  p264CodecCtx->bit_rate = 400000;
  /* resolution must be a multiple of two */
  p264CodecCtx->width = frame->width;
  p264CodecCtx->height = frame->height;
  /* frames per second */
  p264CodecCtx->time_base = (AVRational){1, 25};
  p264CodecCtx->framerate = (AVRational){25, 1};

  p264CodecCtx->gop_size = 10;
  p264CodecCtx->max_b_frames = 1;
  p264CodecCtx->pix_fmt = AV_PIX_FMT_YUV422P;
  if (p264Codec->id == AV_CODEC_ID_H264)
    av_opt_set(p264CodecCtx->priv_data, "preset", "slow", 0);
  if (avcodec_open2(p264CodecCtx, p264Codec, NULL) < 0) {
    printf("Could not open codec.\n");
    return;
  }
  frame->pts = 1;
  if (avcodec_send_frame(p264CodecCtx, frame) < 0) {
    printf("avcodec_send_frame error\n");
    return;
  }
  int ret = 0;
  ret = avcodec_receive_packet(p264CodecCtx, p264Packet);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    printf("ret:%d, EAGAIN:%d, AVERROR_EOF:%d\n", ret, AVERROR(EAGAIN),
           AVERROR_EOF);
  } else if (ret < 0) {
    printf("Error during decoding\n");
  } else {
    printf("saving frame %3d\n", p264CodecCtx->frame_number);
  }
  printf("size: %d\n", p264Packet->size);
  av_free_packet(p264Packet);
}

void transcode(AVFrame *srcFrame) {

  AVFrame *frame;
  frame = av_frame_alloc();
  if (!frame) {
    printf("Could not allocate video frame\n");
    return;
  }
  int buffSize = av_image_get_buffer_size(AV_PIX_FMT_YUV422P, srcFrame->width,
                                          srcFrame->height, 16);
  printf("buffSize:%d\n", buffSize);
  uint8_t *buff = (uint8_t *)av_malloc(buffSize);
  if (av_image_fill_arrays(frame->data, frame->linesize, buff,
                           AV_PIX_FMT_YUV422P, srcFrame->width,
                           srcFrame->height, 16) < 0) {
    printf("av_image_alloc error\n");
    return;
  }
  frame->format = AV_PIX_FMT_YUV422P;
  frame->width = srcFrame->width;
  frame->height = srcFrame->height;

  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(srcFrame->width, srcFrame->height, AV_PIX_FMT_YUYV422,
                          srcFrame->width, srcFrame->height, AV_PIX_FMT_YUV422P,
                          SWS_BICUBIC, NULL, NULL, NULL);
  sws_scale(swsCtx, (const uint8_t *const *)srcFrame->data, srcFrame->linesize,
            0, srcFrame->height, frame->data, frame->linesize);

  printf("transcode AV_PIX_FMT_NONE:%d, format: %d\n", AV_PIX_FMT_NONE,
         frame->format);
  encode(frame);
  sws_freeContext(swsCtx);
  av_frame_free(&frame);
}

void decode(AVStream *stream, AVPacket *packet) {
  AVCodecContext *codecCtx = stream->codec;
  printf("AV_CODEC_ID_NONE:%d, codecID: %d\n", AV_CODEC_ID_NONE,
         codecCtx->codec_id);
  AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
  if (codec == NULL) {
    printf("codec not found\n");
    return;
  }
  if (avcodec_open2(codecCtx, codec, NULL) < 0) {
    printf("Could not open codec\n");
    return;
  }
  AVFrame *frame;
  frame = av_frame_alloc();
  if (!frame) {
    printf("Could not allocate video frame\n");
    return;
  }
  if (avcodec_send_packet(codecCtx, packet) < 0) {
    printf("Error sending a packet for decoding\n");
    return;
  }
  int ret = 0;
  ret = avcodec_receive_frame(codecCtx, frame);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    printf("decode ret:%d, EAGAIN:%d, AVERROR_EOF:%d\n", ret, AVERROR(EAGAIN),
           AVERROR_EOF);
  } else if (ret < 0) {
    printf("Error during decoding\n");
  } else {
    printf("saving frame %3d\n", codecCtx->frame_number);
  }
  printf("AV_PIX_FMT_NONE:%d, format: %d\n", AV_PIX_FMT_NONE, frame->format);
  printf("pkt_size: %d, width:%d, height:%d\n", frame->pkt_size, frame->width,
         frame->height);
  printf("codecCtx width:%d, height:%d\n", codecCtx->width, codecCtx->height);
  transcode(frame);
  av_frame_free(&frame);
}

void main() {
  avdevice_register_all();

  AVInputFormat *fmt = av_find_input_format("v4l2");
  AVFormatContext *ctx = avformat_alloc_context();
  if (avformat_open_input(&ctx, "/dev/video0", fmt, NULL) != 0) {
    printf("can not open input stream\n");
    return;
  }

  if (avformat_find_stream_info(ctx, NULL) < 0) {
    printf("Couldn't find stream information.\n");
    return;
  }

  AVPacket pkt;
  int i = 0;
  for (i = 0; i < 10; i++) {
    if (av_read_frame(ctx, &pkt) < 0) {
      printf("av_read_frame error \n");
      break;
    }
    decode(ctx->streams[0], &pkt);
    av_packet_unref(&pkt);
  }

  avformat_close_input(&ctx);

  return;
}
