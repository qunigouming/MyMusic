#include "BottomPlayWidget.h"

BottomPlayWidget::BottomPlayWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BottomPlayWidgetClass())
{
    ui->setupUi(this);
    ui->play_model_btn->setText(QChar(0xe00b));
    ui->last_btn->setText(QChar(0xe010));
    ui->stopandplay_btn->setText(QChar(0xe00d));
    ui->next_btn->setText(QChar(0xe00f));
    ui->playlist_btn->setText(QChar(0xe015));
    ui->collect_btn->setText(QChar(0xe609));
    ui->volume_btn->setText(QChar(0xe013));
    ui->more_btn->setText(QChar(0xe61c));

    _player = new FFPlayer(this);
    playSigConnect();
    uiSigConnect();
}

BottomPlayWidget::~BottomPlayWidget()
{
    delete _player;
    qDebug() << "Delete player success";
	delete ui;
}

void BottomPlayWidget::play(const SongInfo& info)
{
    ui->music_icon->setPixmap(QPixmap(info.icon));
    ui->music_icon->startRotation();

    ui->music_title->setText(info.title);
    ui->music_author->setText(info.author);

    // 若不是停止状态则先停止再播放
    if (_player->getState() != PlayerState::STOP) {
        qDebug() << "正在播放" << info.title;
        _player->stop();
    }
    _current_SongPath = info.path;
    _player->play(_current_SongPath);
}

void BottomPlayWidget::informMainWindow()
{
}

void BottomPlayWidget::playSigConnect()
{
    // 连接播放器初始化信号
    connect(_player, &FFPlayer::initFinished, this, [this]() {
        _duration = _player->getDuration();
        // qDebug() << "duration:" << _duration;
        ui->play_progressbar->setRange(0, _duration);
        ui->play_progressbar->setTotalTime(_duration);
    });

    // 监听播放器时钟信号
    connect(_player, &FFPlayer::clockChanged, this, [this](int clock) {
        ui->play_progressbar->setValue(clock);
    });

    connect(_player, &FFPlayer::stateChanged, this, [this](PlayerState state) {
        switch (state) {
        case PlayerState::STOP:
            ui->stopandplay_btn->setText(QChar(0xe00d));
            break;
        case PlayerState::PLAYING:
            ui->stopandplay_btn->setText(QChar(0xe00E));
            break;
        case PlayerState::PAUSE:
            ui->stopandplay_btn->setText(QChar(0xe00d));
            break;
        }
    });

    // 监听播放器播放完成信号
    connect(_player, &FFPlayer::PlayFinished, this, [this] {
        // 当收到信号，FFPlayer已经清理完所有资源了，不用再调用stop清理一遍
        qDebug() << "播放完成";
        if (_playModel == PlayModel::LISTLOOP) {
            emit playNextSong();
        }
        else if (_playModel == PlayModel::SINGLELOOP) {
            _player->play(_current_SongPath);
        }
        // ui->stopandplay_btn->setText(QChar(0xe00d));
    });
}

void BottomPlayWidget::uiSigConnect()
{
    connect(ui->play_model_btn, &QPushButton::clicked, this, [this] {
        switch (_playModel) {
            case PlayModel::LISTLOOP:
                _playModel = PlayModel::SINGLELOOP;
                break;
            case PlayModel::SINGLELOOP:
                _playModel = PlayModel::RANDOM;
                break;
            case PlayModel::RANDOM:
                _playModel = PlayModel::SEQUENCE;
                break;
            case PlayModel::SEQUENCE:
                _playModel = PlayModel::LISTLOOP;
                break;
        }
    });
    connect(ui->last_btn, &QPushButton::clicked, this, [this] {
        emit playLastSong();
    });

    connect(ui->stopandplay_btn, &QPushButton::clicked, this, [this] {
        PlayerState state = _player->getState();
        if (state == PlayerState::PLAYING) {
            _player->pause();
        }
        else {
            _player->play();
        }
    });

    connect(ui->next_btn, &QPushButton::clicked, this, [this] {
        emit playNextSong();
    });

    connect(ui->play_progressbar, &ProgressBar::sliderReleased, _player, [this] {
        _player->seek(ui->play_progressbar->value());;
    });

    connect(ui->play_progressbar, &ProgressBar::sliderPressed, _player, [this] {
        _player->seek(ui->play_progressbar->value());
    });
}

