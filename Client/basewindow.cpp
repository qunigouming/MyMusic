#include "basewindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QFile>

BaseWindow::BaseWindow(QWidget *parent) : QWidget(parent)
{
    //去除边框
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //关闭窗口时释放资源
    setAttribute(Qt::WA_DeleteOnClose);
    //初始化标题栏
    initTitleBar();
}

void BaseWindow::onBtnMinClicked()
{
    showMinimized();        //最小化
}

void BaseWindow::onBtnRestoreClicked()
{
    QPoint windowPos;
    QSize windowSize;
    m_titleBar->getRestoreInfo(windowPos,windowSize);   //获取最大化前的大小和位置
    this->setGeometry(QRect(windowPos,windowSize));     //设置位置
}

void BaseWindow::onBtnMaxClicked()
{
    //设置标题栏位置
    m_titleBar->saveRestoreInfo(this->pos(),QSize(this->width(),this->height()));
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    setGeometry(FactRect);      //设置窗体位置
}

void BaseWindow::onBtnCloseClicked()
{
    close();
}

void BaseWindow::initTitleBar()
{
    m_titleBar = new TitleBar(this);
    m_titleBar->move(0,0);      //移动到窗口左上角
    m_titleBar->setButtonType();        //手动进行按钮类型初始化设置
    m_titleBar->raise();

    connect(m_titleBar,&TitleBar::MinBtnClicked,this,&BaseWindow::onBtnMinClicked);
    connect(m_titleBar,&TitleBar::RestoreBtnClicked,this,&BaseWindow::onBtnRestoreClicked);
    connect(m_titleBar,&TitleBar::MaxBtnClicked,this,&BaseWindow::onBtnMaxClicked);
    connect(m_titleBar,&TitleBar::CloseBtnClicked,this,&BaseWindow::onBtnCloseClicked);
}

//绘制窗体的背景颜色
//void BaseWindow::paintEvent(QPaintEvent *event)
//{
//    //设置背景色
//    QPainter painter(this);
//    QPainterPath pathBack;
//    pathBack.setFillRule(Qt::WindingFill);
//    pathBack.addRoundedRect(QRect(0,0,this->width(),this->height()),3,3);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform); //指示引擎应该使用平滑的像素映射转换算法(如双线性)而不是最近邻。
//    painter.fillPath(pathBack,QBrush(QColor(238,223,204)));     //设置填充颜色
//    return QWidget::paintEvent(event);
//}

void BaseWindow::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/" + sheetName + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen()){
        QString styleSheet = this->styleSheet();        //获取样式
        styleSheet += QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }
}
