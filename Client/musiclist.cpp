#include "musiclist.h"
#include "ui_musiclist.h"
#include <QDebug>

MusicList::MusicList(QStringList list,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicList),
    m_MusicList(list)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);     //不显示标题栏
    ListStatus = LISTPLAYER::LOOP;          //默认循环播放
    AnewCreateList();       //生成变化列表
    MusicIndex = 0;
    ui->tableWidget->verticalHeader()->hide();
    hide();             //默认隐藏
    ShowMusicList();
    // 不可编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 取消焦点
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    // 设置行选择
    ui->tableWidget->setSelectionBehavior(QTableWidget::SelectRows);
    //列表被双击，发送给主窗口来处理
    connect(ui->tableWidget,&QTableWidget::itemDoubleClicked,this,[=](QTableWidgetItem* item){
        emit childTabDouble(item);
    });
    //列表播放模式被改变就重新生成列表
    connect(this,&MusicList::PlayStatusChange,this,&MusicList::AnewCreateList);
    //主窗口点击上一首，发送上一首的名字
    connect(this,&MusicList::LastMusic,this,&MusicList::LastMusicList);
    //主窗口点击下一首，发送下一首的名字
    connect(this,&MusicList::NextMusic,this,&MusicList::NextMusicList);
}

MusicList::~MusicList()
{
    delete ui;
}

//重新设置列表
void MusicList::setMusicList(QStringList list)
{
    m_MusicList = list;
    ShowMusicList();        //显示列表
}

//设置音乐下标
void MusicList::SetMusicIndex(int index)
{
    QString name = m_MusicList.at(index);
    MusicIndex = m_ChangedList.indexOf(name);
}

//播放列表状态变化，顺序循环 -> 单曲循环 -> 随机播放
void MusicList::ListModeChange()
{
    if (ListStatus == LISTPLAYER::LOOP)
        ListStatus = LISTPLAYER::SINGLELOOP;
    else if (ListStatus == LISTPLAYER::SINGLELOOP)
        ListStatus = LISTPLAYER::RANDOM;
    else    ListStatus = LISTPLAYER::LOOP;
    emit PlayStatusChange();
}

//针对列表播放模式重新创建列表并展示列表
void MusicList::AnewCreateList()
{
    switch(ListStatus){
        case LISTPLAYER::LOOP: {
            LoopListCreate();
            break;
        }
        case LISTPLAYER::SINGLELOOP: {
            SingleLoopCreate();
            break;
        }
        case LISTPLAYER::RANDOM: {
            RandomListCreate();
            break;
        }
        default: break;
    }
}

//发送给主窗口上一首歌名
void MusicList::LastMusicList()
{
    MusicIndex = MusicIndex - 1 < 0 ? m_ChangedList.size() : MusicIndex - 1;
    emit MusicName(m_ChangedList.at(MusicIndex));
}

void MusicList::NextMusicList()
{
    MusicIndex = MusicIndex + 1 >= m_ChangedList.size() ? 0 : MusicIndex + 1;
    emit MusicName(m_ChangedList.at(MusicIndex));
}

/*这些列表非展示品，只为内部播放策略*/
//列表循环模式的列表创建
void MusicList::LoopListCreate()
{
    m_ChangedList = m_MusicList;
}

//单曲循环模式的列表创建
void MusicList::SingleLoopCreate()
{
    m_ChangedList = m_MusicList;
}

//随机播放列表的创建
void MusicList::RandomListCreate()
{
    std::random_shuffle(m_ChangedList.begin(),m_ChangedList.end());     //进行随机排序
}

//将固定列表显示到控件上
void MusicList::ShowMusicList()
{
    int row = m_MusicList.size();
    ui->tableWidget->setRowCount(row);
    int i = 0;
    for (QString MusicName : m_MusicList){
        QTableWidgetItem *item = new QTableWidgetItem(MusicName);
        item->setTextAlignment(Qt::AlignCenter);        //居中
        ui->tableWidget->setItem(i++,0,item);
    }
}
