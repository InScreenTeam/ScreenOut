#include "Muxer.h"

#define STREAM_FRAME_RATE 10
#define COMPRESSION_RATIO 0.2

namespace ScreenOut
{
	Muxer::Muxer(int width, int height)
		: logger("muxer.log")
	{
		this->width = width;
		this->height = height;
	}

	void Muxer::Initialize()
	{
		int ret;
		av_register_all();   
		char* filename = "test.mp4";		
		avformat_alloc_output_context2(&formatContext, NULL, NULL, filename);
		if (!formatContext) 
		{
			logger << Logger::Level::LOG_ERROR << 
				"Could not deduce output format from file extension: using MPEG.";
			avformat_alloc_output_context2(&formatContext, NULL, "mpeg", filename);
		}
		if (!formatContext) 
			logger << Logger::Level::LOG_ERROR << "Could not allocate output format context";
		AVOutputFormat* format = formatContext->oformat;
		/* Add the audio and video streams using the default format codecs
		 * and initialize the codecs. */
		videoStream = NULL;
		audioStream = NULL;
		if (format->video_codec != AV_CODEC_ID_NONE) 
		{
			videoStream = AddStream(&videoCodec, format->video_codec);
		}
		if (format->audio_codec != AV_CODEC_ID_NONE) 
		{
			//audioStream = AddStream(&audioCodec, format->audio_codec);
		}
		/* Now that all the parameters are set, we can open the audio and
		 * video codecs and allocate the necessary encode buffers. */
		if (videoStream)
			OpenVideo();
		if (audioStream)
			OpenAudio();
		av_dump_format(formatContext, 0, filename, 1);
		/* open the output file, if needed */
		if (!(format->flags & AVFMT_NOFILE)) {
			ret = avio_open(&formatContext->pb, filename, AVIO_FLAG_WRITE);
			if (ret < 0) 
			{
				logger << Logger::Level::LOG_ERROR << "Could not open output file";				
			}
		}
		/* Write the stream header, if any. */
		ret = avformat_write_header(formatContext, NULL);
		if (ret < 0) {
			logger << Logger::Level::LOG_ERROR << "Could not write stream header.";
		}

	}

	void Muxer::OpenVideo()
	{
		int ret;
		AVCodecContext *codecContext = videoStream->codec;
		/* open the codec */
		ret = avcodec_open2(codecContext, videoCodec, NULL);
		if (ret < 0) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not open video codec.";
		}
		/* allocate and init a re-usable frame */
		frame = avcodec_alloc_frame();
		if (!frame) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not allocate video frame.";			
		}
		frame->pts = 0;		
	}

	void Muxer::OpenAudio()
	{

	}

	AVStream* Muxer::AddStream(AVCodec** codec, AVCodecID codecId)
	{
		AVCodecContext *codecContext;
		AVStream *stream;
		/* find the encoder */
		*codec = avcodec_find_encoder(codecId);
		if (!(*codec)) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not find encoder for '"
				<< avcodec_get_name(codecId) << "' codec.";
		}

		stream = avformat_new_stream(formatContext, *codec);
		if (!stream) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not allocate stream";			
		}
		stream->id = formatContext->nb_streams - 1;
		codecContext = stream->codec;
		switch ((*codec)->type) 
		{
			case AVMEDIA_TYPE_AUDIO:
				stream->id = 1;
				codecContext->sample_fmt  = AV_SAMPLE_FMT_S16;
				codecContext->bit_rate    = 64000;
				codecContext->sample_rate = 44100;
				codecContext->channels    = 2;
				break;
			case AVMEDIA_TYPE_VIDEO:
				codecContext->codec_id = codecId;
				codecContext->bit_rate = width * height * 8 * COMPRESSION_RATIO * STREAM_FRAME_RATE;
				/* Resolution must be a multiple of two. */
				codecContext->width    = width;
				codecContext->height   = height;
				/* timebase: This is the fundamental unit of time (in seconds) in terms
				 * of which frame timestamps are represented. For fixed-fps content,
				 * timebase should be 1/framerate and timestamp increments should be
				 * identical to 1. */
				codecContext->time_base.den = STREAM_FRAME_RATE;
				codecContext->time_base.num = 1;
				codecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
				codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
				if (codecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
					/* just for testing, we also add B frames */
					codecContext->max_b_frames = 0;
				}
				if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) 
				{
					/* Needed to avoid using macroblocks in which some coeffs overflow.
					 * This does not happen with normal video, it just happens here as
					 * the motion of the chroma plane does not match the luma plane. */
					codecContext->mb_decision = 0;
				}				
				break;
			default:
				break;
		}
		/* Some formats want stream headers to be separate. */
		if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
			codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

		return stream;
	}
	
	double Muxer::CurrentTimeStamp()
	{
		return (double)videoStream->pts.val * videoStream->time_base.num /
                        videoStream->time_base.den;
	}

	void Muxer::Flush()
	{				
		int ret, got_packet;
		AVPacket packet = {0};		
		do
		{
			av_init_packet(&packet);
			ret = avcodec_encode_video2(videoStream->codec, &packet, NULL, &got_packet);		
			if (!ret && got_packet && packet.size) 
			{
				packet.stream_index = videoStream->index;				
				ret = av_interleaved_write_frame(formatContext, &packet);
			} 
			logger << Logger::Level::LOG_INFO << "Current frame pts: " << 
			(double)frame->pts * videoStream->time_base.num / videoStream->time_base.den <<
			"; Stream pts: " << CurrentTimeStamp() <<
			";";
			av_free_packet(&packet);
		} while (got_packet);
		/* Write the trailer, if any. The trailer must be written before you
		 * close the CodecContexts open when you wrote the header; otherwise
		 * av_write_trailer() may try to use memory that was freed on
		 * av_codec_close(). */		
		av_write_trailer(formatContext);
		/* Close each codec. */
		if (videoStream)
			CloseVideo();
		if (audioStream)
			CloseAudio();

		if (!(formatContext->oformat->flags & AVFMT_NOFILE))
			/* Close the output file. */
				avio_close(formatContext->pb);
		/* free the stream */
		avformat_free_context(formatContext);
	}

	void Muxer::WriteVideoFrame(AVPicture* buffer)
	{		

		*((AVPicture *)frame) = *buffer;
		int ret;		
		AVCodecContext *codec = videoStream->codec;
		AVPacket packet = { 0 };
		int got_packet;
		av_init_packet(&packet);		
		/* encode the image */
		//packet.duration = av_rescale_q(1000 / STREAM_FRAME_RATE, 
			//videoStream->codec->time_base, videoStream->time_base);
		ret = avcodec_encode_video2(codec, &packet, frame, &got_packet);
		if (ret < 0) 
		{
			logger << Logger::Level::LOG_ERROR << 
				"Error encoding video frame";			
		}
		/* If size is zero, it means the image was buffered. */		
		if (!ret && got_packet && packet.size) 
		{
			packet.stream_index = videoStream->index;
			/* Write the compressed frame to the media file. */
			ret = av_interleaved_write_frame(formatContext, &packet);
		} 
		else 			
			ret = 0;					
		if (ret != 0) 
		{
			logger << Logger::Level::LOG_ERROR << "Error while writing video frame.";			
		}				
		videoStream->codec->delay;
		frame->pts += av_rescale_q(1, videoStream->codec->time_base, videoStream->time_base);				
		logger << Logger::Level::LOG_INFO << "Current frame pts: " << 
			(double)frame->pts * videoStream->time_base.num / videoStream->time_base.den <<
			"; Stream pts: " << CurrentTimeStamp() <<
			"; Packet pts: " << (double)packet.pts * videoStream->time_base.num / videoStream->time_base.den;
	}

	void Muxer::CloseVideo()
	{
		avcodec_close(videoStream->codec);				
		av_free(frame);
	}

	void Muxer::CloseAudio()
	{

	}

	Muxer::~Muxer(void)
	{
	}
}
