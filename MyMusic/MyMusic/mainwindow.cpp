#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QScreen>
#include <QDebug>
#include <QColor>
#include <algorithm>
#include "funclistwidgetitem.h"
#include "Tool/FileInspectorImp.h"
#include <QJsonDocument>
#include "UploadWidget.h"
#include <QFileDialog>
#include "UserManager.h"
#include "tcpmanager.h"
#include <QMessageBox>
#include "LocalDataManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    //initialization icon-font
    ui->searchBtn->setText(QChar(0xe005));

    ui->perletBtn->setText(QChar(0xe006));
    ui->setBtn->setText(QChar(0xe61c));
    ui->complexionBtn->setText(QChar(0xe009));
    srand(time(NULL));
    ui->minimizeBtn->setText(QChar(0xe650));
    ui->zoomBtn->setText(QChar(0xe65d));
    ui->closeBtn->setText(QChar(0xe67d));
    ui->TitleWidget->installEventFilter(this);
    ui->mainBtn->setIconSize(QSize(25, 25));
    compWidget = new complexionWidget(this);
    compWidget->hide();
    connect(compWidget, &complexionWidget::changeColor, this, &MainWindow::changeSkinColor);

    _am_view = new TableView(MusicTableViewType::NET_MODEL, this);
    ui->all_music_widget->layout()->addWidget(_am_view);

    _lm_view = new TableView(MusicTableViewType::LOCAL_MODEL, this);
    ui->local_music_view->layout()->addWidget(_lm_view);

    initBaseFuncLWg();

    ui->headIcon->setPixmap(QPixmap(UserManager::GetInstance()->getIcon()).scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->userLabel->setText(UserManager::GetInstance()->getName());

    // 将音乐列表添加到TableView中
    auto musicList = UserManager::GetInstance()->getMusicList();
    for (auto& music : musicList) {
        _am_view->addSong(SongInfo(music.get()));
    }
    bindConntoView(_am_view);
    bindConntoView(_lm_view);

    connect(_am_view, &TableView::likeChanged, this, [this](int id, bool status) {
        QJsonObject jsonObj;
        jsonObj["fromUid"] = UserManager::GetInstance()->getUid();
        jsonObj["song_id"] = id;
        jsonObj["status"] = status; // 0:取消收藏 1:收藏
        jsonObj["flag"] = DEFAULT_COLLECT_SONGLIST;     // 默认收藏歌单or其他类型歌单
        QJsonDocument doc(jsonObj);
        emit TcpManager::GetInstance()->sig_send_data(ReqID::ID_COLLECT_SONG_REQ, doc.toJson());
    });

    connect(TcpManager::GetInstance().get(), &TcpManager::sig_con_status, [&](bool status) {
        if (!status) {
            QMessageBox::warning(this, "连接服务器", "服务器连接失败");
        }
    });

    connect(ui->upload_Btn, &QPushButton::clicked, this, [this]() {
        qDebug() << "Upload clicked";
        UploadWidget* uploadWidget = new UploadWidget();
        uploadWidget->show();
    });

    _heartbeatTimer = new QTimer(this);
    connect(_heartbeatTimer, &QTimer::timeout, this, [this]() {
        auto userInfo = UserManager::GetInstance()->getUserInfo();
        QJsonObject jsonObj;
        jsonObj["fromuid"] = userInfo->uid;
        QJsonDocument doc(jsonObj);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        emit TcpManager::GetInstance()->sig_send_data(ReqID::ID_HEARTBEAT_REQ, data);
    });

    _heartbeatTimer->start(10000);

    // 映射表
    _stackWidgetPageMap[SidebarItemType::MainPage] = StackWidgetPage::MainPage;
    _stackWidgetPageMap[SidebarItemType::AIChat] = StackWidgetPage::ChatPage;
    _stackWidgetPageMap[SidebarItemType::FavoriteMusic] = StackWidgetPage::SongListPage;
    _stackWidgetPageMap[SidebarItemType::MyCollection] = StackWidgetPage::None;
    _stackWidgetPageMap[SidebarItemType::LatestPlay] = StackWidgetPage::None;
    _stackWidgetPageMap[SidebarItemType::LocalMusic] = StackWidgetPage::LocalMusicPage;
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->TitleWidget) {
        static QPoint dragPosition;
        //鼠标按下时记录位置
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos() - parentWidget()->geometry().topLeft();      //记录鼠标全局位置
#else
                dragPosition = e->globalPosition().toPoint() - parentWidget()->geometry().topLeft();      //记录鼠标全局位置
#endif
                return true;
            }
        }
        //鼠标移动并且左键按下，移动窗口
        if (event->type() == QEvent::MouseMove){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                parentWidget()->move(e->globalPos() - dragPosition);
#else
                parentWidget()->move(e->globalPosition().toPoint() - dragPosition);
#endif
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    opt.init(this);
#else
    opt.initFrom(this);
#endif
    QPainter p(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MainWindow::on_closeBtn_clicked()
{
    qApp->exit(0);
}


void MainWindow::on_minimizeBtn_clicked()
{
    parentWidget()->showMinimized();
}

void MainWindow::on_zoomBtn_clicked(bool checked)
{
    if (checked) {
        parentWidget()->showMaximized();
        ui->zoomBtn->setText(QChar(0xe692));
    } else {
        parentWidget()->showNormal();
        ui->zoomBtn->setText(QChar(0xe65d));
    }
}

void MainWindow::initBaseFuncLWg()
{
    connect(ui->funcListWid, &SidebarWidget::itemClicked, this, [this](int index) {
        if (index == static_cast<int>(SidebarItemType::MainPage)) {
            // 主页
            ui->stackedWidget->setCurrentIndex(static_cast<int>(_stackWidgetPageMap[SidebarItemType::MainPage]));
        }
        else if (index == static_cast<int>(SidebarItemType::AIChat)) {
            // AI聊天
            ui->stackedWidget->setCurrentIndex(static_cast<int>(_stackWidgetPageMap[SidebarItemType::AIChat]));
        }
        else if (index == static_cast<int>(SidebarItemType::FavoriteMusic)) {
            // 发送获取喜欢的音乐请求
            QJsonObject jsonObj;
            jsonObj["fromUid"] = UserManager::GetInstance()->getUid();
            jsonObj["flag"] = DEFAULT_COLLECT_SONGLIST;
            QJsonDocument doc(jsonObj);
            TcpManager::GetInstance()->sig_send_data(ReqID::ID_GET_COLLECT_SONG_LIST_INFO_REQ, doc.toJson());
            // 喜欢的音乐
            ui->stackedWidget->setCurrentIndex(static_cast<int>(_stackWidgetPageMap[SidebarItemType::FavoriteMusic]));
            _activeView = _am_view;
        }
        else if (index == static_cast<int>(SidebarItemType::LocalMusic)) {
            // 本地音乐
            ui->stackedWidget->setCurrentIndex(static_cast<int>(_stackWidgetPageMap[SidebarItemType::LocalMusic]));
            _activeView = _lm_view;
            readLocalMusicConfig();
        }
    });
}

void MainWindow::scanFileToTableView(QStringList list)
{
    QThread* thread = new QThread;
    FileInspectorImp* inspector = new FileInspectorImp;
    inspector->moveToThread(thread);
    _lm_view->clearAllSongs();
    connect(inspector, &FileInspectorImp::filesFound, this, [this](const QList<FileInfo>& files){
        for (auto& file : files) {
            QString name = file.title;
            QString author = file.artist;
            QString duration = file.duration;
            QString file_size = file.size;
            QString path = file.filePath;
            QPixmap cover = file.cover;
            QString album = file.album;
            _lm_view->addSong(SongInfo(name, duration, file_size, path, cover, author, album));
        }
    });
    connect(inspector, &FileInspectorImp::scanFinished, this, [this, thread](){
        qDebug() << "scan finished";
        thread->quit();
    });
    connect(_lm_view, &TableView::allSongsAdded, this, [this]() {
        ui->total_music_Lab->setText("共 " + QString::number(_lm_view->rowCount()) + " 首");
    });
    connect(thread, &QThread::finished, inspector, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
    inspector->startScan(list, "*.mp3");
}

float MainWindow::calculateLuminace(const QColor &color)
{
    return 0.2126f * color.red() + 0.7152f * color.green() + 0.0722f * color.blue();
}

void MainWindow::calculateColors(const QColor& showcaseColor, QColor &listColor, QColor &uiColor, QColor &bottomColor)
{
    float y = calculateLuminace(showcaseColor);
    //qDebug() << y;
    bool isLight = y > 127.5f;
    if (isLight) {
        listColor.setRed(std::max(showcaseColor.red() - 15, 0));
        listColor.setGreen(std::max(showcaseColor.green() - 12, 0));
        listColor.setBlue(std::max(showcaseColor.blue() - 9, 0));

        uiColor.setRed(std::max(showcaseColor.red() - 8, 0));
        uiColor.setGreen(std::max(showcaseColor.green() - 6, 0));
        uiColor.setBlue(std::max(showcaseColor.blue() - 3, 0));

        bottomColor.setRed(std::min(showcaseColor.red() + 5, 255));
        bottomColor.setGreen(std::min(showcaseColor.green() + 5, 255));
        bottomColor.setBlue(std::min(showcaseColor.blue() + 5, 255));
    } else {
        listColor.setRed(std::min(showcaseColor.red() + 30, 255));
        listColor.setGreen(std::min(showcaseColor.green() + 30, 255));
        listColor.setBlue(std::min(showcaseColor.blue() + 30, 255));

        uiColor.setRed(std::min(showcaseColor.red() + 14, 255));
        uiColor.setGreen(std::min(showcaseColor.green() + 14, 255));
        uiColor.setBlue(std::min(showcaseColor.blue() + 14, 255));

        bottomColor.setRed(std::min(showcaseColor.red() + 40, 255));
        bottomColor.setGreen(std::min(showcaseColor.green() + 40, 255));
        bottomColor.setBlue(std::min(showcaseColor.blue() + 42, 255));
    }
}

void MainWindow::readLocalMusicConfig()
{
    scanFileToTableView(LocalDataManager::GetInstance()->getFilePaths());
}

void MainWindow::bindConntoView(TableView* view)
{
    connect(view, &TableView::rowDoubleClicked, this, [this, view](const SongInfo& info) {
        qDebug() << "Receive rowDoubleClicked";
        ui->bottomWid->play(info);
        _currentPlayIndex = view->currentIndex().row();     // 更新播放索引
    });
    connect(ui->bottomWid, &BottomPlayWidget::playNextSong, this, [this, view](PlayModel model) {
        qDebug() << "Receive playNextSong";
        if (_activeView != view)    return;     // 只处理当前视图
        if (view->rowCount() == 0) return;
        if (_currentPlayIndex == -1) return;

        // 通过不同播放模式获取下一首歌曲索引
        int nextIndex = 0;
        if (model == PlayModel::LISTLOOP) {
            nextIndex = _currentPlayIndex + 1;
        }
        else if (model == PlayModel::SINGLELOOP) {
            nextIndex = _currentPlayIndex;
        }
        else if (model == PlayModel::RANDOM) {
            // 通过随机数去生成下一首歌曲索引
            // 若当前索引和下一首索引相同，则重新生成下一首索引
            do {
                nextIndex = rand() % view->rowCount();
                qDebug() << "nextIndex" << nextIndex;
            } while (nextIndex == _currentPlayIndex);
        }

        if (nextIndex >= view->rowCount()) {
            nextIndex = 0;
        }

        _currentPlayIndex = nextIndex;
        SongInfo info = view->getSongInfoByProxyRow(nextIndex);
        ui->bottomWid->play(info);
        // 更新选中行
        view->setCurrentIndex(view->model()->index(nextIndex, 0));
    });
}

void MainWindow::on_complexionBtn_clicked()
{
    QPoint popupPos = ui->complexionBtn->mapToGlobal(QPoint(0, 30));
    popupPos.setX(popupPos.x() - (compWidget->width() >> 1));
    compWidget->move(popupPos);
    compWidget->show();
}

void MainWindow::changeSkinColor(QColor showcaseColor)
{
    //calculate all color
    QColor listColor, uiColor, bottomColor;
    calculateColors(showcaseColor, listColor, uiColor, bottomColor);
    //qDebug() << "change the color" << showcaseColor << listColor << uiColor << bottomColor;
    //set new color
    setStyleSheet(QString(R"(* { background-color: rgb(%1, %2, %3); })").arg(uiColor.red()).arg(uiColor.green()).arg(uiColor.blue()));
    ui->leftListWid->setStyleSheet(QString(R"(* { background-color: rgb(%1, %2, %3); })").arg(listColor.red()).arg(listColor.green()).arg(listColor.blue()));
    ui->bottomWid->setStyleSheet(QString(R"(* { background-color: rgb(%1, %2, %3); })").arg(bottomColor.red()).arg(bottomColor.green()).arg(bottomColor.blue()));
    compWidget->setStyleSheet(QString(R"(#complexionWidget {
                                    border: none;
                                    border-radius: 30px;
                                    background-color: rgb(%1, %2, %3);})").arg(bottomColor.red()).arg(bottomColor.green()).arg(bottomColor.blue()));
}

void MainWindow::on_setBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(static_cast<int>(StackWidgetPage::SetPage));
}


void MainWindow::on_selectDir_Btn_clicked()
{
    selectlocmusic_dlg = new SelectLocMusic_Dlg(this);

    // 获取扫描文件夹路径
    connect(selectlocmusic_dlg, &SelectLocMusic_Dlg::sig_selectDir, [this](const QStringList& path) {
        scanFileToTableView(path);
    });
    selectlocmusic_dlg->show();
}

