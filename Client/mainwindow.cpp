/*
 *双击列表项后，
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QBuffer>


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/favicon.ico"));
    setWindowTitle("My音乐");
    initWindow();
    initTitle();
    initVolume();
    initMediaPlay();
    connect(m_tcp,&TcpSocket::MusicListRequest,this,&MainWindow::GetMusicList);
    connect(m_tcp,&TcpSocket::MusicOpenRequest,this,&MainWindow::MusicPlayer);          //接收到数据执行音乐播放操作
    //connect(ui->tableWidget,&QTableWidget::doubleClicked,this,&MainWindow::MusicOpenQuest);
    connect(ui->tableWidget,&QTableWidget::itemDoubleClicked,this,&MainWindow::MusicOpenQuest);
    connect(m_MusicPlayer,SIGNAL(error(QMediaPlayer::Error)),this,SLOT(print(QMediaPlayer::Error)));
    //connect(m_MusicPlayer,&QMediaPlayer::mediaStatusChanged)
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow &MainWindow::getInstance()
{
    static MainWindow instance;
    return instance;
}

void MainWindow::ShowWindow(QString name)
{
    m_UserName = name;
    ui->UserNameLabel->setText(m_UserName);     //设置名字
    show();
}



//加载音乐列表到控件上
void MainWindow::GetMusicList(QStringList list)
{
    m_MusicList = list;
    ShowMusicList();
    initMusicList();
}

//收到播放回复，准备播放
void MainWindow::MusicPlayer()
{
    ui->stopMusicBtn->setToolTip("播放");
    ui->stopMusicBtn->setStyleSheet("QPushButton {"
                                        "width: 36px;"
                                        "height: 36px;"
                                        "border-radius: 18px;"
                                        "background-color: rgba(197, 192, 192, 0.3);"
                                        "margin: 5px;"
                                        "border-image: url(:/openMusic.png);"
                                    "}");
    qDebug() << "重新设置播放媒体";
    m_MusicPlayer->setMedia(QUrl::fromLocalFile("/MyMusic/Music.mp3"));
    //m_MusicPlayer->setMedia(QMediaContent(),&buffer);
}

//双击列表item发送播放请求
void MainWindow::MusicOpenQuest(QTableWidgetItem *item)
{
    int row_index = item->row();        //获取所在列表的行
    m_MusicListW->SetMusicIndex(row_index);
    m_MusicName = m_MusicList.at(row_index);        //获取音乐名字
    //播放前先设置播放媒体为空，好让socket修改文件
//    QMediaContent content;
    m_MusicPlayer->setMedia(QMediaContent());
    qDebug() << "设置播放媒体为空";
    m_tcp->sendmsg(TcpMSGType::MUSICOPEN_REQUEST,m_MusicName);        //发送播放请求
    ui->musicName->setText(m_MusicName);  //显示当前播放的音乐文本
}

void MainWindow::initWindow()
{
    //加载样式
    QFile file(":/Client.css");
    if (file.open(QIODevice::ReadOnly)){
        QString style = this->styleSheet();
        style += QLatin1String(file.readAll());
        this->setStyleSheet(style);
    }else qDebug() << "导入失败";
    m_tcp = Login::getInstance().SendSocket();
    m_tcp->sendmsg(TcpMSGType::MUSICLIST_REQUEST);        //发送请求数据
    ui->volumeBtn->installEventFilter(this);
    QListWidgetItem *item = new QListWidgetItem;
    item->setText("音乐大厅");
    item->setTextAlignment(Qt::AlignCenter);

    //为搜索框右边添加一个小按钮
    searchAction = new QAction(ui->searchLinE);
    searchAction->setIcon(QIcon(":/search.png"));
    searchAction->setCheckable(true);
    ui->searchLinE->addAction(searchAction,QLineEdit::TrailingPosition);
    ui->functionList->addItem(item);
    connect(searchAction,&QAction::triggered,this,[=](bool checked){
        if (checked){
            QString musicname = ui->searchLinE->text();
            int index = m_MusicList.indexOf(musicname);
            if (index == -1){       //没有此音乐
                ui->functionWidget->setCurrentIndex(1);
                return;
            }
            ui->tableWidget->clear();
            QTableWidgetItem *item = new QTableWidgetItem(musicname);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(0,0,item);
        }
        else {
            ui->searchLinE->clear();        //清空文本内容
            ui->functionWidget->setCurrentIndex(0);
            ShowMusicList();
        }
    });

    // 不可编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 取消焦点
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    // 设置行选择
    ui->tableWidget->setSelectionBehavior(QTableWidget::SelectRows);
}

//标题栏初始化
void MainWindow::initTitle()
{
    //安装事件过滤器
    ui->TitleWidget->installEventFilter(this);
}

void MainWindow::initMediaPlay()
{
    m_MusicPlayer = new QMediaPlayer(this);     //实例化一个QMediaPlayer对象
    m_MusicPlayer->setVolume(m_volume);         //设置初始音量
    //当媒体文件变化时发射durationChanged信号：用于获取媒体总时长
    connect(m_MusicPlayer,&QMediaPlayer::durationChanged,this,&MainWindow::onDuratxionChanged);
    //当前媒体文件进度发生变化时发射此信号：用于实时获取播放进度
    connect(m_MusicPlayer,&QMediaPlayer::positionChanged,this,&MainWindow::onPositionChanged);
    //只有媒体加载完成才播放，绑定媒体状态变化信号
    connect(m_MusicPlayer,&QMediaPlayer::mediaStatusChanged,this,&MainWindow::startToPlay);
}

//对音量窗口进行初始化
void MainWindow::initVolume()
{
    m_volumeW = new Volume(this);
    m_volumeW->hide();
    m_volume = 20;
    m_volumeW->installEventFilter(this);        //安装事件过滤器
    //绑定音量变化信号
    connect(m_volumeW,&Volume::SendVolume,this,&MainWindow::getVolume);
    //绑定静音函数
    connect(this,&MainWindow::sendVolume,m_volumeW,&Volume::SetNoVolume);
    //绑定静音恢复函数
    connect(this,&MainWindow::RecoverVolu,m_volumeW,&Volume::SetVolume);
}

void MainWindow::initMusicList()
{
    m_MusicListW = new MusicList(m_MusicList,this);
    m_MusicListW->setFixedSize(ui->tableWidget->width()/2,ui->tableWidget->height());
    m_MusicListW->hide();
    //点击列表的子项，发送请求
    connect(m_MusicListW,&MusicList::childTabDouble,this,&MainWindow::MusicOpenQuest);
    //点击播放模式按钮，进行模式转换变化
    connect(ui->switchBtn,&QPushButton::clicked,m_MusicListW,&MusicList::ListModeChange);
    //模式改变修改样式
    connect(m_MusicListW,&MusicList::PlayStatusChange,this,[=]{
        switch (m_MusicListW->ListStatus) {
        case LISTPLAYER::LOOP:
            ui->switchBtn->setStyleSheet("#switchBtn {"
                                         "font-size: 10px;"
                                         "background-color: transparent;"
                                         "image: url(:/ListCircul.png);"
                                     "}");
            break;
        case LISTPLAYER::SINGLELOOP:
            ui->switchBtn->setStyleSheet("#switchBtn {"
                                         "font-size: 10px;"
                                         "background-color: transparent;"
                                         "image: url(:/SingleCycle.png);"
                                     "}");
            break;
        case LISTPLAYER::RANDOM:
            ui->switchBtn->setStyleSheet("#switchBtn {"
                                         "font-size: 10px;"
                                         "background-color: transparent;"
                                         "image: url(:/RandomPlay.png);"
                                     "}");
            break;
        default:
            break;
        }
    });
    //接收音乐列表窗口发来的音乐名称
    connect(m_MusicListW,&MusicList::MusicName,this,&MainWindow::GetMusicName);
}

void MainWindow::ShowMusicList()
{
    int row = m_MusicList.size();
    ui->tableWidget->setRowCount(row);      //设置列表行数
    int i = 0;
    for (QString MusicName : m_MusicList){
        QTableWidgetItem *item = new QTableWidgetItem(MusicName);
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(MusicName);        //设置提示文本
        ui->tableWidget->setItem(i++,0,item);
    }
}

//设置总时长提示
void MainWindow::onDuratxionChanged(qint64 duration)
{
    ui->MusicTimeSlider->setMaximum(duration);
    int secs = duration/1000;       //获取秒
    int mins = secs/60;
    secs %= 60;
    ui->endMusicLabel->setText(QString("%1:%2").arg(mins).arg(secs));
}

//播放进度改变时改变进度提示
void MainWindow::onPositionChanged(qint64 position)
{
    ui->MusicTimeSlider->setValue(position);
    if (ui->MusicTimeSlider->isSliderDown())    return;
    int secs = position/1000;
    int mins = secs/60;
    secs %= 60;
    ui->startMusicLabel->setText(QString("%1:%2").arg(mins).arg(secs));
}

//对播放器状态的捕获的槽函数
void MainWindow::startToPlay(QMediaPlayer::MediaStatus status)
{
    //只有在加载完毕才播放
    qDebug() << "播放状态变化" << status;
    if (status == QMediaPlayer::LoadedMedia){
        qDebug() << "准备完毕，开始播放";
        m_MusicPlayer->play();
    }
    else if (status == QMediaPlayer::EndOfMedia){
        //对于播放完毕的情况下，直接发送下一首信号即可
        //对于音乐列表播放策略为单曲循环的情况时，直接将进度设置为0即可
        if (m_MusicListW->ListStatus == LISTPLAYER::SINGLELOOP)
            m_MusicPlayer->setPosition(0);
        else
            emit m_MusicListW->NextMusic();
    }
}

//测试用
void MainWindow::print(QMediaPlayer::Error error)
{
    qDebug() << error;
}

//设置音量窗口中得到的音量值
void MainWindow::getVolume(int volume)
{
    m_volume = volume;
    //qDebug() << "修改了音量值" << volume;
    m_MusicPlayer->setVolume(volume);
    if (volume == 0){
        ui->volumeBtn->setStyleSheet("QPushButton {"
                                     "border-style: hidden;"
                                     "border-image: url(:/volumeCross.png);"
                                 "}");
        //ui->volumeBtn->setObjectName("NoVolume");
    }
    else {
        ui->volumeBtn->setStyleSheet("QPushButton {"
                                         "border-style: hidden;"
                                         "border-image: url(:/volumeOpen.png);"
                                     "}");
        //ui->volumeBtn->setObjectName("volumeBtn");
    }
}

//获取上/下一首音乐名称
void MainWindow::GetMusicName(QString MusicName)
{
    m_MusicName = MusicName;
    m_MusicPlayer->setMedia(QMediaContent());
    m_tcp->sendmsg(TcpMSGType::MUSICOPEN_REQUEST,m_MusicName);        //发送播放请求
    ui->musicName->setText(m_MusicName);  //显示当前播放的音乐文本
}

//滑动进度条更新进度
void MainWindow::on_MusicTimeSlider_sliderReleased()
{
    int value = ui->MusicTimeSlider->value();
    m_MusicPlayer->setPosition(value);
}

//事件过滤器
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //对标题栏进行事件判断
    if (watched == ui->TitleWidget){
        //鼠标按下时记录位置
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *e = (QMouseEvent*)event;
            if (e->buttons() == Qt::LeftButton)
                dragPosition = e->globalPos();      //记录鼠标全局位置
        }
        //鼠标移动并且左键按下，移动窗口
        if (event->type() == QEvent::MouseMove){
            QMouseEvent *e = (QMouseEvent*)event;
            if (e->buttons() == Qt::LeftButton){
                QPoint tempPos = e->globalPos() - dragPosition;     //用一个临时位置记录鼠标移动变化
                move(this->pos() + tempPos);        //鼠标移动多少，窗口走动多少
                dragPosition = e->globalPos();
            }
        }
    }

    //对音量按钮的事件进行判断
    if (watched == ui->volumeBtn){
        if (event->type() == QMouseEvent::Enter){
            //获取到控件在全局的位置来移动音量窗口的位置
            m_volumeW->move(ui->volumeBtn->mapToGlobal(QPoint(0,0)).x(), ui->volumeBtn->mapToGlobal(QPoint(0,0)).y() - 125);
            m_volumeW->show();
        }
        if (event->type() == QMouseEvent::Leave){
            //鼠标不在音量窗口上关闭音量窗体
            if (!m_volumeW->geometry().contains(QCursor::pos())){
                m_volumeW->close();
            }
        }
    }
    //对音量窗体的事件进行判断
    if (watched == m_volumeW){
        if (event->type() == QMouseEvent::Leave)
            m_volumeW->close();
    }
    return QWidget::eventFilter(watched,event);
}

//点击音量按钮时发生的事件
void MainWindow::on_volumeBtn_clicked(bool checked)
{
    //qDebug() << "按下音量按钮" << checked << m_volume;
    if (checked){
        if (m_volume == 0)      //本身音量为0，恢复为默认值
            emit sendVolume(20);
        else
            emit sendVolume();     //静音
    }
    else {
        emit RecoverVolu();     //恢复音量
    }
}

//停止按钮
void MainWindow::on_stopMusicBtn_clicked(bool checked)
{
    //没有播放，直接退出
    if (m_MusicPlayer->state() == QMediaPlayer::StoppedState)   return;
    if (checked){
        ui->stopMusicBtn->setToolTip("暂停");
        m_MusicPlayer->pause();
        ui->stopMusicBtn->setStyleSheet("QPushButton {"
                                            "width: 36px;"
                                            "height: 36px;"
                                            "border-radius: 18px;"
                                            "background-color: rgba(197, 192, 192, 0.3);"
                                            "margin: 5px;"
                                            "border-image: url(:/stopMusic.png);"
                                        "}");
    }
    else{
        ui->stopMusicBtn->setToolTip("播放");
        m_MusicPlayer->play();
        ui->stopMusicBtn->setStyleSheet("QPushButton {"
                                            "width: 36px;"
                                            "height: 36px;"
                                            "border-radius: 18px;"
                                            "background-color: rgba(197, 192, 192, 0.3);"
                                            "margin: 5px;"
                                            "border-image: url(:/openMusic.png);"
                                        "}");
    }
}

//上一首
void MainWindow::on_lastMusicBtn_clicked()
{
    emit m_MusicListW->LastMusic();
}

//下一首
void MainWindow::on_nextMusicBtn_clicked()
{
    emit m_MusicListW->NextMusic();
}

//音乐列表按钮点击
void MainWindow::on_MusiclistBtn_clicked(bool checked)
{
    if (checked){
        m_MusicListW->move(ui->tableWidget->mapToGlobal(QPoint(0,0)).x()+ui->tableWidget->width()/2,ui->tableWidget->mapToGlobal(QPoint(0,0)).y());
        m_MusicListW->show();
    }
    else m_MusicListW->hide();
}

//点击最小化按钮
void MainWindow::on_MinWindowBtn_clicked()
{
    showMinimized();
}

//点击关闭按钮
void MainWindow::on_ClearWindowBtn_clicked()
{
    close();
}
