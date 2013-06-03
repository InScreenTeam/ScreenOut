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

	void Muxer::Initialize(char* filename)
	{
		int ret;
		av_register_all();   			

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
		
		videoStream = NULL;
		audioStream = NULL;
		if (format->video_codec != AV_CODEC_ID_NONE) 
		{
			videoStream = AddStream(&videoCodec, format->video_codec);
		}
		if (format->audio_codec != AV_CODEC_ID_NONE) 
		{
			audioStream = AddStream(&audioCodec, format->audio_codec);
		}
		
		if (videoStream)
			OpenVideo();
		if (audioStream)
			OpenAudio();
		av_dump_format(formatContext, 0, filename, 1);
		
		if (!(format->flags & AVFMT_NOFILE)) {
			ret = avio_open(&formatContext->pb, filename, AVIO_FLAG_WRITE);
			if (ret < 0) 
			{
				logger << Logger::Level::LOG_ERROR << "Could not open output file";				
			}
		}
		
		ret = avformat_write_header(formatContext, NULL);
		if (ret < 0) {
			logger << Logger::Level::LOG_ERROR << "Could not write stream header.";
		}

	}

	void Muxer::OpenVideo()
	{
		int ret;
		AVCodecContext *codecContext = videoStream->codec;

		ret = avcodec_open2(codecContext, videoCodec, NULL);
		if (ret < 0) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not open video codec.";
		}		

		frame = avcodec_alloc_frame();
		if (!frame) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not allocate video frame.";			
		}
		frame->pts = 0;		
	}

	void Muxer::WriteVideoFrame(AVPicture* buffer)
	{		

		*((AVPicture *)frame) = *buffer;
		int ret;		
		AVCodecContext *codec = videoStream->codec;
		AVPacket packet = { 0 };
		int got_packet;
		av_init_packet(&packet);		
		ret = avcodec_encode_video2(codec, &packet, frame, &got_packet);
		if (ret < 0) 
		{
			logger << Logger::Level::LOG_ERROR << 
				"Error encoding video frame";			
		}	
		if (!ret && got_packet && packet.size) 
		{
			packet.stream_index = videoStream->index;			
			ret = av_interleaved_write_frame(formatContext, &packet);
		} 
		else 			
			ret = 0;					
		if (ret != 0) 
		{
			logger << Logger::Level::LOG_ERROR << "Error while writing video frame.";			
		}						
		frame->pts += av_rescale_q(1, videoStream->codec->time_base, videoStream->time_base);				
		/*
		logger << Logger::Level::LOG_INFO << "Current frame pts: " << 
			(double)frame->pts * videoStream->time_base.num / videoStream->time_base.den <<
			"; Stream pts: " << CurrentVideoTimeStamp() <<
			"; Packet pts: " << (double)packet.pts * videoStream->time_base.num / videoStream->time_base.den; */
	}

	double Muxer::CurrentVideoTimeStamp()
	{
		return (double)videoStream->pts.val * videoStream->time_base.num /
                        videoStream->time_base.den;
	}

	void Muxer::CloseVideo()
	{
		avcodec_close(videoStream->codec);				
		av_free(frame);
	}

	void Muxer::OpenAudio()
	{
		AVCodecContext *codecContext = audioStream->codec;;
		int ret;	
		
		ret = avcodec_open2(codecContext, audioCodec, NULL);
		if (ret < 0) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not open audio codec.";
			throw;
		}		

		if (codecContext->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
			audioFrameSize = 10000;
		else
			audioFrameSize = codecContext->frame_size;

		samplesBuffer = (int16_t*)av_malloc(audioFrameSize *
							av_get_bytes_per_sample(codecContext->sample_fmt) *
							codecContext->channels);
		if (!samplesBuffer) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not allocate audio samples buffer.";
			throw;
		}
	}

	void Muxer::WriteAudioFrame(int16_t* samples)
	{
		AVCodecContext *codecContext = audioStream->codec;
		AVPacket pkt = { 0 };
		AVFrame *frame = avcodec_alloc_frame();
		int got_packet, ret;

		av_init_packet(&pkt);		
		
		frame->nb_samples = audioFrameSize;
		avcodec_fill_audio_frame(frame, codecContext->channels, codecContext->sample_fmt,
								 (uint8_t *)samples,
								 audioFrameSize *
								 av_get_bytes_per_sample(codecContext->sample_fmt) *
								 codecContext->channels, 1);

		ret = avcodec_encode_audio2(codecContext, &pkt, frame, &got_packet);
		if (ret < 0) {
			logger << Logger::Level::LOG_ERROR << "Error encoding audio frame at "
				<< CurrentAudioTimeStamp() << "s.";
			
		}

		if (!got_packet)
			return;

		pkt.stream_index = audioStream->index;
		
		ret = av_interleaved_write_frame(formatContext, &pkt);
		if (ret != 0)
		{
			logger << Logger::Level::LOG_ERROR << "Error while writing audio frame at"
				<< CurrentAudioTimeStamp() << "s.";								
		}
		avcodec_free_frame(&frame);
	}

	double Muxer::CurrentAudioTimeStamp()
	{
		return (double)audioStream->pts.val * audioStream->time_base.num /
                        audioStream->time_base.den;
	}

	void Muxer::CloseAudio()
	{
		avcodec_close(audioStream->codec);
		av_free(samplesBuffer);
	}

	AVStream* Muxer::AddStream(AVCodec** codec, AVCodecID codecId)
	{
		AVCodecContext *codecContext;
		AVStream *stream;

		*codec = avcodec_find_encoder(codecId);
		if (!(*codec)) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not find encoder for '"
				<< avcodec_get_name(codecId) << "' codec.";
		}

		stream = avformat_new_stream(formatContext, *codec);
		if (!stream) 
		{
			logger << Logger::Level::LOG_ERROR << "Could not allocate stream.";			
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
				
				codecContext->width    = width;
				codecContext->height   = height;				

				codecContext->time_base.den = STREAM_FRAME_RATE;
				codecContext->time_base.num = 1;

				codecContext->gop_size = 12;
				codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
				if (codecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) 
				{					
					codecContext->max_b_frames = 0;
				}
				if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) 
				{					
					codecContext->mb_decision = 0;
				}				
				break;
			default:
				break;
		}		
		if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
			codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
		return stream;
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
			"; Stream pts: " << CurrentVideoTimeStamp() <<
			";";
			av_free_packet(&packet);
		} while (got_packet);
		do
		{
			av_init_packet(&packet);			
			ret = avcodec_encode_audio2(audioStream->codec, &packet, NULL, &got_packet);		
			if (!ret && got_packet && packet.size) 
			{
				packet.stream_index = audioStream->index;				
				ret = av_interleaved_write_frame(formatContext, &packet);
			} 
			logger << Logger::Level::LOG_INFO << "Current audio frame pts: " << 
			(double)frame->pts * audioStream->time_base.num / audioStream->time_base.den <<
			"; Stream pts: " << CurrentVideoTimeStamp() <<
			";";
			av_free_packet(&packet);
		} while (got_packet);
		
		av_write_trailer(formatContext);
		
		if (videoStream)
			CloseVideo();
		if (audioStream)
			CloseAudio();

		if (!(formatContext->oformat->flags & AVFMT_NOFILE))
				avio_close(formatContext->pb);		
		avformat_free_context(formatContext);
	}

	void Muxer::Reset()
	{

	}

	Muxer::~Muxer(void)
	{
	}
}
