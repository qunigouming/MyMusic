#include "tableheaderview.h"

#define MIN_WIDTH 200

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
    // 第一列被调整：改变第二列宽度
    if (logicalIndex == 1) {
        if (newSize < MIN_WIDTH) {
            blockSignals(true);
            resizeSection(1, MIN_WIDTH);
            blockSignals(false);
            return;
        }
        int delta = newSize - oldSize;
        int section1Size = sectionSize(2);

        int newSection1Size = section1Size - delta;
        if (newSection1Size < MIN_WIDTH) {
            //qDebug() << "block 1 section";
            blockSignals(true);
            resizeSection(1, oldSize);
            blockSignals(false);
            return;
        }

        // 防止递归调用
        blockSignals(true);
        resizeSection(2, newSection1Size);
        adjustTotalWidth();     //补偿长度，否则会出现Table长度缩短的情况
        blockSignals(false);
    }
    // 第二列被调整：改变第四列宽度
    else if (logicalIndex == 2) {
        if (newSize < MIN_WIDTH) {
            blockSignals(true);
            resizeSection(2, MIN_WIDTH);
            blockSignals(false);
            return;
        }
        int delta = newSize - oldSize;
        int section4Size = sectionSize(4);
        int newSection4Size = section4Size - delta;
        if (newSection4Size < MIN_WIDTH) {
            //qDebug() << "block 2 section";
            blockSignals(true);
            resizeSection(2, oldSize);
            blockSignals(false);
            return;
        }

        // 防止递归调用
        blockSignals(true);
        resizeSection(4, newSection4Size);
        adjustTotalWidth();
        blockSignals(false);
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

void TableHeaderView::initSize(const QTableView *view)
{
    //初始化尺寸，0 3 4固定大小，1 2列按剩余尺寸分配
    setSectionResizeMode(QHeaderView::Fixed);
    resizeSection(0, 30);
    resizeSection(3, 50);
    resizeSection(4, 80);
    setSectionResizeMode(1, QHeaderView::Interactive);
    int remainwidth = view->width() - sectionSize(0) - sectionSize(3) - sectionSize(4);
    int res1 = 0, res2 = 0;
    distrubution(remainwidth, 3, 2, res1, res2);
    resizeSection(1, res1);
    setSectionResizeMode(2, QHeaderView::Interactive);
    resizeSection(2, res2);
    qDebug() << remainwidth << res1 << res2;
    _initStatus = true;
}
