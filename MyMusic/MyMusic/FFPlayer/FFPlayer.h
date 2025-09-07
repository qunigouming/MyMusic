#pragma once

#include <QString>
#include <QObject>

#include "FFPlayer/DecoderThreadManager.h"

class FFPlayer : public QObject
{
    Q_OBJECT
public:
    explicit FFPlayer(QObject* parent = nullptr);
    ~FFPlayer();

    void play(const QString& filepath);          // 用于最开始播放
    void play();        // 通常用于暂停后继续播放
    void stop();        // 停止播放
    void pause();       // 暂停播放

    PlayerState getState();

    int getDuration();

public slots:
    void seek(int clock);

signals:
    void stateChanged(PlayerState state); // 状态变化信号，用于控制播放器状态
    void initFinished();                // 初始化成功信号
    void PlayFinished();                // 播放完毕信号
    void clockChanged(double clock);    // 时钟同步信号

private:
    void fateError();
    void setState(PlayerState state);

    void readFile();

private:
    AVFormatContext *_formatCtx = nullptr;

    std::unique_ptr<DecoderThreadManager> _audioDecoderMgr;        // 音频解码器

    QString _filepath;

    PlayerState _state = PlayerState::STOP;

    bool _hasAudio = false;
    bool _hasVideo = false;

    int _seek_pos = -1;
};

