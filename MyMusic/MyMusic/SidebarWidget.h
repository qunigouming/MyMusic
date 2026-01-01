#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

// 侧边栏导航栏项目类型
enum class SidebarItemType {
	MainPage = 0,
	AIChat,
	FavoriteMusic,
	MyCollection,
	LatestPlay,
	LocalMusic
};

inline bool operator<(SidebarItemType lhs, SidebarItemType rhs) {
	return static_cast<int>(lhs) < static_cast<int>(rhs);
}

/*
*  description: 侧边栏组件，包含了导航栏和歌单栏
*/
class SidebarComponent : public QWidget {
	Q_OBJECT

public:
    SidebarComponent(QWidget *parent = nullptr);

signals:
	void itemClicked(int index);

private:
	void setupUI();
	void addNavItem(const QChar& icon, const QString& text);
	void addPlaylistItem(const QString& icon_url, const QString& name);

private:
    QVBoxLayout *_mainLayout = nullptr;

	int _itemCount = 0;
	int _currentIndex = -1;
	QWidget* _currentSelectedItem = nullptr;
};

/* 
*  description: 用于主窗口的侧边栏， 该类直接提供给外部使用
*/
class SidebarWidget : public QWidget
{
	Q_OBJECT

public:
	SidebarWidget(QWidget *parent);
	~SidebarWidget();

signals:
	void itemClicked(int index);

private:
	QScrollArea *_scrollArea = nullptr;
};

