#include "songdelegate.h"
#include <QApplication>
#include "tableview.h"

SongDelegate::SongDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void SongDelegate::setHoveredRow(int row)
{
    _hoveredRow = row;
}

void SongDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
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

bool SongDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        PlayItemWidget* widget = qobject_cast<PlayItemWidget*>(object);
        if (widget) {
            TableView* tableView = qobject_cast<TableView*>(parent());
            if (tableView) {
                // 转发鼠标事件到表格视图
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QMouseEvent* mouseEvent = new QMouseEvent(
                    QEvent::MouseMove,
                    widget->mapTo(tableView, widget->rect().center()),
                    Qt::NoButton,
                    Qt::NoButton,
                    Qt::NoModifier
                    );
#else
                QPoint localPos = widget->mapTo(tableView, widget->rect().center());
                QPoint globalPos = widget->mapToGlobal(localPos);
                QMouseEvent* mouseEvent = new QMouseEvent(
                    QEvent::MouseMove,
                    localPos,
                    globalPos,
                    Qt::NoButton,
                    Qt::NoButton,
                    Qt::NoModifier
                    );
#endif
                QCoreApplication::postEvent(tableView, mouseEvent);
            }
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

QWidget* SongDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (index.column() == 1) {
        PlayItemWidget* widget = new PlayItemWidget(parent);
        // connect(widget);
        return widget;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void SongDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (index.column() == 1) {
        PlayItemWidget* widget = static_cast<PlayItemWidget*>(editor);
        if (!widget) {
            qDebug() << "widget is nullptr";
            return;
        }

        //get proxy model
        const QSortFilterProxyModel* proxyModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
        if (!proxyModel) {
            qDebug() << "not a proxy model";
            return;
        }

        const TableViewModel* model = qobject_cast<const TableViewModel*>(proxyModel->sourceModel());
        if (!model) {
            qDebug() << "model is nullptr!!!";
            return;
        }

        QModelIndex srcIndex = proxyModel->mapToSource(index);
        const SongInfo& song = model->songAt(srcIndex.row());
        // 没图片用路径，有图片用图片
        if (song.icon.isNull()) {
            //qDebug() << "set icon from url" << song.icon_url;
            widget->setSongIcon(song.icon_url);
        }
        else {
            //qDebug() << "set icon from icon";
            widget->setSongIcon(song.icon);
        }
        widget->setSongName(song.title);
        widget->setAuthorName(song.author);
    } else QStyledItemDelegate::setEditorData(editor, index);
}

bool SongDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {
    //设置第三列点击修改bool值
    if (event->type() == QEvent::MouseButtonPress && index.column() == 3) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            bool current = model->data(index, Qt::EditRole).toBool();
            emit likeChanged(index.row(), !current);
            model->setData(index, !current, Qt::EditRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
