#include "sortproxymodel.h"
#include <QStringList>

SortProxyModel::SortProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setSortCaseSensitivity(Qt::CaseSensitive);          //区分大小写
    setSortLocaleAware(true);                           //
    sort(-1, Qt::AscendingOrder);
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

QVariant SortProxyModel::data(const QModelIndex& index, int role) const
{
    if (index.column() == 0 && role == Qt::DisplayRole) {
        return index.row() + 1;
    }
    return QSortFilterProxyModel::data(index, role);
}

bool SortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    TableViewModel* model = qobject_cast<TableViewModel*>(sourceModel());
    if (!model) return false;

    if (sortColumn() == -1) {
        qDebug() << "sort";
        const SongInfo& leftSong = model->songAt(source_left.row());
        const SongInfo& rightSong = model->songAt(source_right.row());
        return leftSong.insertOrder < rightSong.insertOrder;
    }

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
