#ifndef MUSICLIST_H
#define MUSICLIST_H

#include <QDialog>
#include "protocol.h"
#include <QTableWidgetItem>

namespace Ui {
class MusicList;
}

class MusicList : public QDialog
{
    Q_OBJECT

public:
    explicit MusicList(QStringList list,QWidget *parent = 0);
    ~MusicList();
    void setMusicList(QStringList);     //公开接口，方便重新设置音乐列表
    void SetMusicIndex(int index);      //设置音乐下标

    LISTPLAYER ListStatus;              //列表播放模式

signals:
    void LastMusic();                   //上一首
    void NextMusic();                   //下一首
    void MusicName(QString);            //针对播放策略发送音乐名称
    void PlayStatusChange();            //播放状态改变时发送该信号
    void childTabDouble(QTableWidgetItem*);

public slots:
    void ListModeChange();            //改变列表播放模式
    void AnewCreateList();              //列表播放模式被改变了就重新生成列表次序
    void LastMusicList();
    void NextMusicList();

private:
    void LoopListCreate();              //列表循环模式的列表创建
    void SingleLoopCreate();            //单曲循环模式的列表创建
    void RandomListCreate();            //随机播放列表的创建
    void ShowMusicList();               //显示列表数据

    QStringList m_MusicList;            //音乐列表,存储原始数据
    QStringList m_ChangedList;          //变化列表,存储直接放入空间数据的列表
    int MusicIndex;                     //当前播放音乐的索引(不论是什么列表)
    Ui::MusicList *ui;
};

#endif // MUSICLIST_H
