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
    void initSize(int viewportWidth);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    // When the widget initialize, the sections and the viewport do not match,needs the function to adjustment.
    void updateGeometries() override;

    void mouseMoveEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

private slots:
    void onSectionResized(int logicalIndex, int oldSize, int newSize);

private:
    void distrubution(const int total, int ratio1, int ratio2, int& result1, int& result2);
    void adjustTotalWidth();
    int calculateTotalWidth() const;

    bool _initStatus = false;
    int _hoveredSection = -1;
};

#endif // TABLEHEADERVIEW_H
