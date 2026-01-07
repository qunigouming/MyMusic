#include "BottomPlayWidget.h"
#include <QMouseEvent>
#include "LocalDataManager.h"


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
    _volumeWidget = QSharedPointer<VolumeWidget>(new VolumeWidget);
    ui->volume_btn->installEventFilter(this);
    _audioEffectDialog = QSharedPointer<AudioEffectDialog>(new AudioEffectDialog);
    connect(_volumeWidget.get(), &VolumeWidget::volumeChanged, this, [this](int volume) {
        _player->setVolume(volume);
        LocalDataManager::GetInstance()->setVolume(volume);
        setVolumeUI(volume);
    });

    connect(ui->volume_btn, &QPushButton::clicked, [this](bool checked) {
        if (checked) _volumeWidget->mute();
        else _volumeWidget->unMute();
    });
    connect(ui->more_btn, &QPushButton::clicked, this, [this] {
        _audioEffectDialog->show();
    });
    _player = new FFPlayer(this);
    playSigConnect();
    uiSigConnect();
    _volumeWidget->setVolume(LocalDataManager::GetInstance()->getVolume());
    connect(_audioEffectDialog.get(), &AudioEffectDialog::EQValueChanged, _player, &FFPlayer::updateBand);
    connect(_audioEffectDialog.get(), &AudioEffectDialog::envComboChanged, _player, &FFPlayer::setEnvironment);
    
}

BottomPlayWidget::~BottomPlayWidget()
{
    delete _player;
    qDebug() << "Delete player success";
	delete ui;
}

void BottomPlayWidget::play(const SongInfo& info)
{
    if (info.icon.isNull()) {
        // 网络图片加载
        ui->music_icon->setPixmap(info.icon_url);
    }
    else {
        ui->music_icon->setPixmap(QPixmap(info.icon));
    }
    ui->music_icon->startRotation();

    ui->music_title->setText(info.title);
    ui->music_author->setText(info.author);

    _current_SongPath = info.path;
    _player->play(_current_SongPath);
}

bool BottomPlayWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->volume_btn) {
        if (event->type() == QEvent::Enter) {
            // qDebug() << "Enter volume button";
            QPoint target = ui->volume_btn->mapToGlobal(QPoint(0, 0));
            QSize size = _volumeWidget->size();
            _volumeWidget->move(target.x(), target.y() - size.height() + 15);
            _volumeWidget->show();
            return false;
        }

        if (event->type() == QEvent::Leave) {
            // qDebug() << "Leave volume button";
            if (!_volumeWidget->geometry().contains(QCursor::pos()))
                _volumeWidget->hide();
            return false;
        }
    }
    return QWidget::eventFilter(obj, event);
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
    connect(_player, &FFPlayer::clockChanged, this, [this](double clock) {
        int s = static_cast<int>(std::floor(clock));
        ui->play_progressbar->setValue(s);
        if (s == _duration) {
            _player->stop();
            emit playNextSong(_playModel);
        }
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
}

void BottomPlayWidget::uiSigConnect()
{
    connect(ui->play_model_btn, &QPushButton::clicked, this, [this] {
        switch (_playModel) {
            case PlayModel::LISTLOOP:
                ui->play_model_btn->setText(QChar(0xe00c));
                _playModel = PlayModel::SINGLELOOP;
                break;
            case PlayModel::SINGLELOOP:
                ui->play_model_btn->setText(QChar(0xe00a));
                _playModel = PlayModel::RANDOM;
                break;
            case PlayModel::RANDOM:
                ui->play_model_btn->setText(QChar(0xe00b));
                _playModel = PlayModel::LISTLOOP;
                break;
        }
        emit playModelChanged(_playModel);
    });
    connect(ui->last_btn, &QPushButton::clicked, this, [this] {
        emit playLastSong(_playModel);
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
        emit playNextSong(_playModel);
    });

    connect(ui->play_progressbar, &ProgressBar::sliderReleased, _player, [this] {
        _player->seek(ui->play_progressbar->value());;
    });

    connect(ui->play_progressbar, &ProgressBar::sliderPressed, _player, [this] {
        _player->seek(ui->play_progressbar->value());
    });
}

void BottomPlayWidget::setVolumeUI(int volume)
{
    if (volume == 0) ui->volume_btn->setText(QChar(0xe011));
    else if (volume > 0 && volume <= 30) ui->volume_btn->setText(QChar(0xe012));
    else if (volume > 30 && volume <= 60) ui->volume_btn->setText(QChar(0xe013));
    else ui->volume_btn->setText(QChar(0xe014));
}

