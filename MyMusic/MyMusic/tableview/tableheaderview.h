#ifndef TABLEHEADERVIEW_H
#define TABLEHEADERVIEW_H

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QTableView>
#include <QPainter>

class TableHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit TableHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    void initSize(const QTableView* view);

protected:
    void mousePressEvent(QMouseEvent *e) override {
        int section = logicalIndexAt(e->pos());
        if (section == 0 || section == 3) {
            e->ignore();
            return;
        }
        //可控制列
        e->accept();
        QHeaderView::mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        int section = logicalIndexAt(e->pos());
        if (1 == section || 2 == section || 4 == section) {
            // 设置排序指示器
            Qt::SortOrder order = Qt::AscendingOrder;
            if (sortIndicatorSection() == section) {
                order = (sortIndicatorOrder() == Qt::AscendingOrder)
                ? Qt::DescendingOrder : Qt::AscendingOrder;
            }
            setSortIndicator(section, order);

            // 发出排序信号
            emit sortIndicatorChanged(section, order);
            e->accept();
        } else QHeaderView::mouseReleaseEvent(e);
    }

    void updateGeometries() override {
        QHeaderView::updateGeometries();
        adjustTotalWidth();
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        int section = logicalIndexAt(e->pos());
        if (section != _hoveredSection) {
            _hoveredSection = section;
            viewport()->update();
        }
        QHeaderView::mouseMoveEvent(e);
    }

    void leaveEvent(QEvent *event) override {
        _hoveredSection = -1;
        viewport()->update();
        QHeaderView::leaveEvent(event);
    }

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override {
        // 绘制背景
        QStyleOptionHeader opt;
        initStyleOption(&opt);
        opt.rect = rect;
        opt.section = logicalIndex;
        if (logicalIndex == 0) opt.textAlignment = Qt::AlignCenter;
        else opt.textAlignment = (Qt::AlignLeft | Qt::AlignVCenter);
        opt.text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();

        // 绘制悬停效果
        if ((logicalIndex == 1 || logicalIndex == 2 || logicalIndex == 4) && logicalIndex == _hoveredSection) {
            opt.state |= QStyle::State_MouseOver;
            // 绘制排序指示器
            if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
                opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                ? QStyleOptionHeader::SortDown
                : QStyleOptionHeader::SortUp;
            }
        }

        // 绘制表头部分
        style()->drawControl(QStyle::CE_Header, &opt, painter, this);
    }

private slots:
    void onSectionResized(int logicalIndex, int oldSize, int newSize);

private:
    void distrubution(const int total, int ratio1, int ratio2, int& result1, int& result2);
    void adjustTotalWidth() {
        int total = calculateTotalWidth();

        int viewportWidth = viewport()->width();
        //qDebug() << "adjust width: " << total << "\tview width: " << viewportWidth;
        if (total != viewportWidth) {
            int delta = viewportWidth - total;
            int section4Size = sectionSize(4);
            int newSection4Size = qMax(minimumSectionSize(), section4Size + delta);

            blockSignals(true);
            resizeSection(4, newSection4Size);
            blockSignals(false);
        }
    }
    int calculateTotalWidth() const {
        int total = 0;
        for (int i = 0; i < count(); ++i)
            total += sectionSize(i);
        return total;
    }

    bool _initStatus = false;
    int _hoveredSection = -1;
};

#endif // TABLEHEADERVIEW_H
