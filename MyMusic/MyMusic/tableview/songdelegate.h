#ifndef SONGDELEGATE_H
#define SONGDELEGATE_H

#include <QEvent>
#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include "playitemwidget.h"
#include "tableviewmodel.h"
#include "sortproxymodel.h"

class SongDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SongDelegate(QObject *parent = nullptr);
    void setHoveredRow(int row);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool eventFilter(QObject *object, QEvent *event) override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    int _hoveredRow = -1;
};

#endif // SONGDELEGATE_H
