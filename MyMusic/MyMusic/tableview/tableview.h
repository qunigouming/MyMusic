#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include "songdelegate.h"
#include "tableviewmodel.h"
#include "tableheaderview.h"
#include "sortproxymodel.h"
#include <QTimer>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(MusicTableViewType view_type = MusicTableViewType::NET_MODEL, QWidget* parent = nullptr);
    ~TableView();

    void addSong(const SongInfo& song);
    void addSong(const QList<SongInfo>& songs);
    void clearAllSongs();
    int rowCount();
    void setBatchSize(int size);            // 设置分批处理歌曲数量

    SongInfo getSongInfoByProxyRow(int proxyRow) const;

signals:
    void allSongsAdded();       // 添加完成信号
    void rowDoubleClicked(const SongInfo& path);
    void likeChanged(const int id, const bool status);
    // 功能按钮点击（row 为代理行索引，buttonIndex 见 SongDelegate::ButtonIndex），
    // 预留接口：后续在外部连接此信号实现下载/收藏/评论/更多等逻辑
    void rowButtonClicked(const int row, const int buttonIndex);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void processPendingSongs();     // 处理待添加歌曲

private:
    QRect rowRect(int row) const;   // 整行 viewport 矩形，悬停重绘用
    void updateHoveredButton(const QPoint &pos, int row);   // 跟踪功能按钮悬停状态，命中变化时局部重绘

private:
    TableHeaderView* _header = nullptr;
    SortProxyModel* _proxyModel = nullptr;
    SongDelegate* _delegate = nullptr;
    TableViewModel* _model = nullptr;

    int _hoveredRow = -1;
    int _hoveredButton = -1;
    MusicTableViewType _type = MusicTableViewType::NET_MODEL;

    // 分批处理相关变量
    QTimer* _batchTimer = nullptr;
    QList<SongInfo> _pendingSongs;
    int _batchSize = 50;            // 每次处理歌曲数量
};
#endif // TABLEVIEW_H
