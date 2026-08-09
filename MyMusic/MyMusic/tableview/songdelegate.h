#ifndef SONGDELEGATE_H
#define SONGDELEGATE_H

#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QHash>
#include <QSet>
#include <QList>
#include <QHelpEvent>
#include "tableviewmodel.h"
#include "sortproxymodel.h"

class SongDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    // 功能按钮索引（后续点击逻辑用）
    enum ButtonIndex {
        Download = 0,   // 下载
        Collect = 1,    // 收藏
        Comment = 2,    // 评论
        More = 3        // 更多
    };

    explicit SongDelegate(QObject *parent = nullptr);
    void setHoveredRow(int row);
    void setHoveredButton(int button);   // 设置悬停高亮的按钮索引，-1 表示无

    // 与 paint 共用同一几何计算，供 hover 按钮命中测试复用
    QList<QRect> buttonRects(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    // 返回鼠标位置命中的功能按钮索引（ButtonIndex），未命中返回 -1
    int buttonAt(const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void likeChanged(const int row, const bool status);
    void iconLoaded(const QString &url, int sourceRow);   // 网络图标下载完成，通知视图局部重绘
    // 功能按钮点击（row 为代理行索引，buttonIndex 见 ButtonIndex），预留接口供外部实现下载/收藏等逻辑
    void buttonClicked(const int row, const int buttonIndex);

private:
    QPixmap iconForSong(const SongInfo& song, int sourceRow) const;

    int _hoveredRow = -1;
    int _hoveredButton = -1;
    mutable QHash<QString, QPixmap> _iconCache;   // key=icon_url 或 "pix:"+pixmap.cacheKey()；值=已缩放40x40
    mutable QSet<QString> _loadingUrls;           // 正在下载的url，防止重复请求
};

#endif // SONGDELEGATE_H
