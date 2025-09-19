#ifndef TABLEVIEWMODEL_H
#define TABLEVIEWMODEL_H

#include <QAbstractTableModel>
#include <QPixmap>
#include <QDebug>
#include <QIcon>
#include "dataInfo.h"
#include <QTime>

enum class MusicTableViewType {
    NET_MODEL = 1,
    LOCAL_MODEL = 2
};

struct SongInfo {
    QString icon_url;
    QString title;
    QString album;
    bool isLiked;
    QString duration;
    QString file_size;
    bool isVIP;
    QString author;
    QString path;
    QPixmap icon;
    int insertOrder;

    SongInfo(QString str_title, QString str_duration, QString str_file_size,
        QString str_path, QPixmap icon = QPixmap(), QString str_author = "", QString str_album = "未知专辑",
        QString str_icon_url = "", bool b_isLiked = false, bool b_isVIP = false, int insertOrder = -1)
        : icon_url(str_icon_url), icon(icon), title(str_title), album(str_album), isLiked(b_isLiked), duration(str_duration),
        file_size(str_file_size), isVIP(b_isVIP), author(str_author), path(str_path), insertOrder(insertOrder) {
        // 先获取歌曲的封面，如果没有封面则使用url，若还没有封面则使用默认封面
        if (icon.isNull()) {
            if (icon_url.isEmpty()) {
                icon_url = ":/source/image/default_album.png";
            }
        }
    }

    SongInfo(MusicInfo& music_info) {
        icon_url = QString::fromStdString(music_info.song_icon);
        title = QString::fromStdString(music_info.title);
        album = QString::fromStdString(music_info.album);
        isLiked = music_info.is_like;

        QTime time(0, 0, music_info.duration);
        duration = time.toString("mm:ss");

        isVIP = false;
        author = QString::fromStdString(music_info.artists);
        path = QString::fromStdString(music_info.file_url);
    }

    SongInfo(MusicInfo* music_info) {
        icon_url = QString::fromStdString(music_info->song_icon);
        title = QString::fromStdString(music_info->title);
        album = QString::fromStdString(music_info->album);
        isLiked = music_info->is_like;

        QTime time(0, 0, music_info->duration);
        duration = time.toString("mm:ss");

        isVIP = false;
        author = QString::fromStdString(music_info->artists);
        path = QString::fromStdString(music_info->file_url);
    }
};

class TableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum CustomRoles {
        SongIconRole = Qt::UserRole + 1,
        SongNameRole,
        AuthorNameRole
    };
    explicit TableViewModel(MusicTableViewType view_type, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return _songs.size();
    }

    int columnCount(const QModelIndex &parent) const override {
        return 5;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return "#";
            case 1: return "标题";
            case 2: return "专辑";
            case 3: {
                if (_type == MusicTableViewType::NET_MODEL) return "喜欢"; else return "大小";
            }
            case 4: return "时长";
            }
        }
        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid())   return QVariant();
        const SongInfo& song = _songs.at(index.row());
        int column = index.column();
        //qDebug() << "song index: " << song.index << "\tcolumn: " << column << "role: " << role;
        switch(role) {
            case Qt::DisplayRole:
                switch (column) {
                case 0: return QVariant();     // 代理模型会处理，这里返回QVariant()
                case 2: return song.album;
                case 3: if (_type == MusicTableViewType::LOCAL_MODEL)   return song.file_size; else break;
                case 4: return song.duration;
                default:
                    break;
                }
                break;
            case Qt::DecorationRole:
                if (column == 3) {
                    if (_type == MusicTableViewType::NET_MODEL)
                        return song.isLiked ? QIcon(":/source/icon/like.png") : QIcon(":/source/icon/disLike.png");
                }
            case Qt::EditRole:
                if (column == 3 && (_type == MusicTableViewType::NET_MODEL)) return song.isLiked;
            //未知原因，使用(Qt::AlignLeft | Qt::AlignVCenter)有问题，但Qt::AlignRight没有问题
            // case Qt::TextAlignmentRole: {
            //     if (column == 0) return Qt::AlignCenter;
            // }
        }
        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (!index.isValid() || role != Qt::EditRole)   return false;
        int row = index.row();
        _songs[row].isLiked = value.toBool();
        emit dataChanged(index, index, {role});
        return true;
    }

    void addSong(SongInfo& song) {
        beginInsertRows(QModelIndex(), _songs.size(), _songs.size());
        song.insertOrder = ++_insertCounter;
        _songs.append(song);
        endInsertRows();
    }

    void addSong(QList<SongInfo>& songs) {
        beginInsertRows(QModelIndex(), _songs.size(), _songs.size() + songs.size() - 1);
        for (auto& song : songs) {
            song.insertOrder = ++_insertCounter;
            _songs.append(song);
        }
        endInsertRows();
    }

    void clearAllSongs() {
        if (_songs.isEmpty())   return;
        beginResetModel();
        _songs.clear();
        _insertCounter = 0;
        endResetModel();
    }

    const SongInfo& songAt(const int row) const {
        return _songs.at(row);
    }

private:
    QVector<SongInfo> _songs;

    MusicTableViewType _type = MusicTableViewType::NET_MODEL;
    int _insertCounter = 0;     // 插入计数器
};

#endif // TABLEVIEWMODEL_H
