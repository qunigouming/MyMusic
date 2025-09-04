#ifndef SORTPROXYMODEL_H
#define SORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "tableviewmodel.h"

class SortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SortProxyModel(QObject *parent = nullptr);
    //void setVisibleRange(int start, int end);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

//private:
//    int _visibleStart = -1;
//    int _visibleEnd = -1;
};

#endif // SORTPROXYMODEL_H
