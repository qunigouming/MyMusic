#include "tableview.h"
#include <QPixmap>
#include <QScrollBar>
#include <QPaintEvent>

TableView::TableView(MusicTableViewType view_type, QWidget *parent)
    : QTableView(parent), _type(view_type)
{
    //view property
    setFixedSize(900, 600);
    setFrameShape(QFrame::NoFrame);
    setShowGrid(false);     //设置为无网格
    setMouseTracking(true);

    //QOpenGLWidget *openGLWidget = new QOpenGLWidget(this);
    //setViewport(openGLWidget);

    //set model
    _model = new TableViewModel(_type, this);
    _proxyModel = new SortProxyModel(this);
    _proxyModel->setSourceModel(_model);
    setModel(_proxyModel);
    setSortingEnabled(true);

    _delegate = new SongDelegate(this);
    setItemDelegate(_delegate);
    _header = new TableHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(_header);
    _header->initSize(this);

    //addition data
    //for (int i = 1; i <= 1; ++i) {
    //    _model->addSong(SongInfo("aaa", "04:12", "10M", ""));
    //}

    //开启编辑器持久化
    //for (int i = 0; i < _proxyModel->rowCount(); ++i) openPersistentEditor(_proxyModel->index(i, 1));

    verticalHeader()->setDefaultSectionSize(50);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalHeader()->hide();
    setFocusPolicy(Qt::NoFocus);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    _batchTimer = new QTimer(this);
    _batchTimer->setInterval(100);
    connect(_batchTimer, &QTimer::timeout, this, &TableView::processPendingSongs);

    //connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TableView::updateVisibleRange);
    //connect(viewport(), &QAbstractScrollArea::, this, &TableView::updateVisibleRange);

    _scrollUpdateTimer = new QTimer(this);
    _scrollUpdateTimer->setInterval(20);
    _scrollUpdateTimer->setSingleShot(true);
    connect(_scrollUpdateTimer, &QTimer::timeout, this, [this]() {
        updatePersistentEditors();
    });

    connect(this, &TableView::doubleClicked, this, [this](const QModelIndex& index) {
        // 获取索引对应的文件路径
        SongInfo info = _model->songAt(index.row());
        emit rowDoubleClicked(info);
    });

    setStyleSheet(R"(
    QTableView {
        background-color: #91ceea;
        border: none;
    }

    QTableView::item:hover {
        background: #7ab8d9;
    }

    QTableView::item:selected {
        background: #7ab8d9;
    }
    /* 垂直滚动条整体 */
    QScrollBar:vertical {
        border: none;
        background: transparent;
        width: 6px;
        margin: 0px;
    }
        
    /* 垂直滚动条手柄 */
    QScrollBar::handle:vertical {
        background-color: rgba(160, 160, 160, 180);
        border-radius: 3px;
        min-height: 100px;
        margin: 0px;
    }
        
    /* 垂直滚动条手柄悬停 */
    QScrollBar::handle:vertical:hover {
        background-color: rgba(120, 120, 120, 200);
    }
    
    /* 垂直滚动条手柄按下 */
    QScrollBar::handle:vertical:pressed {
        background-color: rgba(80, 80, 80, 220);
    }
    
    /* 隐藏所有不必要的滚动条部分 */
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical,
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical,
    QScrollBar::up-arrow:vertical,
    QScrollBar::down-arrow:vertical {
        background: transparent;
        border: none;
        height: 0px;
        width: 0px;
    }
    )");
}

TableView::~TableView() {
    if (_batchTimer) {
        _batchTimer->stop();
        delete _batchTimer;
    }
}

void TableView::addSong(const SongInfo& song)
{
    // 添加到待处理列表
    _pendingSongs.append(song);

    if (!_batchTimer->isActive()) {
        _batchTimer->start();
    }
}

void TableView::addSong(const QList<SongInfo>& songs)
{
    _pendingSongs.append(songs);

    if (!_batchTimer->isActive()) {
        _batchTimer->start();
    }
}

void TableView::clearAllSongs()
{
    if (_batchTimer && _batchTimer->isActive()) {
        _batchTimer->stop();
    }

    _pendingSongs.clear();
    for (int row : _editorRows) {
        QModelIndex index = _proxyModel->index(row, 1);
        if (index.isValid()) {
            closePersistentEditor(index);
        }
    }
    _editorRows.clear();
    if (_model) {
        _model->clearAllSongs();
    }

    setVisibleProxyRange(-1, -1);

    _hoveredRow = -1;
    if (_delegate) {
        _delegate->setHoveredRow(-1);
    }

    viewport()->update();
}

int TableView::rowCount()
{
    return _proxyModel->rowCount();
}

void TableView::setBatchSize(int size)
{
    _batchSize = qMax(1, size);
}

SongInfo TableView::getSongInfoByProxyRow(int proxyRow) const
{
    if (!_proxyModel || !_model)    return SongInfo("", "", "", "");
    QModelIndex proxyIndex = _proxyModel->index(proxyRow, 0);
    QModelIndex sourceIndex = _proxyModel->mapToSource(proxyIndex);
    return _model->songAt(sourceIndex.row());
}

void TableView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    int row = index.isValid() ? index.row() : -1;
    if (row != _hoveredRow) {
        // 更新悬停行
        _hoveredRow = row;
        // 通知委托悬停行变化
        if (_delegate) {
            _delegate->setHoveredRow(row);
        }
        // 重绘视图
        viewport()->update();
    }

    QTableView::mouseMoveEvent(event);
}

void TableView::leaveEvent(QEvent *event)
{
    if (_hoveredRow != -1) {
        _hoveredRow = -1;

        // 通知委托悬停行变化
        if (_delegate) {
            _delegate->setHoveredRow(-1);
        }

        // 重绘视图
        viewport()->update();
    }

    QTableView::leaveEvent(event);
}

void TableView::scrollContentsBy(int dx, int dy)
{
    QTableView::scrollContentsBy(dx, dy);
    updateVisibleRange();
    if (_scrollUpdateTimer) {
        _scrollUpdateTimer->stop();
        _scrollUpdateTimer->start();
    }
        // updatePersistentEditors();
}

void TableView::resizeEvent(QResizeEvent* event)
{
    QTableView::resizeEvent(event);
    updateVisibleRange();
    updatePersistentEditors();
}

void TableView::paintEvent(QPaintEvent* event)
{
    if (_visibleStart == -1 || _visibleEnd == -1) {
        // 没有设置范围时，正常绘制
        QTableView::paintEvent(event);
        return;
    }

    // 计算可视范围的矩形区域
    QRect visibleRect;
    if (_visibleStart <= _visibleEnd) {
        int top = rowViewportPosition(_visibleStart);
        int bottom = rowViewportPosition(_visibleEnd) + rowHeight(_visibleEnd);
        visibleRect = QRect(0, top, viewport()->width(), bottom - top);
    }

    // 只绘制可视范围内的区域
    QPaintEvent filteredEvent(event->region() & visibleRect);
    QTableView::paintEvent(&filteredEvent);
}

void TableView::updateVisibleRange()
{
    if (!isVisible())   return;
    if (!_proxyModel)   return;

    QRect viewportRect = viewport()->rect();
    int startRow = rowAt(viewportRect.top());
    int endRow = rowAt(viewportRect.bottom());

    qDebug() << "View rows:" << startRow << "to" << endRow;

        // 处理边界情况
    if (startRow == -1) startRow = 0;
    if (endRow == -1) endRow = _proxyModel->rowCount() - 1;

    // 增加缓冲行，避免滚动时频繁加载
    int buffer = 10;
    int newStart = qMax(0, startRow - buffer);
    int newEnd = qMin(_model->rowCount() - 1, endRow + buffer);

    setVisibleProxyRange(newStart, newEnd);
}

void TableView::updatePersistentEditors()
{
    // 计算当前可视行
    QSet<int> newVisibleRows;
    QRect viewportRect = viewport()->rect();
    int startRow = rowAt(viewportRect.top());
    int endRow = rowAt(viewportRect.bottom());

    if (startRow != -1 && endRow != -1) {
        for (int row = startRow; row <= endRow; ++row) {
            newVisibleRows.insert(row);
        }
    }

    // 添加缓冲行
    int buffer = 10;
    for (int row = qMax(0, startRow - buffer); row <= qMin(_proxyModel->rowCount() - 1, endRow + buffer); ++row) {
        newVisibleRows.insert(row);
    }

    // 可视区域真正变化时才处理编辑器
    if (newVisibleRows != _visibleRows) {
        // 找出需要关闭编辑器的行
        QSet<int> rowsToClose = _editorRows - newVisibleRows;
        for (int row : rowsToClose) {
            QModelIndex index = _proxyModel->index(row, 1);
            if (index.isValid()) {
                closePersistentEditor(index);
            }
        }

        // 找出需要开启编辑器的行
        QSet<int> rowsToOpen = newVisibleRows - _editorRows;
        for (int row : rowsToOpen) {
            QModelIndex index = _proxyModel->index(row, 1);
            if (index.isValid()) {
                openPersistentEditor(index);
            }
        }

        // 更新记录
        _visibleRows = newVisibleRows;
        _editorRows = newVisibleRows;
    }
}

void TableView::setVisibleProxyRange(int start, int end)
{
    _visibleStart = start;
    _visibleEnd = end;
    viewport()->update();
}

void TableView::processPendingSongs() {
    if (_pendingSongs.isEmpty()) {
        _batchTimer->stop();
        emit allSongsAdded();       // 通知外部添加完成
        return;
    }

    int processCount = qMin(_batchSize, _pendingSongs.size());
    QList<SongInfo> batch = _pendingSongs.mid(0, processCount);
    _pendingSongs = _pendingSongs.mid(processCount);

    _model->addSong(batch);

    for (int i = 0; i < batch.size(); ++i) {
        QModelIndex index = _proxyModel->mapFromSource(_model->index(_model->rowCount() - batch.size() + i, 1));
        if (index.isValid()) {
            openPersistentEditor(index);
        }
    }

    updateVisibleRange();
    updatePersistentEditors();

    viewport()->update();
}
