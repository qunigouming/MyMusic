#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

/******************************************************************************
 *
 * @file       windowmanager.h
 * @brief      The class usually be used to manage the all kinds of primary window
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

#include <QMainWindow>
#include "logindialog.h"
#include "mainwindow.h"
#include <QSharedPointer>
#include "SysTray.h"

class WindowManager : public QMainWindow
{
    Q_OBJECT
public:
    explicit WindowManager(QWidget *parent = nullptr);
    ~WindowManager() = default;

private slots:
    void slotSwitchMainWindow();

private:
    QSharedPointer<LoginDialog> _loginDialog;
    QSharedPointer<MainWindow> _mainWindow;
    QSharedPointer<SysTray> _sysTray;
};

#endif // WINDOWMANAGER_H
