#pragma once
#include <QThread>
#include <memory>
#include "FFPlayer/DecoderInterface.h"
#include <mutex>

class DecoderThreadManager : public QObject
{
    Q_OBJECT
public:
	explicit DecoderThreadManager(
        std::function<DecoderInterface*(AVFormatContext*, QObject*)> decoderFactory,
        QObject* parent = nullptr);
    ~DecoderThreadManager();
    bool init(AVFormatContext* fmtCtx);
    bool start();           // 开启线程，创建资源
    void stop();            // 停止线程，清空资源

    DecoderInterface* decoder() const;

    bool isRunning() const;

signals:
    void initFinished(bool success);        // 解码器初始化完成信号
    void clockChanged(int clock);

private slots:
    void onDecodingFinished();

private:
    std::function<DecoderInterface* (AVFormatContext*, QObject*)> _decoderFactory;
    std::unique_ptr<DecoderInterface> _decoder;
    std::unique_ptr<QThread> _thread;

    std::mutex _mutex;
};
