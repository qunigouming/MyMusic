#pragma once
#include "DecoderInterface.h"
#include <QAudioSink>
#include <QIODevice>
#include <QThread>

#include "AudioStreamProcessor.h"
#include "Equalizer.h"
#include "ToneControl.h"

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

	void setVolume(int volume) override;
	void switchAudioDevice();

public slots:
	void setDecoderState(PlayerState state) override;
	void updateBand(int index, float gain);
	void setEnvironment(int index);
	void setEnvDepthValue(int value);
	void setEnvIntensityValue(int value);
	void setBassLevel(int value);
	void setTrableLevel(int value);

	void onAudioOutputChanged();

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
	EnvironmentPreset _lastPreset = EnvironmentPreset::None;
	int _lastDepth = 0;
	int _lastStrength = 0;
	std::unique_ptr<AudioStreamProcessor>  _audioStreamProcessor = nullptr;
	std::unique_ptr<Equalizer10Band> _equalizer;
	std::unique_ptr<ToneControl> _toneControl;
	std::atomic_bool _needResetAudio = false;
};

