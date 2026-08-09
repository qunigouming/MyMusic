#include "songdelegate.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QToolTip>
#include <QAbstractItemView>
#include <QDir>

namespace {

// 几何常量（对齐原 PlayItemWidget 的布局参数）
const int kIconSize = 40;        // 图标尺寸
const int kIconMarginLeft = 10;  // 图标左边距
const int kFuncWidth = 120;      // 功能按钮区总宽
const int kMarginRight = 10;     // 按钮区右边距
const int kButtonSize = 20;      // 单个按钮尺寸
const int kButtonSpacing = 13;   // 按钮间距
const int kMinTextWidth = 80;    // 文本区最小宽度，小于则隐藏

// 进程内唯一网络管理器 + 磁盘缓存
// （原实现每个 PlayItemWidget 一个管理器指向同一缓存目录，Qt 禁止同目录多实例）
QNetworkAccessManager* sharedNetManager()
{
    static QNetworkAccessManager* manager = []() {
        auto* m = new QNetworkAccessManager;
        auto* cache = new QNetworkDiskCache(m);
        cache->setCacheDirectory(QDir::tempPath() + "/image_cache");
        m->setCache(cache);
        return m;
    }();
    return manager;
}

QPixmap defaultIcon()
{
    static QPixmap icon(":/source/image/default_album.png");
    return icon;
}

QFont iconFont()
{
    QFont font("iconfont");
    font.setPixelSize(14);
    return font;
}

QFont titleFont()
{
    QFont font;
    font.setBold(true);
    font.setPixelSize(14);
    return font;
}

QFont authorFont()
{
    QFont font;
    font.setPixelSize(10);
    return font;
}

// 单元格几何：paint 与 buttonRects 共用同一来源
struct CellGeom {
    QRect iconRect;
    QRect titleRect;
    QRect authorRect;
    QList<QRect> buttons;
};

CellGeom cellGeometry(const QRect &cell, bool showButtons)
{
    CellGeom geom;
    int iconX = cell.left() + kIconMarginLeft;
    int iconY = cell.top() + (cell.height() - kIconSize) / 2;
    geom.iconRect = QRect(iconX, iconY, kIconSize, kIconSize);

    if (showButtons) {
        int funcX = cell.right() - kFuncWidth - kMarginRight;
        int funcY = cell.top() + (cell.height() - kButtonSize) / 2;
        for (int i = 0; i < 4; ++i) {
            int x = funcX + i * (kButtonSize + kButtonSpacing);
            geom.buttons.append(QRect(x, funcY, kButtonSize, kButtonSize));
        }
    }

    // 与原布局一致：无论是否 hover 都预留按钮区（宽 200 = 左边距10+图标40+间距10+按钮区120+右边距10+间距10）
    int textX = geom.iconRect.right() + 10;
    int textWidth = cell.width() - 200;
    if (textWidth >= kMinTextWidth) {
        geom.titleRect = QRect(textX, cell.top() + 20, textWidth, 16);
        geom.authorRect = QRect(textX, cell.top() + 35, textWidth, 12);
    }
    return geom;
}

} // namespace

SongDelegate::SongDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void SongDelegate::setHoveredRow(int row)
{
    _hoveredRow = row;
}

void SongDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 除第一列外的列全部走基类（行号/专辑/点赞/时长）
    if (index.column() != 1) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // 先画背景：基类在 DisplayRole 为空时仍绘制 item 背景，
    // 样式表的 QTableView::item:hover/selected 整行高亮依赖它，不能跳过
    QStyledItemDelegate::paint(painter, option, index);

    // 取源模型中的歌曲信息
    const QSortFilterProxyModel* proxyModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (!proxyModel) return;
    const TableViewModel* model = qobject_cast<const TableViewModel*>(proxyModel->sourceModel());
    if (!model) return;
    QModelIndex srcIndex = proxyModel->mapToSource(index);
    const SongInfo& song = model->songAt(srcIndex.row());

    bool hovered = index.row() == _hoveredRow;
    CellGeom geom = cellGeometry(option.rect, hovered);

    // 图标（内存缓存 + 网络异步加载，恒 ≤40x40 等比缩放后居中）
    QPixmap icon = iconForSong(song, srcIndex.row());
    if (!icon.isNull()) {
        QPoint topLeft = geom.iconRect.center() - QPoint(icon.width() / 2, icon.height() / 2);
        painter->drawPixmap(topLeft, icon);
    }

    // 标题（用 fontMetrics 先省略再绘制，替代 ElidedLabel）
    if (!geom.titleRect.isEmpty()) {
        painter->setFont(titleFont());
        painter->setPen(option.palette.color(QPalette::WindowText));
        QString elided = painter->fontMetrics().elidedText(song.title, Qt::ElideRight, geom.titleRect.width());
        painter->drawText(geom.titleRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
    }

    // 作者
    if (!geom.authorRect.isEmpty()) {
        painter->setFont(authorFont());
        painter->setPen(QColor(0x66, 0x66, 0x66));
        QString elided = painter->fontMetrics().elidedText(song.author, Qt::ElideRight, geom.authorRect.width());
        painter->drawText(geom.authorRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
    }

    // 悬停时显示功能按钮（iconfont 字形；鼠标悬停的按钮高亮，对齐原样式表 QLabel:hover {color:#1e90ff}）
    if (hovered) {
        static const QStringList glyphs = {
            QString(QChar(0xe635)),   // 下载
            QString(QChar(0xe609)),   // 收藏
            QString(QChar(0xe891)),   // 评论
            QString(QChar(0xe61c))    // 更多
        };
        painter->setFont(iconFont());
        for (int i = 0; i < geom.buttons.size(); ++i) {
            painter->setPen(i == _hoveredButton ? QColor(0x1e, 0x90, 0xff) : QColor(0x66, 0x66, 0x66));
            painter->drawText(geom.buttons.at(i), Qt::AlignCenter, glyphs.at(i));
        }
    }
}

void SongDelegate::setHoveredButton(int button)
{
    _hoveredButton = button;
}

QList<QRect> SongDelegate::buttonRects(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return cellGeometry(option.rect, index.row() == _hoveredRow).buttons;
}

int SongDelegate::buttonAt(const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QRect> rects = buttonRects(option, index);
    for (int i = 0; i < rects.size(); ++i) {
        if (rects.at(i).contains(pos)) {
            return i;
        }
    }
    return -1;
}

QPixmap SongDelegate::iconForSong(const SongInfo& song, int sourceRow) const
{
    // 构造缓存 key：有本地 QPixmap 用 pixmap 的 cacheKey，否则用 icon_url
    QString key;
    if (!song.icon.isNull()) {
        key = "pix:" + QString::number(song.icon.cacheKey());
    } else {
        key = song.icon_url.isEmpty() ? "default" : song.icon_url;
    }

    auto it = _iconCache.constFind(key);
    if (it != _iconCache.constEnd()) {
        return it.value();
    }

    // paint 是 const 成员而信号是非 const 方法，lambda 内通过去 const 的 self 发射信号
    SongDelegate* self = const_cast<SongDelegate*>(this);

    QPixmap source;
    if (!song.icon.isNull()) {
        source = song.icon;
    } else if (song.icon_url.startsWith("http", Qt::CaseInsensitive)) {
        // 网络图片：单例管理器异步下载，成功后发信号触发视图局部重绘
        if (!_loadingUrls.contains(song.icon_url)) {
            _loadingUrls.insert(song.icon_url);
            QNetworkRequest request(song.icon_url);
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
            QNetworkReply* reply = sharedNetManager()->get(request);
            connect(reply, &QNetworkReply::finished, self, [self, reply, url = song.icon_url, sourceRow]() {
                reply->deleteLater();
                self->_loadingUrls.remove(url);
                if (reply->error() == QNetworkReply::NoError) {
                    QPixmap pixmap;
                    if (pixmap.loadFromData(reply->readAll())) {
                        self->_iconCache.insert(url, pixmap.scaled(kIconSize, kIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        emit self->iconLoaded(url, sourceRow);
                    }
                }
            });
        }
        // 下载期间先用默认封面占位
        source = defaultIcon();
    } else {
        // 资源（:/）或本地路径：同步加载；失败缓存默认图，避免反复尝试失败路径
        source = QPixmap(song.icon_url);
        if (source.isNull()) {
            source = defaultIcon();
        }
    }

    QPixmap scaled = source.scaled(kIconSize, kIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _iconCache.insert(key, scaled);
    return scaled;
}

void SongDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    // 应用整行悬停效果
    if (index.row() == _hoveredRow) {
        option->state |= QStyle::State_MouseOver;
    }
    if (index.column() == 0) option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}

bool SongDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // 功能按钮 tooltip（对齐原 PlayItemWidget 的 QLabel tooltip：下载/收藏/评论/更多）
    if (index.column() == 1 && event->type() == QEvent::ToolTip) {
        static const QStringList tips = { tr("下载"), tr("收藏"), tr("评论"), tr("更多") };
        int button = buttonAt(event->pos(), option, index);
        if (button >= 0 && button < tips.size()) {
            QToolTip::showText(event->globalPos(), tips.at(button), view);
            return true;
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

bool SongDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    // 第三列点击切换喜欢状态
    if (event->type() == QEvent::MouseButtonPress && index.column() == 3) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            bool current = model->data(index, Qt::EditRole).toBool();
            emit likeChanged(index.row(), !current);
            model->setData(index, !current, Qt::EditRole);
            return true;
        }
    }

    // 第一列功能按钮点击（仅左键松开且命中按钮时触发，索引见 ButtonIndex）
    if (event->type() == QEvent::MouseButtonRelease && index.column() == 1) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            int button = buttonAt(me->pos(), option, index);
            if (button >= 0) {
                emit buttonClicked(index.row(), button);
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
