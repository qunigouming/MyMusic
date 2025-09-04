#include "sortproxymodel.h"
#include <QStringList>

SortProxyModel::SortProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setSortCaseSensitivity(Qt::CaseSensitive);          //区分大小写
    setSortLocaleAware(true);                           //
}

//void SortProxyModel::setVisibleRange(int start, int end)
//{
//    _visibleStart = start;
//    _visibleEnd = end;
//    qDebug() << "Setting visible range:" << start << "to" << end;
//    invalidateFilter();     // 触发过滤更新
//}

int SortProxyModel::rowCount(const QModelIndex& parent) const
{
    if (sourceModel())  return sourceModel()->rowCount(parent);
    return 0;
}

bool SortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    TableViewModel* model = qobject_cast<TableViewModel*>(sourceModel());
    if (!model) return false;
    int col = source_left.column();
    const SongInfo& leftSong = model->songAt(source_left.row());
    const SongInfo& rightSong = model->songAt(source_right.row());
    switch (col) {
    case 1:
        return leftSong.title < rightSong.title;
    case 2:
        return leftSong.album < rightSong.album;
    case 4:
    {
        auto toSeconds = [](const QString& duration) {
            QStringList parts = duration.split(':');
            if (parts.size() != 2)  return 0;
            return parts.at(0).toInt() * 60 + parts.at(1).toInt();
        };
        return toSeconds(leftSong.duration) < toSeconds(rightSong.duration);
    }
    default:
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}

bool SortProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    //// 若不指定显示范围，则显示所有行
    //if (_visibleStart == -1 || _visibleEnd == -1)   return true;
    //// 若用源模型行号处理，行号顺序会被改变，但是用代理模型顺序不变
    //// 将源模型行号转换为代理模型行号
    //QModelIndex proxyIndex = mapFromSource(sourceModel()->index(source_row, 0, source_parent));
    //if (!proxyIndex.isValid()) return false;

    //// 检查是否在可见范围内
    //int proxyRow = proxyIndex.row();
    //bool accept = proxyRow >= _visibleStart && proxyRow <= _visibleEnd;
    //qDebug() << "filterAcceptsRow:" << source_row << "accept:" << accept;
    return true;
}
