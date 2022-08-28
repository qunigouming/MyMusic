#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QDialog>
#include "titlebar.h"

class BaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);

signals:

public slots:
    void onBtnMinClicked();
    void onBtnRestoreClicked();
    void onBtnMaxClicked();
    void onBtnCloseClicked();

protected:
    TitleBar *m_titleBar;

private:
    void initTitleBar();
    //void paintEvent(QPaintEvent *event);
    void loadStyleSheet(const QString &sheetName);
};

#endif // BASEWINDOW_H
