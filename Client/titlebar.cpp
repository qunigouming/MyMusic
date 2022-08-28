#include "titlebar.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QDebug>

#define BUTTON_HEIGHT 30        //按钮高度
#define BUTTON_WIDTH 30         //按钮宽度
#define TITLE_HEIGHT 30         //标题栏高度

TitleBar::TitleBar(QWidget *parent) : QWidget(parent),
    m_colorR(153),
    m_colorG(153),
    m_colorB(153),
    m_isPressed(false),
    m_buttonType(ButtonType::MIN_MAX_BUTTON),
    m_windowBorderWidth(0),
    m_isTransparent(false)
{
    initControl();
    initConnections();
    loadStyleSheet("Title");
}

//设置标题栏背景颜色
void TitleBar::setBackgroundColor(int r, int g, int b, bool isTransparent)
{
    m_colorR = r;
    m_colorG = g;
    m_colorB = b;
    m_isTransparent = isTransparent;
    //使用绘画事件重新绘制窗口
    update();
}

//设置标题栏图片
void TitleBar::setTitleIcon(QString filepath, QSize IconSize)
{
    QPixmap titleIcon(filepath);
    m_pIcon->setPixmap(titleIcon.scaled(IconSize));
}

void TitleBar::setTitleContent(QString titleContent, int titleFontSize)
{
    QFont font = m_pTitleContent->font();
    font.setPointSize(titleFontSize);       //设置字体大小
    m_pTitleContent->setFont(font);
    //设置标题内容
    m_pTitleContent->setText(titleContent);
    m_titleContent = titleContent;
}

void TitleBar::setTitleWidth(int width)
{
    this->setFixedWidth(width);
}

void TitleBar::setButtonType(ButtonType buttonType)
{
    m_buttonType = buttonType;
    switch(buttonType){
        case ButtonType::MIN_BUTTON:{
            m_pMaxBtn->setVisible(false);
            m_pRestoreBtn->setVisible(false);
            break;
        }
        case ButtonType::MIN_MAX_BUTTON:{
            m_pRestoreBtn->setVisible(false);
            break;
        }
        case ButtonType::ONLY_CLOSE_BUTTON:{
            m_pMinBtn->setVisible(false);
            m_pRestoreBtn->setVisible(false);
            m_pMaxBtn->setVisible(false);
            break;
        }
        default:
            break;
    }
}

//设置标题滚动
void TitleBar::setTitleRoll()
{
    connect(&m_titleRollTimer,SIGNAL(timeout()),this,SLOT(onRollTitle()));
    m_titleRollTimer.start(200);        //滚动200毫秒
}

//设置窗口边框大小
void TitleBar::setWindowBorderWidth(int borderWidth)
{
    m_windowBorderWidth = borderWidth;
}

//保存窗口最大化前窗口的位置及大小
void TitleBar::saveRestoreInfo(const QPoint point, const QSize size)
{
    m_restorePos = point;
    m_restoreSize = size;
}

//获取窗口最大化前窗口的位置及大小
void TitleBar::getRestoreInfo(QPoint &point, QSize &size)
{
    point = m_restorePos;
    size = m_restoreSize;
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    //是否设置标题透明
    if (!m_isTransparent){
        //设置背景色
        QPainter painter(this);
        QPainterPath pathBack;
        pathBack.setFillRule(Qt::WindingFill);  //指定使用非零winding规则填充区域。
        pathBack.addRoundedRect(QRect(0,0,this->width(),this->height()),3,3);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillPath(pathBack,QBrush(QColor(m_colorR,m_colorG,m_colorB)));  //设置填充模式
    }

    //当窗口长度变化了，标题栏的长度一起变化
    if (this->width() != (this->parentWidget()->width() - m_windowBorderWidth))
        this->setFixedWidth(this->parentWidget()->width() - m_windowBorderWidth);
    QWidget::paintEvent(event);
}

//双击标题栏时进行最大化和最小化操作
void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    //判断是否有该按钮，防止出BUG
    if (m_buttonType == ButtonType::MIN_MAX_BUTTON){
        if (m_pMaxBtn->isVisible())
            onBtnMaxClicked();
        else onBtnRestoreClicked();
    }
    return QWidget::mouseDoubleClickEvent(event);
}

//鼠标按下事件
void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (m_buttonType == ButtonType::MIN_MAX_BUTTON){
        //窗口最大化时禁止拖动窗口
        if (m_pMaxBtn->isVisible()){
           m_isPressed = true;
           m_startMovePos = event->globalPos();     //记录鼠标全局位置
        }
    }
    else{
        m_isPressed = true;
        m_startMovePos = event->globalPos();
    }
    return QWidget::mousePressEvent(event);
}

//鼠标移动事件
void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed){
        QPoint movePoint = event->globalPos() - m_startMovePos;     //记录移动的位置
        QPoint widgetPos = this->parentWidget()->pos();     //获取父窗口位置
        m_startMovePos = event->globalPos();        //更新位置
        this->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());       //移动位置
    }
    return QWidget::mouseMoveEvent(event);
}

//鼠标释放事件
void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;        //释放时禁止移动，增加程序安全性
    return QWidget::mouseReleaseEvent(event);
}

void TitleBar::onBtnMinClicked()
{
    emit MinBtnClicked();
}

void TitleBar::onBtnRestoreClicked()
{
    m_pRestoreBtn->setVisible(false);
    m_pMaxBtn->setVisible(true);
    emit RestoreBtnClicked();
}

void TitleBar::onBtnMaxClicked()
{
    m_pMaxBtn->setVisible(false);
    m_pRestoreBtn->setVisible(true);
    emit MaxBtnClicked();
}

void TitleBar::onBtnCloseClicked()
{
    emit CloseBtnClicked();
}

//标题栏滚动
void TitleBar::onRollTitle()
{
    static int nPos = 0;
    QString titleContent = m_titleContent;
    //截取的字符串比原字符串长时，重新截取
    if (nPos > titleContent.length())
        nPos = 0;
    m_pTitleContent->setText(titleContent.mid(nPos));
    ++nPos;
}

//初始化各控件
void TitleBar::initControl()
{
    m_pIcon = new QLabel;
    m_pTitleContent = new QLabel;

    m_pMinBtn = new QPushButton;
    m_pRestoreBtn = new QPushButton;
    m_pMaxBtn = new QPushButton;
    m_pCloseBtn = new QPushButton;

    m_pMinBtn->setFixedSize(QSize(BUTTON_WIDTH,BUTTON_HEIGHT));
    m_pRestoreBtn->setFixedSize(QSize(BUTTON_WIDTH,BUTTON_HEIGHT));
    m_pMaxBtn->setFixedSize(QSize(BUTTON_WIDTH,BUTTON_HEIGHT));
    m_pCloseBtn->setFixedSize(QSize(BUTTON_WIDTH,BUTTON_HEIGHT));

    //设置对象名称
    m_pTitleContent->setObjectName("TitleContent");
    m_pMinBtn->setObjectName("ButtonMin");
    m_pRestoreBtn->setObjectName("ButtonRestore");
    m_pMaxBtn->setObjectName("ButtonMax");
    m_pCloseBtn->setObjectName("ButtonClose");

    //设置提示文本
    m_pMinBtn->setToolTip(QStringLiteral("最小化"));
    m_pRestoreBtn->setToolTip(QStringLiteral("还原"));
    m_pMaxBtn->setToolTip(QStringLiteral("最大化"));
    m_pCloseBtn->setToolTip(QStringLiteral("关闭"));

    //开始布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_pIcon);
    mainLayout->addWidget(m_pTitleContent);

    mainLayout->addWidget(m_pMinBtn);
    mainLayout->addWidget(m_pRestoreBtn);
    mainLayout->addWidget(m_pMaxBtn);
    mainLayout->addWidget(m_pCloseBtn);

    mainLayout->setContentsMargins(5,0,0,0);        //设置左上右底的边距
    mainLayout->setSpacing(0);

    m_pTitleContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);         //设置策略为尽可能大和永不增长和收缩
    this->setFixedHeight(TITLE_HEIGHT);         //设置高
    this->setWindowFlags(Qt::FramelessWindowHint);      //隐藏标题栏
}

void TitleBar::initConnections()
{
    connect(m_pMinBtn,&QPushButton::clicked,this,&TitleBar::onBtnMinClicked);
    connect(m_pRestoreBtn,&QPushButton::clicked,this,&TitleBar::onBtnRestoreClicked);
    connect(m_pMaxBtn,&QPushButton::clicked,this,&TitleBar::onBtnMaxClicked);
    connect(m_pCloseBtn,&QPushButton::clicked,this,&TitleBar::onBtnCloseClicked);
}

//加载样式文件
void TitleBar::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/" + sheetName + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen()){
        QString styleSheet = this->styleSheet();    //获取该窗口小部件的样式
        styleSheet += QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }
}
