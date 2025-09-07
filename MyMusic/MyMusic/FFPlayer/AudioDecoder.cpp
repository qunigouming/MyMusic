#include "AudioDecoder.h"
#include <QAudioFormat>

AudioDecoder::AudioDecoder(AVFormatContext* fmtCtx, QObject* parent)
    : DecoderInterface(fmtCtx)
{
}

AudioDecoder::~AudioDecoder()
{
    free();
    qDebug() << "~AudioDecoder()";
}

bool AudioDecoder::init()
{
    int ret = initDecoder();
    if (ret < 0) {
        emit DecoderInterface::initFinished(false);
        return false;
    }
    ret = initSws();
    if (ret < 0) {
        emit DecoderInterface::initFinished(false);
        return false;
    }
    
    QAudioFormat format;
    format.setSampleRate(_outSpec.sampleRate);
    format.setChannelCount(_outSpec.channels);
    format.setSampleFormat(QAudioFormat::Int16);

    _audioSink = new QAudioSink(format);
    _audioDevice = _audioSink->start();

    emit DecoderInterface::initFinished(true);
    return true;
}

void AudioDecoder::free()
{
    _stream = nullptr;
    _clock = 0;
    _pktQueue.clear();
    if (_swrCtx) {
        swr_free(&_swrCtx);
        _swrCtx = nullptr;
    }
    if (_inFrame) {
        av_frame_free(&_inFrame);
        _inFrame = nullptr;
    }
    if (_outFrame) {
        av_frame_free(&_outFrame);
        _outFrame = nullptr;
    }

    if (_decodecCtx) {
        avcodec_close(_decodecCtx);
        _decodecCtx = nullptr;
    }
    if (_audioSink) {
        _audioSink->stop();
        _audioDevice->close();
        delete _audioSink;
        _audioDevice = nullptr;
        _audioSink = nullptr;
    }
}

int AudioDecoder::getStreamIndex()
{
    return _streamIndex;
}

void AudioDecoder::setDecoderState(PlayerState state)
{
    _state = state;
}

int AudioDecoder::initDecoder()
{
    int ret = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    RET(av_find_best_stream);
    _streamIndex = ret;
    _stream = _formatCtx->streams[ret];
    if (!_stream) {
        qDebug() << "Audio stream is empty!";
        return -1;
    }

    AVCodec* decoder = avcodec_find_decoder(_stream->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found" << _stream->codecpar->codec_id;
        return -1;
    }
    _decodecCtx = avcodec_alloc_context3(decoder);
    if (!_decodecCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }
    ret = avcodec_parameters_to_context(_decodecCtx, _stream->codecpar);
    RET(avcodec_parameters_to_context);
    ret = avcodec_open2(_decodecCtx, decoder, nullptr);
    RET(avcodec_open2);
    _decodecCtx->pkt_timebase = _stream->time_base;
    return 0;
}

int AudioDecoder::initSws()
{
    // 获取输入参数
    _inSpec.sampleRate = _decodecCtx->sample_rate;
    _inSpec.channels = _decodecCtx->channels;
    _inSpec.channelLayout = _decodecCtx->channel_layout;
    _inSpec.sampleFmt = _decodecCtx->sample_fmt;


    //设置输出参数
    _outSpec.sampleRate = 44100;
    _outSpec.sampleFmt = AV_SAMPLE_FMT_S16;
    _outSpec.channelLayout = AV_CH_LAYOUT_STEREO;
    _outSpec.channels = av_get_channel_layout_nb_channels(_outSpec.channelLayout);
    _outSpec.bytesPerSampleFrame = av_get_bytes_per_sample(_outSpec.sampleFmt) * _outSpec.channels;

    _swrCtx = swr_alloc_set_opts(nullptr,
        _outSpec.channelLayout,
        _outSpec.sampleFmt,
        _outSpec.sampleRate,
        _inSpec.channelLayout,
        _inSpec.sampleFmt,
        _inSpec.sampleRate,
        0,
        nullptr);

    if (!_swrCtx) {
        qDebug() << "swr_alloc_set_opts error";
        return -1;
    }

    int ret = swr_init(_swrCtx);
    RET(swr_init);

    _inFrame = av_frame_alloc();
    if (!_inFrame) {
        qDebug() << "av_frame_alloc input error";
        return -1;
    }

    _outFrame = av_frame_alloc();
    if (!_outFrame) {
        qDebug() << "av_frame_alloc output error";
        return -1;
    }

    ret = av_samples_alloc(_outFrame->data,
        _outFrame->linesize,
        _outSpec.channels,
        4096,
        _outSpec.sampleFmt,
        0);
    RET(av_samples_alloc);
    return 0;
}

void AudioDecoder::start()
{
    _state = PlayerState::PLAYING;
    while (true) {
        if (_state == PlayerState::STOP)    break;
        if (_pktQueue.empty() || _state == PlayerState::PAUSE) {
            if (_state == PlayerState::STOP)    break;
            QThread::msleep(1);
            qDebug() << "wait new packet" << _pktQueue.size();
            continue;
        }
        AVPacket pkt;
        _pktQueue.pop(pkt);

        // 收到这个数据，代表刚刚执行过seek，需要清空解码器数据
        if (strcmp((char*)pkt.data, "FLUSH") == 0) {
            avcodec_flush_buffers(_decodecCtx);
            av_packet_unref(&pkt);
            continue;
        }

        // 获取时间戳
        if (pkt.pts != AV_NOPTS_VALUE) {
            _clock = av_q2d(_stream->time_base) * pkt.pts;
            // qDebug() << "audio clock:" << static_cast<int>(std::floor(_clock));
            emit DecoderInterface::clockChanged(_clock);
        }

        int ret = avcodec_send_packet(_decodecCtx, &pkt);
        av_packet_unref(&pkt);
        CONTINUE(avcodec_send_packet);
        ret = avcodec_receive_frame(_decodecCtx, _inFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)   CONTINUE(avcodec_receive_frame);
        int outSamples = av_rescale_rnd(_outSpec.sampleRate, _inFrame->nb_samples, _inSpec.sampleRate, AV_ROUND_UP);
        ret = swr_convert(_swrCtx,
            _outFrame->data,
            outSamples,
            (const uint8_t**) _inFrame->data,
            _inFrame->nb_samples);
        BREAK(swr_convert);
        
        int size = ret * _outSpec.bytesPerSampleFrame;
        while (_audioSink->bytesFree() < size && _state != PlayerState::STOP) {
            QThread::msleep(1);
        }
        if (_state == PlayerState::STOP)    break;
        _audioDevice->write((char*)_outFrame->data[0], size);
    }
    qDebug() << "AudioDecoder::start() end";
    emit DecoderInterface::decodingFinished();
}
