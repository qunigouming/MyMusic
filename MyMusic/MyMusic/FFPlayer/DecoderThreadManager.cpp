#include "DecoderThreadManager.h"

DecoderThreadManager::DecoderThreadManager(std::function<DecoderInterface* (AVFormatContext*, QObject*)> decoderFactory, QObject* parent)
    : QObject(parent), _decoderFactory(decoderFactory)
{
}

DecoderThreadManager::~DecoderThreadManager()
{
    stop();
}

bool DecoderThreadManager::init(AVFormatContext* fmtCtx)
{
    if (!fmtCtx || !_decoderFactory)    return false;

    if (_decoder) {
        _decoder.reset();
    }

    _decoder.reset(_decoderFactory(fmtCtx, this));
    if (_decoder) {
        qDebug() << "decoder init success";
    }
    else {
        qDebug() << "decoder init failed";
    }
    connect(_decoder.get(), &DecoderInterface::initFinished, this, &DecoderThreadManager::initFinished, Qt::QueuedConnection);
    connect(_decoder.get(), &DecoderInterface::clockChanged, this, &DecoderThreadManager::clockChanged, Qt::QueuedConnection);
    //connect(_decoder.get(), &DecoderInterface::decodingFinished, this, [this] {
    //    qDebug() << "decoding finished";
    //    stop();
    //}, Qt::QueuedConnection);
    //connect(_decoder.get(), &DecoderInterface::decodingFinished, this, &DecoderThreadManager::onDecodingFinished, Qt::QueuedConnection);

    return true;
}

bool DecoderThreadManager::start()
{
    if (!_decoder)  return false;

    if (_thread && _thread->isRunning()) {
        _thread->quit();
        _thread->wait();
    }

    _thread = std::make_unique<QThread>();
    _decoder->moveToThread(_thread.get());
    qDebug() << "DecoderThreadManager::start";
    connect(_thread.get(), &QThread::started, _decoder.get(), [this] {
        std::lock_guard<std::mutex> lock(_mutex);
        _decoder->init();
        qDebug() << "DecoderThreadManager::start::started";
        _decoder->start();
    }, Qt::QueuedConnection);
    connect(_decoder.get(), &DecoderInterface::decodingFinished, _decoder.get(), [this] {
        _decoder.reset();
    });
    connect(_thread.get(), &QThread::finished, _thread.get(), [this] {
        qDebug() << "线程结束";
        _thread.reset();
    });

    _thread->start();
    return true;
}

void DecoderThreadManager::stop()
{
    qDebug() << "DecoderThreadManager::stop";
    if (_thread && _thread->isRunning()) {

        _thread->quit();
        // 2秒超时等待
        if (!_thread->wait(5000)) {
            _thread->terminate();
            qDebug() << "线程超时";
            _thread->wait();
        }
        _thread.reset();
    }
}

DecoderInterface* DecoderThreadManager::decoder() const
{
    return _decoder.get();
}

bool DecoderThreadManager::isRunning() const
{
    return _thread && _thread->isRunning();
}

void DecoderThreadManager::setVolume(int volume)
{
    if (!_decoder)  return;
    _decoder->setVolume(volume);
}


void DecoderThreadManager::onDecodingFinished()
{
    stop();
}