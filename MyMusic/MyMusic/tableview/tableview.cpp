#include "tableview.h"
#include <QPixmap>
#include <QScrollBar>

TableView::TableView(MusicTableViewType view_type, QWidget *parent)
    : QTableView(parent), _type(view_type)
{
    //view property
    // setFixedSize(900, 600);
    setMinimumSize(QSize(900, 600));
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

    // 排序后行序变化：重置悬停状态并整体重绘（编辑器机制已移除，无需开/关持久编辑器）
    connect(_proxyModel, &QSortFilterProxyModel::layoutChanged, this, [this]() {
        _hoveredRow = -1;
        _hoveredButton = -1;
        if (_delegate) {
            _delegate->setHoveredRow(-1);
            _delegate->setHoveredButton(-1);
        }
        viewport()->update();
    });

    // 功能按钮点击转发（预留接口：后续在外部连接 rowButtonClicked 实现下载/收藏等逻辑）
    connect(_delegate, &SongDelegate::buttonClicked, this, &TableView::rowButtonClicked);

    connect(_delegate, &SongDelegate::likeChanged, this, [this](const int row, const bool status) {
        // 注意：row 是代理行索引，排序后与源行不一致，必须先映射回源模型再取 id
        QModelIndex srcIndex = _proxyModel->mapToSource(_proxyModel->index(row, 3));
        emit likeChanged(_model->songAt(srcIndex.row()).id, status);
    });

    // 网络图标下载完成后，只重绘对应行
    connect(_delegate, &SongDelegate::iconLoaded, this, [this](const QString&, int sourceRow) {
        QModelIndex proxyIndex = _proxyModel->mapFromSource(_model->index(sourceRow, 1));
        if (proxyIndex.isValid()) {
            update(proxyIndex);
        }
    });

    _header = new TableHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(_header);
    _header->initSize(this->width());

    verticalHeader()->setDefaultSectionSize(50);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalHeader()->hide();
    setFocusPolicy(Qt::NoFocus);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    _batchTimer = new QTimer(this);
    _batchTimer->setInterval(100);
    connect(_batchTimer, &QTimer::timeout, this, &TableView::processPendingSongs);

    connect(this, &TableView::doubleClicked, this, [this](const QModelIndex& index) {
        // 获取索引对应的文件路径
        SongInfo info = getSongInfoByProxyRow(index.row());
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
    if (_model) {
        _model->clearAllSongs();
    }

    _hoveredRow = -1;
    _hoveredButton = -1;
    if (_delegate) {
        _delegate->setHoveredRow(-1);
        _delegate->setHoveredButton(-1);
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
        // 悬停行变化只重绘新旧两行，不再整视口 update
        if (_hoveredRow >= 0 && _hoveredRow < _proxyModel->rowCount()) {
            viewport()->update(rowRect(_hoveredRow));
        }
        // 更新悬停行
        _hoveredRow = row;
        // 通知委托悬停行变化
        if (_delegate) {
            _delegate->setHoveredRow(row);
        }
        if (row >= 0) {
            viewport()->update(rowRect(row));
        }
    }

    // 跟踪功能按钮悬停状态（命中变化时局部重绘对应单元格）
    updateHoveredButton(event->pos(), row);

    QTableView::mouseMoveEvent(event);
}

void TableView::leaveEvent(QEvent *event)
{
    if (_hoveredRow != -1) {
        int oldRow = _hoveredRow;
        _hoveredRow = -1;

        // 通知委托悬停行变化
        if (_delegate) {
            _delegate->setHoveredRow(-1);
        }

        // 只重绘原悬停行
        if (oldRow < _proxyModel->rowCount()) {
            viewport()->update(rowRect(oldRow));
        }
    }

    if (_hoveredButton != -1) {
        _hoveredButton = -1;
        if (_delegate) {
            _delegate->setHoveredButton(-1);
        }
    }

    QTableView::leaveEvent(event);
}

void TableView::updateHoveredButton(const QPoint &pos, int row)
{
    int button = -1;
    if (row >= 0 && _delegate) {
        QModelIndex cellIndex = _proxyModel->index(row, 1);
        if (cellIndex.isValid()) {
            // buttonRects 只需 option.rect，无需完整构造 view options
            QStyleOptionViewItem option;
            option.rect = visualRect(cellIndex);
            button = _delegate->buttonAt(pos, option, cellIndex);
        }
    }

    if (button != _hoveredButton) {
        _hoveredButton = button;
        if (_delegate) {
            _delegate->setHoveredButton(button);
        }
        // 重绘整个单元格即可覆盖按钮区域
        if (row >= 0) {
            viewport()->update(visualRect(_proxyModel->index(row, 1)));
        }
    }
}

QRect TableView::rowRect(int row) const
{
    return QRect(0, rowViewportPosition(row), viewport()->width(), rowHeight(row));
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

    viewport()->update();
}
