#include "tableheaderview.h"
#include "LogManager.h"

TableHeaderView::TableHeaderView(Qt::Orientation orientation, QWidget *parent) : QHeaderView(orientation, parent) {
    setSectionsClickable(true);
    setHighlightSections(true);
    connect(this, &TableHeaderView::sectionResized, this, &TableHeaderView::onSectionResized);

    setSortIndicator(-1, Qt::AscendingOrder);
    setMouseTracking(true);

    setStyleSheet(R"(
    QHeaderView::section {
        background-color: #91ceea;
        padding: 4px;
        border: none;
        border-bottom: 1px solid #6fc0e0;
    }
    QHeaderView::section:hover {
        background-color: #7ab8d9;
        border-radius: 3px;
    }
    )");
}

void TableHeaderView::onSectionResized(int logicalIndex, int oldSize, int newSize)
{
    if (!_initStatus)   return;             //防止初始化设置长度时设置失效
        // 1. Define Constants locally for clarity
    const int MIN_W_COL = 200; // Title/Album min width
    const int MIN_W_DUR = 80;  // Duration min width

    // ==========================================================
    // LOGIC FOR COLUMN 1 (TITLE) -> Affects Col 2 (Album) & 4 (Duration)
    // ==========================================================
    if (logicalIndex == 1) {
        int finalNewSize = newSize;
        bool isClamped = false;

        // Check if the column ITSELF is too small
        if (newSize < MIN_W_COL) {
            finalNewSize = MIN_W_COL;
            isClamped = true;
        }

        int delta = finalNewSize - oldSize; // Recalculate delta based on clamped size
        int section2Size = sectionSize(2);

        // --- Drag Right (Title expands, Album shrinks) ---
        if (delta > 0) {
            int newSection2 = section2Size - delta;

            if (newSection2 < MIN_W_COL) {
                // Album hit minimum
                // 1. Set Album to min
                // 2. Pass the overflow to Duration
                int overflow = MIN_W_COL - newSection2;
                int section4Size = sectionSize(4);
                int newSection4 = section4Size - overflow;

                blockSignals(true);
                resizeSection(2, MIN_W_COL); // Force Album min

                if (newSection4 >= MIN_W_DUR) {
                    resizeSection(1, finalNewSize); // Title grows fully
                    resizeSection(4, newSection4);  // Duration shrinks
                }
                else {
                    // Duration ALSO hit min. Stop everything.
                    // Recalculate max possible Title width
                    resizeSection(4, MIN_W_DUR);
                    int maxPossibleTitle = oldSize + (section2Size - MIN_W_COL) + (section4Size - MIN_W_DUR);
                    resizeSection(1, maxPossibleTitle);
                }
                blockSignals(false);
                return;
            }
        }
        // --- Drag Left (Title shrinks, Album expands) ---
        else {
            int newSection2 = section2Size - delta;

            // Just apply. Neighbor growing has no min-width issues.
            blockSignals(true);
            if (isClamped) resizeSection(1, finalNewSize); // Force Title to 100 if it tried to go to 90
            resizeSection(2, newSection2);
            blockSignals(false);
            return;
        }

        // Apply Standard Change if no special edge cases met
        blockSignals(true);
        if (isClamped) resizeSection(1, finalNewSize);
        resizeSection(2, section2Size - delta);
        blockSignals(false);
    }
    // ==========================================================
    // LOGIC FOR COLUMN 2 (ALBUM) -> Affects Col 4 (Duration)
    // ==========================================================
    else if (logicalIndex == 2) {
        int finalNewSize = newSize;
        bool isClamped = false;

        // Check if Album ITSELF is too small
        if (newSize < MIN_W_COL) {
            finalNewSize = MIN_W_COL;
            isClamped = true;
        }

        int delta = finalNewSize - oldSize;
        int section4Size = sectionSize(4);
        int newSection4 = section4Size - delta;

        // Check if pushing into Duration violates Duration's min width
        if (delta > 0 && newSection4 < MIN_W_DUR) {
            blockSignals(true);
            resizeSection(2, oldSize); // Reject resize
            blockSignals(false);
        }
        else {
            blockSignals(true);
            if (isClamped) resizeSection(2, finalNewSize); // Apply Clamp
            resizeSection(4, newSection4); // Adjust neighbor
            blockSignals(false);
        }
    }
}

void TableHeaderView::distrubution(const int total, int ratio1, int ratio2, int &result1, int &result2)
{
    if (ratio1 == 0 || ratio2 == 0 || total <= 0) return;

    // 计算比例总和
    int sumRatios = ratio1 + ratio2;

    // 计算基本分配值和余数
    int baseValue = total / sumRatios;
    int remainder = total % sumRatios;

    result1 = baseValue * ratio1;
    result2 = baseValue * ratio2;

    //将剩下的值加到最大值上
    int& maxresult = (result1 > result2) ? result1 : result2;
    maxresult += remainder;
}

void TableHeaderView::adjustTotalWidth() {
    int total = calculateTotalWidth();

    int viewportWidth = viewport()->width();
    // LOG(INFO) << "adjust width: " << total << "\tview width: " << viewportWidth;
    if (total != viewportWidth) {
        int delta = viewportWidth - total;
        int section4Size = sectionSize(4);
        int newSection4Size = qMax(minimumSectionSize(), section4Size + delta);

        blockSignals(true);
        resizeSection(4, newSection4Size);
        blockSignals(false);
    }
}

int TableHeaderView::calculateTotalWidth() const {
    int total = 0;
    for (int i = 0; i < count(); ++i)
        total += sectionSize(i);
    return total;
}

void TableHeaderView::initSize(int viewportWidth)
{
    if (_initStatus) return;
    //初始化尺寸，0 3 4固定大小，1 2列按剩余尺寸分配
    setSectionResizeMode(0, QHeaderView::Fixed); // 1 column
    setSectionResizeMode(3, QHeaderView::Fixed); // 3 column
    resizeSection(0, 50);
    resizeSection(3, 50);
    resizeSection(4, 80);
    int fixedWidths = sectionSize(0) + sectionSize(3) + sectionSize(4);
    int remainwidth = viewportWidth - fixedWidths;
    int res1 = 0, res2 = 0;
    distrubution(remainwidth, 3, 2, res1, res2);
    resizeSection(1, res1);
    setSectionResizeMode(2, QHeaderView::Interactive);
    resizeSection(2, res2);
    qDebug() << remainwidth << res1 << res2;
    _initStatus = true;
}

void TableHeaderView::mousePressEvent(QMouseEvent* e) {
    int section = logicalIndexAt(e->pos());
    if (section == 0 || section == 3) {
        e->ignore();
        return;
    }
    QHeaderView::mousePressEvent(e);
}

void TableHeaderView::mouseReleaseEvent(QMouseEvent* e) {
    int section = logicalIndexAt(e->pos());
    if (section == 0 || section == 3) {
        e->ignore();
        return;
    }
    e->accept();
    int currentSortSection = sortIndicatorSection();
    Qt::SortOrder currentOrder = sortIndicatorOrder();

    Qt::SortOrder newOrder = Qt::AscendingOrder;
    int newSection = section;

    if (currentSortSection == section) {
        if (currentOrder == Qt::AscendingOrder)
            newOrder = Qt::DescendingOrder;
        else {
            // 当前为降序，取消排序
            newSection = -1;
            newOrder = Qt::AscendingOrder;
        }
    }
    else {
        // 当前为不同列，默认升序
        newOrder = Qt::AscendingOrder;
    }
    setSortIndicator(newSection, newOrder);
    emit sortIndicatorChanged(newSection, newOrder);
}

void TableHeaderView::updateGeometries() {
    QHeaderView::updateGeometries();
    adjustTotalWidth();
}

void TableHeaderView::mouseMoveEvent(QMouseEvent* e) {
    int section = logicalIndexAt(e->pos());
    if (section != _hoveredSection) {
        _hoveredSection = section;
        viewport()->update();
    }
    QHeaderView::mouseMoveEvent(e);
}

void TableHeaderView::leaveEvent(QEvent* event) {
    _hoveredSection = -1;
    viewport()->update();
    QHeaderView::leaveEvent(event);
}

void TableHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
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
