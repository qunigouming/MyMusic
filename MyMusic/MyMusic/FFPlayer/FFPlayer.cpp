#include "FFPlayer.h"
#include <QEventLoop>
#include "FFPlayer/AudioDecoder.h"
#include <cstringt.h>
#include <QTimer>

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
			qDebug() << "Audio decoder initialization failed";
		}
		qDebug() << "Audio decoder initialization success";
		emit initFinished();
	});
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::clockChanged, this, [this](double clock) {
		emit clockChanged(clock);
	});

	connect(this, &FFPlayer::volumeChanged, _audioDecoderMgr.get(), &DecoderThreadManager::setVolume);
}

FFPlayer::~FFPlayer()
{
	qDebug() << "~FFPlayer";
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
	qDebug() << "free FFPlayer";
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
	qDebug() << "ffmpeg fate error";
	stop();
}

void FFPlayer::setState(PlayerState state)
{
	qDebug() << "setState" << (int)state;
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
	int ret = avformat_open_input(&_formatCtx, _filepath.toStdString().c_str(), nullptr, &opt);
	END(avformat_open_input);
	ret = avformat_find_stream_info(_formatCtx, nullptr);
    END(avformat_find_stream_info);
	if (_audioDecoderMgr->init(_formatCtx)) {
		if (!_audioDecoderMgr->start()) {
			qDebug() << "Audio decoder start failed";
		}
	} else {
		qDebug() << "Audio decoder initialization failed";
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
		qDebug() << "timer timeout";
		stop();
		// emit PlayFinished();
		return;
	}

	// emit initFinished();
	int streamIndex = _audioDecoderMgr->decoder() ? _audioDecoderMgr->decoder()->getStreamIndex() : -1;
	qDebug() << "streamIndex" << streamIndex;
	AVPacket pkt;
	while (_state != PlayerState::STOP) {
		if (_seek_pos >= 0) {
			AVRational avRational = { 1, AV_TIME_BASE };
			int64_t seek_target = _seek_pos / av_q2d(_formatCtx->streams[streamIndex]->time_base);
			if (av_seek_frame(_formatCtx, streamIndex, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
				qDebug() << "seek error";
			}
			else {
				if (_hasAudio) {
					AVPacket pkt_tmp;
					av_new_packet(&pkt_tmp, 10);
					snprintf((char*)pkt_tmp.data, 10, "%s", "FLUSH");
					_audioDecoderMgr->decoder()->clearPkt();
                    _audioDecoderMgr->decoder()->pushPkt(pkt_tmp);
				}
			}
			// qDebug() << "seek to: " << _seek_pos;
            _seek_pos = -1;
		}
		ret = av_read_frame(_formatCtx, &pkt);
		if (ret == 0) {
            if (_hasAudio && pkt.stream_index == streamIndex) {
				_audioDecoderMgr->decoder()->pushPkt(pkt);
			}
			else {
				av_packet_unref(&pkt);
			}
		}
		else if (ret == AVERROR(EAGAIN) || ret == AVERROR(ETIMEDOUT) || ret == AVERROR(ECONNRESET)) {
			qDebug() << "Network error occurred, attempting to reconnect...";
			// 等待避免频繁重连
			std::this_thread::sleep_for(std::chrono::seconds(2));
			// TODO: 重连机制
			continue;
		}
		else if (ret == AVERROR_EOF) {
            //setState(PlayerState::STOP);
			// qDebug() << "read file finished!!!";
		}
		else {
			ERROR_BUF;
			qDebug() << "av_read_frame Error" << errbuf;
			setState(PlayerState::STOP);
			continue;
		}
	}
	// emit PlayFinished();
}
