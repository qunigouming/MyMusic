#include "complexionwidget.h"
#include "ui_complexionwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QColorDialog>

complexionWidget::complexionWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::complexionWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);     //设置为无边框无阴影，Popup式窗口
    //this->installEventFilter(this);
    _skinColor.setRgb(153, 212, 237);       //set default color
    connect(ui->color_lab_1, &ColorBlockLabel::clicked, [this]{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QColor color = ui->color_lab_1->palette().background().color();
#else
        QColor color = ui->color_lab_1->palette().window().color();
#endif
        _skinColor = color;
        emit changeColor(color);
    });
    connect(ui->color_lab_2, &ColorBlockLabel::clicked, [this]{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QColor color = ui->color_lab_2->palette().background().color();
#else
        QColor color = ui->color_lab_2->palette().window().color();
#endif
        _skinColor = color;
        emit changeColor(color);
    });
    connect(ui->color_lab_3, &ColorBlockLabel::clicked, [this]{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QColor color = ui->color_lab_3->palette().background().color();
#else
        QColor color = ui->color_lab_3->palette().window().color();
#endif
        _skinColor = color;
        emit changeColor(color);
    });
    connect(ui->color_lab_4, &ColorBlockLabel::clicked, [this]{
        //弹出颜色选择框
        QColor color = QColorDialog::getColor(Qt::white, this);
        if (!color.isValid()) {
            color = _skinColor;
        }
        _skinColor = color;
        emit changeColor(_skinColor);
    });
}

complexionWidget::~complexionWidget()
{
    delete ui;
}

void complexionWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;  // 创建一个QStyleOption对象
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    opt.init(this);// 初始化QStyleOption对象，传入当前窗口指针
#else
    opt.initFrom(this);
#endif
    QPainter p(this);// 创建一个QPainter对象，绘制设备为当前窗口

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);// 使用当前窗口的绘图风格绘制小部件
}

bool complexionWidget::eventFilter(QObject *obj, QEvent *event)
{
    // if (obj == this && event->type() == QEvent::MouseButtonPress) {
    //     QMouseEvent *me = static_cast<QMouseEvent*>(event);
    //     if (!this->geometry().contains(me->globalPos())) {
    //         qDebug() << "hide" << me->globalPos() << me->pos() << this->geometry();
    //         this->hide();
    //         return true;
    //     }
    // }
    return QWidget::eventFilter(obj, event);
}
