#include "SysTray.h"
#include <QMenu>
#include <QApplication>

SysTray::SysTray(QWidget*parent) : QSystemTrayIcon(parent), _parent(parent)
{
	initSysTray();
	initMenu();

	show();
}

SysTray::~SysTray()
{
}

void SysTray::initSysTray()
{
	setToolTip("MyMusic");
    setIcon(QIcon(":/source/image/AppPic.png"));

	connect(this, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			_parent->activateWindow();
			_parent->raise();
		}
	});
}

void SysTray::initMenu()
{
	QMenu* menu = new QMenu(_parent);
    menu->addAction("退出", [this]() {
        QApplication::quit();
    });

    setContextMenu(menu);
}

