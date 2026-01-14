#include "FFPlayer.h"
#include <QEventLoop>
#include <cstringt.h>
#include <QTimer>
#include <QMediaDevices>

#include "FFPlayer/AudioDecoder.h"
#include "LogManager.h"

FFPlayer::FFPlayer(QObject* parent) : QObject(parent)
{
	qRegisterMetaType<PlayerState>("PlayerState");
	avformat_network_init();
	_audioDecoderMgr = std::make_unique<DecoderThreadManager>([](AVFormatContext* fmtCtx, QObject* parent) -> DecoderInterface* {
		return new AudioDecoder(fmtCtx, parent);
	}, this);
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::initFinished, this, [this](bool success) {
		_hasAudio = success;
		if (!_hasAudio) {
			LOG(INFO) << "Audio decoder initialization failed";
		}
		LOG(INFO) << "Audio decoder initialization success";
		emit initFinished();
	});
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::clockChanged, this, [this](double clock) {
		emit clockChanged(clock);
	});

	connect(this, &FFPlayer::volumeChanged, _audioDecoderMgr.get(), &DecoderThreadManager::setVolume);

	QMediaDevices* _mediaDevices = new QMediaDevices(this);
	connect(_mediaDevices, &QMediaDevices::audioOutputsChanged, this, [this]() {
		if (_audioDecoderMgr->decoder()) {
			AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
			decoder->switchAudioDevice();
		}
	});

}

FFPlayer::~FFPlayer()
{
	LOG(INFO) << "FFPlayer destory.";
	stop();
}

void FFPlayer::play(const QString& filepath)
{
	if (!filepath.isEmpty()) _filepath = filepath;
	stop();
	setState(PlayerState::PLAYING);
	std::thread([this]() {
		readFile();
	}).detach();
}

void FFPlayer::play()
{
	setState(PlayerState::PLAYING);
}

void FFPlayer::stop()
{
	LOG(INFO) << "free FFPlayer";
	setState(PlayerState::STOP);
	if (_hasAudio) {
		// 清空音频相关资源
		if (_audioDecoderMgr)
			_audioDecoderMgr->stop();
	}

	// 清空视频相关资源
	if (_hasVideo) {

	}
	_hasAudio = false;
	_hasVideo = false;
	_seek_pos = -1;

	if (_formatCtx) {
		avformat_close_input(&_formatCtx);
		avformat_free_context(_formatCtx);
		_formatCtx = nullptr;
	}
}

void FFPlayer::pause()
{
	if (_state != PlayerState::PLAYING)	return;
	setState(PlayerState::PAUSE);
}

void FFPlayer::setVolume(int volume)
{
	emit volumeChanged(volume);
}

PlayerState FFPlayer::getState()
{
	return _state;
}

void FFPlayer::seek(int clock) {
	_seek_pos = clock;
}

int FFPlayer::getDuration()
{
	return _formatCtx ? _formatCtx->duration / AV_TIME_BASE : 0;
}

void FFPlayer::updateBand(int index, float gain)
{
	AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
	decoder->updateBand(index, gain);
}

void FFPlayer::setEnvironment(int index)
{
	AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
	decoder->setEnvironment(index);
}

void FFPlayer::setEnvDepthValue(int value)
{
	AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
	decoder->setEnvDepthValue(value);
}

void FFPlayer::setEnvIntensityValue(int value)
{
    AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
    decoder->setEnvIntensityValue(value);
}

void FFPlayer::setBassLevel(int value)
{
	AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
	decoder->setBassLevel(value);
}

void FFPlayer::setTrableLevel(int value)
{
	AudioDecoder* decoder = dynamic_cast<AudioDecoder*>(_audioDecoderMgr->decoder());
	decoder->setTrableLevel(value);
}

void FFPlayer::fateError()
{
	LOG(FATAL) << "ffmpeg fate error";
	stop();
}

void FFPlayer::setState(PlayerState state)
{
	LOG(INFO) << "setState" << (int)state;
	if (_state == state)	return;
    _state = state;
	emit stateChanged(_state);

	// 同步更新解码器状态
	if (_audioDecoderMgr->decoder()) {
        _audioDecoderMgr->decoder()->setDecoderState(state);
    }
}

void FFPlayer::readFile()
{
	_formatCtx = avformat_alloc_context();
	AVDictionary* opt = nullptr;
	av_dict_set(&opt, "rtsp_transport", "tcp", 0);
	av_dict_set(&opt, "stimeout", "60000000", 0);
	av_dict_set(&opt, "reconnect", "1", 0);
	av_dict_set(&opt, "reconnect_at_eof", "1", 0);
	av_dict_set(&opt, "reconnect_streamed", "1", 0);
	av_dict_set(&opt, "reconnect_delay_max", "5", 0);
	int ret = avformat_open_input(&_formatCtx, _filepath.toStdString().c_str(), nullptr, &opt);
	END(avformat_open_input);
	ret = avformat_find_stream_info(_formatCtx, nullptr);
    END(avformat_find_stream_info);
	if (_audioDecoderMgr->init(_formatCtx)) {
		if (!_audioDecoderMgr->start()) {
			LOG(FATAL) << "Audio decoder start failed";
		}
	} else {
		LOG(FATAL) << "Audio decoder initialization failed";
		return;
	}

	// 等待音频解码器初始化完成
	QEventLoop loop;
	QTimer timer;
	timer.setSingleShot(true);
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::initFinished, &loop, &QEventLoop::quit);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(2000);
	loop.exec();
	if (!timer.isActive()) {
		LOG(FATAL) << "timer timeout";
		stop();
		// emit PlayFinished();
		return;
	}

	// emit initFinished();
	int streamIndex = _audioDecoderMgr->decoder() ? _audioDecoderMgr->decoder()->getStreamIndex() : -1;
	LOG(INFO) << "streamIndex" << streamIndex;
	int64_t streamDuration = _formatCtx->streams[streamIndex]->duration;
	if (streamDuration == AV_NOPTS_VALUE) {
		streamDuration = av_rescale_q(_formatCtx->duration, { 1, AV_TIME_BASE }, _formatCtx->streams[streamIndex]->time_base);
	}

	AVPacket pkt;
	int64_t currentPts = 0;
	int errorCount = 0;
	const int MAX_RETRY = 50;

	const int64_t EOF_THRESHOLD = av_rescale_q(5000000, { 1, AV_TIME_BASE }, _formatCtx->streams[streamIndex]->time_base);
	while (_state != PlayerState::STOP) {
		if (_seek_pos >= 0) {
			AVRational avRational = { 1, AV_TIME_BASE };
			int64_t seek_target = _seek_pos / av_q2d(_formatCtx->streams[streamIndex]->time_base);
			if (av_seek_frame(_formatCtx, streamIndex, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
				LOG(ERROR) << "seek error";
			}
			else {
				if (_hasAudio) {
					AVPacket pkt_tmp;
					av_new_packet(&pkt_tmp, 10);
					snprintf((char*)pkt_tmp.data, 10, "%s", "FLUSH");
					_audioDecoderMgr->decoder()->clearPkt();
                    _audioDecoderMgr->decoder()->pushPkt(pkt_tmp);
				}
				errorCount = 0;
				currentPts = seek_target;
			}
            _seek_pos = -1;
		}
		ret = av_read_frame(_formatCtx, &pkt);
		if (ret == 0) {
			errorCount = 0;		// reset error counter on success
            if (pkt.stream_index == streamIndex) {
				if (pkt.pts != AV_NOPTS_VALUE) {
					currentPts = pkt.pts;
				}
				if (_hasAudio) {
					_audioDecoderMgr->decoder()->pushPkt(pkt);
				}
			}
			else {
				av_packet_unref(&pkt);
			}
		}
		else if (ret == AVERROR(EAGAIN) || ret == AVERROR(ETIMEDOUT) || ret == AVERROR(ECONNRESET)) {
			LOG(INFO) << "Network error occurred, attempting to reconnect...";
			// 等待避免频繁重连
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// TODO: 重连机制
			continue;
		}
		else if (ret == AVERROR_EOF) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //setState(PlayerState::STOP);
			LOG_EVERY_T(INFO, 0.5) << "read file finished!!!";
		}
		else {
			ERROR_BUF;
			LOG_EVERY_T(WARNING, 2) << "av_read_frame Error (Count:" << errorCount << "):" << errbuf << "result: " << ret;
			// judge whether in file end nearby
			bool isNearEnd = (streamDuration != AV_NOPTS_VALUE) &&
				(currentPts >= (streamDuration - EOF_THRESHOLD));
			if (isNearEnd) {
				LOG_EVERY_T(INFO, 2) << "Read error near end of stream (" << ret << "). Treating as normal EOF.";
				// We assume we are done. Just idle.
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}
			errorCount++;
			if (errorCount > MAX_RETRY) {
				LOG(ERROR) << "Stream ended (server closed connection or file truncated). Treating as EOF.";
				// setState(PlayerState::STOP);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
	}
	// emit PlayFinished();
}
