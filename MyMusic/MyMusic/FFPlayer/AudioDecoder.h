#pragma once
#include "DecoderInterface.h"
#include <QAudioSink>
#include <QIODevice>
#include <QThread>

struct SwrSpec {
	int sampleRate;			// 音频采样率
	int channels;			// 声道数
	int channelLayout;		// 声道布局
    enum AVSampleFormat sampleFmt;	// 音频采样格式
    int bytesPerSampleFrame;		// 音频采样字节数
};

class AudioDecoder : public DecoderInterface
{
	Q_OBJECT
public:
	explicit AudioDecoder(AVFormatContext* fmtCtx, QObject* parent = nullptr);
	virtual ~AudioDecoder();
	virtual bool init() override;

	virtual void free() override;

	int getStreamIndex() override;

public slots:
	void setDecoderState(PlayerState state) override;

signals:
	void initFinished(bool success);
	void decodingFinished();

protected:
	virtual int initDecoder() override;
	virtual int initSws() override;

	void start() override;
private:
	SwrContext* _swrCtx = nullptr;
    SwrSpec _inSpec, _outSpec;

	AVFrame* _inFrame = nullptr, *_outFrame = nullptr;

	// 同步变量
	double _clock = 0;

	// 音频输出
	QAudioSink* _audioSink = nullptr;
    QIODevice* _audioDevice = nullptr;
};

