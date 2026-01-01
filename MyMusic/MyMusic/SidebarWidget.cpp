#include "SidebarWidget.h"
#include "SeparatorWidget.h"
#include "TitleWidget.h"
#include "NavItemWidget.h"
#include "PlaylistItemWidget.h"
#include <QHBoxLayout>

SidebarWidget::SidebarWidget(QWidget *parent)
	: QWidget(parent)
{
	_scrollArea = new QScrollArea(this);

    // _scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    SidebarComponent* sidebar = new SidebarComponent();
    _scrollArea->setWidget(sidebar);

    // sidebar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << "Width: " << this->width();
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(_scrollArea);

    setLayout(mainLayout);
    connect(sidebar, &SidebarComponent::itemClicked, this, &SidebarWidget::itemClicked);
}

SidebarWidget::~SidebarWidget()
{}

SidebarComponent::SidebarComponent(QWidget *parent)
	: QWidget(parent)
{
    setupUI();
}

void SidebarComponent::setupUI()
{
    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setSpacing(0);

    addNavItem(QChar(0xe007), "主页(beta)");
    addNavItem(QChar(0xe01A), tr("AI聊天"));
    
    _mainLayout->addWidget(new SeparatorWidget());

    _mainLayout->addWidget(new TitleWidget("我的"));
    addNavItem(QChar(0xe016), "喜欢的音乐");
    addNavItem(QChar(0xe018), "我的收藏");
    addNavItem(QChar(0xe017), "最近播放");
    addNavItem(QChar(0xe008), "本地音乐");

    _mainLayout->addWidget(new SeparatorWidget());

    _mainLayout->addWidget(new TitleWidget("创建的歌单"));

    _mainLayout->addStretch();
}

void SidebarComponent::addNavItem(const QChar& icon, const QString& text)
{
    NavItemWidget* navItem = new NavItemWidget(icon, text, _itemCount++);
    _mainLayout->addWidget(navItem);

    connect(navItem, &NavItemWidget::clicked, this, [this, navItem](int index) {
        // 取消之前选中的
        if (_currentSelectedItem && _currentIndex != index) {
            if (NavItemWidget* item = qobject_cast<NavItemWidget*>(_currentSelectedItem)) {
                item->setSelected(false);
            }
            else if (PlaylistItemWidget* item = qobject_cast<PlaylistItemWidget*>(_currentSelectedItem)) {
                item->setSelected(false);
            }
        }
        navItem->setSelected(true);
        _currentSelectedItem = navItem;
        _currentIndex = index;

        itemClicked(index);
    });
}

void SidebarComponent::addPlaylistItem(const QString& icon_url, const QString& name)
{
    PlaylistItemWidget* playlistItem = new PlaylistItemWidget(icon_url, name, _itemCount++);
    _mainLayout->addWidget(playlistItem);

    connect(playlistItem, &PlaylistItemWidget::clicked, this, [this, playlistItem](int index) {
        // 取消之前选中的
        if (_currentSelectedItem && _currentIndex != index) {
            if (NavItemWidget* item = qobject_cast<NavItemWidget*>(_currentSelectedItem)) {
                item->setSelected(false);
            }
            else if (PlaylistItemWidget* item = qobject_cast<PlaylistItemWidget*>(_currentSelectedItem)) {
                item->setSelected(false);
            }
        }
        playlistItem->setSelected(true);
        _currentSelectedItem = playlistItem;
        _currentIndex = index;

        itemClicked(index);
    });
}
