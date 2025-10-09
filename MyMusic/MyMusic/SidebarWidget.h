#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

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
};

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

