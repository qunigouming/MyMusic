#include "FFPlayer.h"
#include <QEventLoop>
#include "FFPlayer/AudioDecoder.h"
#include <cstringt.h>

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
		emit initFinished();
	});
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::clockChanged, this, [this](int clock) {
		emit clockChanged(clock);
	});
}

FFPlayer::~FFPlayer()
{
	qDebug() << "~FFPlayer";
	stop();
}

void FFPlayer::play(const QString& filepath)
{
	if (!filepath.isEmpty()) _filepath = filepath;
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
	setState(PlayerState::STOP);
	if (_audioDecoderMgr->decoder())
		_audioDecoderMgr->stop();
}

void FFPlayer::pause()
{
	if (_state != PlayerState::PLAYING)	return;
	setState(PlayerState::PAUSE);
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

void FFPlayer::fateError()
{
	qDebug() << "ffmpeg fate error";
	free();
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

void FFPlayer::free()
{
	qDebug() << "free FFPlayer";
	// 清空音频相关资源
	stop();

	// 清空视频相关资源
    if (_hasVideo) {

	}

	_hasVideo = false;

	if (_formatCtx) {
        avformat_close_input(&_formatCtx);
        avformat_free_context(_formatCtx);
        _formatCtx = nullptr;
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
		_audioDecoderMgr->start();
	} else {
		qDebug() << "Audio decoder initialization failed";
		return;
	}
	QEventLoop loop;
	connect(_audioDecoderMgr.get(), &DecoderThreadManager::initFinished, &loop, &QEventLoop::quit);
	loop.exec();
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
		else if (ret == AVERROR_EOF) {
            //setState(PlayerState::STOP);
			// qDebug() << "read file finished!!!";
		}
		else {
			ERROR_BUF;
			qDebug() << "av_read_frame Error" << errbuf;
			continue;
		}
	}
	qDebug() << "play music finished";
	free();
	emit PlayFinished();
}
