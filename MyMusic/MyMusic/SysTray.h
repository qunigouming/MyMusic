#pragma once

#include <QSystemTrayIcon>
#include <QWidget>
#include <QMenu>

class SysTray : public QSystemTrayIcon
{
	Q_OBJECT

public:
	SysTray(QWidget *parent);
	~SysTray();

private:
	void initSysTray();
	void initMenu();

private:
    QWidget* _parent;
	QMenu* _menu = nullptr;
};

