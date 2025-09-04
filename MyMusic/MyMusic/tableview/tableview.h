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
    int rowCount();
    void setBatchSize(int size);

signals:
    void allSongsAdded();       // 添加完成信号
    void rowDoubleClicked(const SongInfo& path);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void processPendingSongs();     // 处理待添加歌曲
    void updateVisibleRange();      // 更新可见行范围
    void updatePersistentEditors();

private:
    void setVisibleProxyRange(int start, int end);

private:
    TableHeaderView* _header = nullptr;
    SortProxyModel* _proxyModel = nullptr;
    SongDelegate* _delegate = nullptr;
    TableViewModel* _model = nullptr;

    int _hoveredRow = -1;
    MusicTableViewType _type = MusicTableViewType::NET_MODEL;

    // 分批处理相关变量
    QTimer* _batchTimer = nullptr;
    QList<SongInfo> _pendingSongs;
    int _batchSize = 50;            // 每次处理歌曲数量

    // 可视区域管理
    QSet<int> _visibleRows;
    QSet<int> _editorRows;

    int _visibleStart = -1;
    int _visibleEnd = -1;

    QTimer* _scrollUpdateTimer = nullptr;
};
#endif // TABLEVIEW_H
