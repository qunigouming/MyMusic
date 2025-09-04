#include "playitemwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPainter>
#include <QCoreApplication>
#include "tableview.h"

PlayItemWidget::PlayItemWidget(QWidget *parent)
    : QWidget{parent}
{
    setStyleSheet(R"(font-size: 18px;)");

    _songIcon = new QLabel(this);
    _songIcon->setPixmap(QPixmap(":/source/icon/08.jpg").scaled(_iconWidth, _iconWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _songIcon->setFixedSize(_iconWidth, _iconWidth);
    _songIcon->setAlignment(Qt::AlignCenter);
    _songIcon->setScaledContents(true);

    //create text container
    _textContainer = new QWidget(this);
    QVBoxLayout* textLayout = new QVBoxLayout(_textContainer);
    textLayout->setContentsMargins(5, 0, 10, 0);
    textLayout->addStretch(5);

    //set song name label
    _songName = new ElidedLabel("test", _textContainer);
    _songName->setStyleSheet("font-weight: bold; font-size: 14px;");
    _songName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _songName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _songName->setTextFormat(Qt::PlainText);
    _songName->setWordWrap(false);
    _songName->setElideMode(Qt::ElideRight);  // 设置省略号模式

    //set vip label
    _songTip = new QLabel(_textContainer);
    _songTip->setPixmap(QPixmap(":/source/icon/vip.png").scaled(20, 20));
    _songTip->setFixedSize(20, 20);

    //set author label
    _authorName = new ElidedLabel("authorName", _textContainer);
    _authorName->setStyleSheet("font-size: 10px;");
    _authorName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _authorName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _authorName->setTextFormat(Qt::PlainText);
    _authorName->setWordWrap(false);
    _authorName->setElideMode(Qt::ElideRight);  // 设置省略号模式

    //add into the layout
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(5);
    nameLayout->addWidget(_songName, 1);

    QHBoxLayout* authorLayout = new QHBoxLayout();
    authorLayout->setContentsMargins(0, 0, 0, 0);
    authorLayout->addWidget(_songTip);
    authorLayout->addWidget(_authorName);

    textLayout->addLayout(nameLayout);
    textLayout->addLayout(authorLayout);

    //set Label Iconfont
    _downLoad = new QLabel(this);
    _downLoad->setFont(QFont("iconfont"));
    _downLoad->setText(QChar(0xe635));
    _downLoad->setFixedSize(20, 20);
    _save = new QLabel(this);
    _save->setFont(QFont("iconfont"));
    _save->setText(QChar(0xe609));
    _save->setFixedSize(20, 20);
    _comment = new QLabel(this);
    _comment->setFont(QFont("iconfont"));
    _comment->setText(QChar(0xe891));
    _comment->setFixedSize(20, 20);
    _more = new QLabel(this);
    _more->setFont(QFont("iconfont"));
    _more->setText(QChar(0xe61c));
    _more->setFixedSize(20, 20);

    //set StyleSheet
    _downLoad->setStyleSheet("QLabel {color: #666;} QLabel:hover {color: #1e90ff;}");
    _save->setStyleSheet("QLabel {color: #666;} QLabel:hover {color: #1e90ff;}");
    _comment->setStyleSheet("QLabel {color: #666;} QLabel:hover {color: #1e90ff;}");
    _more->setStyleSheet("QLabel {color: #666;} QLabel:hover {color: #1e90ff;}");

    _downLoad->setToolTip(tr("下载"));
    _save->setToolTip(tr("收藏"));
    _comment->setToolTip(tr("评论"));
    _more->setToolTip(tr("更多"));

    _funcContainer = new QWidget(this);
    QHBoxLayout* funcLayout = new QHBoxLayout(_funcContainer);
    funcLayout->setContentsMargins(0, 0, 0, 0);
    funcLayout->setSpacing(15);
    funcLayout->addWidget(_downLoad);
    funcLayout->addWidget(_save);
    funcLayout->addWidget(_comment);
    funcLayout->addWidget(_more);

    //set funcContainer is Fixed
    _funcContainer->setFixedSize(_funcWidth, 30);

    //set button is unVisible
    _downLoad->setVisible(false);
    _save->setVisible(false);
    _comment->setVisible(false);
    _more->setVisible(false);

    setLayout(new QVBoxLayout());
}

void PlayItemWidget::setSongIcon(const QPixmap &pixmap)
{
    QPixmap scaled = pixmap.scaled(_iconWidth, _iconWidth,
                                   Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _songIcon->setPixmap(scaled);
    update();
}

void PlayItemWidget::setSongName(const QString& name)
{
    _songName->setText(name);
}

void PlayItemWidget::setAuthorName(const QString& name)
{
    _authorName->setText(name);
}

void PlayItemWidget::enterEvent(QEnterEvent* event)
{
    _downLoad->setVisible(true);
    _save->setVisible(true);
    _comment->setVisible(true);
    _more->setVisible(true);
    update();

    // 转发事件到父级 通知该事件给view，从而达到行悬浮
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMouseEvent* mouseEvent = new QMouseEvent(
        QEvent::MouseMove,
        mapToParent(rect().center()),
        Qt::NoButton,
        Qt::NoButton,
        Qt::NoModifier
        );
#else
    QPoint localPos = mapToParent(rect().center());
    QPoint globalPos = window()->mapToGlobal(localPos);
    QMouseEvent* mouseEvent = new QMouseEvent(
        QEvent::MouseMove,
        localPos,
        globalPos,
        Qt::NoButton,
        Qt::NoButton,
        Qt::NoModifier
        );
#endif
    QCoreApplication::postEvent(parent(), mouseEvent);

    QWidget::enterEvent(event);
}

void PlayItemWidget::leaveEvent(QEvent *event)
{
    _downLoad->setVisible(false);
    _save->setVisible(false);
    _comment->setVisible(false);
    _more->setVisible(false);
    update();

    // 转发事件到父级
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QMouseEvent* mouseEvent = new QMouseEvent(
        QEvent::MouseMove,
        QPoint(-1, -1),  // 无效位置表示离开
        Qt::NoButton,
        Qt::NoButton,
        Qt::NoModifier
        );
#else
    QMouseEvent* mouseEvent = new QMouseEvent(
        QEvent::MouseMove,
        QPoint(-1, -1),
        QPoint(-1, -1),
        Qt::NoButton,
        Qt::NoButton,
        Qt::NoModifier
        );
#endif
    QCoreApplication::postEvent(parent(), mouseEvent);

    QWidget::leaveEvent(event);
}

void PlayItemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void PlayItemWidget::updateLayout()
{
    // 固定图标位置
    _songIcon->move(10, (height() - _songIcon->height()) / 2);

    // 固定功能按钮位置
    int funcX = width() - _funcContainer->width() - 10;
    int funcY = (height() - _funcContainer->height()) / 2;
    _funcContainer->move(funcX, funcY);

    // 更新文本区域位置和大小
    updateTextWidth();
}

void PlayItemWidget::updateTextWidth()
{
    // 计算文本区域可用宽度
    int textX = _songIcon->x() + _songIcon->width() + 10;
    int textWidth = _funcContainer->x() - textX - 10;

    // 判断是否显示文本内容
    bool showText = (textWidth >= _minTextWidth);

    if (showText != _showText) {
        _showText = showText;
        _textContainer->setVisible(_showText);
    }

    // 设置文本区域位置和大小
    if (_showText) {
        _textContainer->setGeometry(textX, 0, textWidth, height());
        _textContainer->setVisible(true);
    } else {
        _textContainer->setVisible(false);
    }
}
