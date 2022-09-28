#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QFile>
#include "login.h"
#include <QStringList>
#include <QMediaPlayer>
#include <QDataStream>
#include "volume.h"
#include "musiclist.h"
#include <QMouseEvent>
#include <QAction>
#include <QBuffer>
#include "set.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow &getInstance();
    void ShowWindow(QString);           //显示窗口

signals:
    void sendVolume(int volume = 0);            //通知音量窗口静音
    void RecoverVolu();             //通知音量窗口恢复音量

private slots:
    void GetMusicList(QStringList);     //加载音乐列表到控件上
    void MusicPlayer();                 //收到播放回复，准备播放
    void MusicOpenQuest(QTableWidgetItem *item);

    void onDuratxionChanged(qint64 duration);        //设置时长
    void onPositionChanged(qint64 position);        //设置当前进度
    void startToPlay(QMediaPlayer::MediaStatus);
    void print(QMediaPlayer::Error error);
    void getVolume(int volume);                     //获取音量窗口滑块值

    void GetMusicName(QString);

    void on_MusicTimeSlider_sliderReleased();

    void on_volumeBtn_clicked(bool checked);

    void on_stopMusicBtn_clicked(bool checked);

    void on_lastMusicBtn_clicked();

    void on_nextMusicBtn_clicked();

    void on_MusiclistBtn_clicked(bool checked);

    void on_MinWindowBtn_clicked();

    void on_ClearWindowBtn_clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event);      //事件过滤器

private:
    QString m_UserName;                 //当前窗口的用户名
    QAction *searchAction;             //搜索按钮

    QString m_MusicName;                //当前播放的音乐名称
    QMediaPlayer *m_MusicPlayer;        //音乐播放设备
    //QBuffer* buff;
    bool SearchBtnflag = true;          //搜索按钮状态
    qint64 m_MusicTimeLen;              //音乐时间长度
    qint64 m_CurrentMusicTime;          //当前音乐时间长度
    Volume *m_volumeW;                   //音量窗口
    QStringList m_MusicList;            //音乐列表
    QStringList m_SearchMusicList;      //搜索音乐列表
    MusicList *m_MusicListW;            //音乐列表窗口
    int m_volume;                       //当前音量

    Set* setWindow;
    //标题栏
    QPoint dragPosition;

    void initWindow();           //初始化主窗口
    void initTitle();            //初始化标题栏
    void initMediaPlay();        //初始化播放器
    void initVolume();           //初始化音量窗口
    void initMusicList();       //初始化音乐列表窗口
    void ShowMusicList();       //显示列表
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
