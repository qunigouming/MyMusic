#include "tableviewmodel.h"

TableViewModel::TableViewModel(MusicTableViewType view_type, QObject *parent)
    : QAbstractTableModel{parent}, _type(view_type)
{
    // 点赞图标只构造一次，paint 时直接返回缓存对象
    _likeIcon = QIcon(":/source/icon/like.png");
    _dislikeIcon = QIcon(":/source/icon/disLike.png");
}
