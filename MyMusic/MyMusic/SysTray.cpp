#include "SysTray.h"
#include <QApplication>

SysTray::SysTray(QWidget*parent) : QSystemTrayIcon(parent), _parent(parent)
{
	initSysTray();
	initMenu();

	show();
}

SysTray::~SysTray()
{
	hide();
	if (_menu) {
		_menu->deleteLater();
        _menu = nullptr;
	}
	setContextMenu(nullptr);
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
	_menu = new QMenu(_parent);
    _menu->addAction("退出", [this]() {
		hide();
		if (_menu) {
			_menu->deleteLater();
            _menu = nullptr;
		}
        QApplication::quit();
    });

    setContextMenu(_menu);
}

