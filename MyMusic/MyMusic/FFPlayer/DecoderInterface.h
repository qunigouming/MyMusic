#pragma once

#include <QDebug>
#include <QObject>

#include "Common/blockqueue.h"
#include "PlayStatePrivate.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include <libswresample\swresample.h>
#include <libswscale\swscale.h>
}

#define ERROR_BUF \
char errbuf[AV_ERROR_MAX_STRING_SIZE]; \
av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);

#define CODE(func, code) \
if (ret < 0) { \
	ERROR_BUF;	\
	qDebug() << __LINE__ << #func << "error" << errbuf; \
	code; \
}

#define END(func) CODE(func, fateError(); return;);
#define RET(func) CODE(func, return ret;);
#define CONTINUE(func) CODE(func, continue;);
#define BREAK(func) CODE(func, break;);

class DecoderInterface : public QObject
{
	Q_OBJECT
public:
	explicit DecoderInterface(AVFormatContext* fmtCtx, QObject* parent = nullptr)
		: _formatCtx(fmtCtx), QObject(parent), _pktQueue(1000, [](AVPacket& pkt) { av_packet_unref(&pkt); }) {}
	virtual ~DecoderInterface() {
		qDebug() << "~DecoderInterface()";
	};
	virtual bool init() = 0;

	virtual void free() = 0;

	virtual int getStreamIndex() = 0;

	virtual void pushPkt(const AVPacket& pkt) {
        _pktQueue.push_back(pkt);
	}

	virtual void clearPkt() {
        _pktQueue.clear();
	}

signals:
	void initFinished(bool success);
	void decodingFinished();
	void clockChanged(double clock);
public slots:
	virtual void setDecoderState(PlayerState state) = 0;
	virtual void start() = 0;

protected:
	virtual int initDecoder() = 0;
	virtual int initSws() = 0;

protected:

	AVFormatContext *_formatCtx = nullptr;
    AVCodecContext *_decodecCtx = nullptr;

	AVStream* _stream = nullptr;

	BlockQueue<AVPacket> _pktQueue;

	int _streamIndex = -1;

	std::atomic<PlayerState> _state = PlayerState::STOP;
};

