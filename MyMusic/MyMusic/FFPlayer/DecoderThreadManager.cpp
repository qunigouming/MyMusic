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

    _decoder.reset(_decoderFactory(fmtCtx, this));

    connect(_decoder.get(), &DecoderInterface::initFinished, this, &DecoderThreadManager::initFinished, Qt::QueuedConnection);
    connect(_decoder.get(), &DecoderInterface::clockChanged, this, &DecoderThreadManager::clockChanged, Qt::QueuedConnection);
    connect(_decoder.get(), &DecoderInterface::decodingFinished, this, &DecoderThreadManager::onDecodingFinished, Qt::QueuedConnection);

    return true;
}

bool DecoderThreadManager::start()
{
    if (!_decoder)  return false;

    _thread = std::make_unique<QThread>();
    _decoder->moveToThread(_thread.get());

    connect(_thread.get(), &QThread::started, _decoder.get(), [this] {
        std::lock_guard<std::mutex> lock(_mutex);
        _decoder->init();
        _decoder->start();
    });
    connect(_decoder.get(), &DecoderInterface::decodingFinished, _thread.get(), &QThread::quit);
    connect(_thread.get(), &QThread::finished, _thread.get(), &QObject::deleteLater);

    _thread->start();
    return true;
}

void DecoderThreadManager::stop()
{
    if (_thread && _thread->isRunning()) {
        _thread->quit();
        _thread->wait();
    }
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _decoder.reset();
    }
    try {
        _thread.reset();
    }
    catch (...) {
        qDebug() << "DecoderThreadManager::stop() - Exception";
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


void DecoderThreadManager::onDecodingFinished()
{
    stop();
}