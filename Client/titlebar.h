#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QEvent>
#include <QPushButton>
#include <QLabel>
#include <QPoint>
#include <QPixmap>
#include <QMouseEvent>
#include <QTimer>
#include <QPainterPath>

enum class ButtonType{
    MIN_BUTTON,      //最小化/关闭按钮
    MIN_MAX_BUTTON,      //最小/最大化/关闭按钮
    ONLY_CLOSE_BUTTON     //只有关闭按钮
};

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent);

    //设置标题栏背景色和是否设置标题栏背景色透明
    void setBackgroundColor(int r = 153, int g = 153, int b = 153, bool isTransparent = false);
    //设置标题栏图标
    void setTitleIcon(QString filepath, QSize IconSize = QSize(25,25));
    //设置标题内容
    void setTitleContent(QString titleContent, int titleFontSize = 9);
    //设置标题栏长度
    void setTitleWidth(int width);
    //设置标题栏上的按钮类型
    void setButtonType(ButtonType buttonType = ButtonType::MIN_MAX_BUTTON);
    //设置标题栏中的标题是否会滚动
    void setTitleRoll();
    //设置窗口边框宽度
    void setWindowBorderWidth(int borderWidth);

    //保存/获取 最大化前窗口的位置及大小
    void saveRestoreInfo(const QPoint point, const QSize size);
    void getRestoreInfo(QPoint &point, QSize &size);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


signals:
    //按钮触发的信号
    void MinBtnClicked();
    void RestoreBtnClicked();
    void MaxBtnClicked();
    void CloseBtnClicked();

public slots:
    //按钮触发的槽
    void onBtnMinClicked();
    void onBtnRestoreClicked();
    void onBtnMaxClicked();
    void onBtnCloseClicked();
    void onRollTitle();         //标题滚动

private:
    //初始化控件
    void initControl();
    //信号槽绑定
    void initConnections();
    //加载样式文件
    void loadStyleSheet(const QString &sheetName);

    QLabel *m_pIcon;             //标题图片
    QLabel *m_pTitleContent;     //标题内容
    QPushButton *m_pMinBtn;      //最小化按钮
    QPushButton *m_pRestoreBtn;  //最大化还原按钮
    QPushButton *m_pMaxBtn;      //最大化按钮
    QPushButton *m_pCloseBtn;    //关闭按钮

    //标题栏颜色
    int m_colorR, m_colorG, m_colorB;

    //最大/最小 化变量
    QPoint m_restorePos;
    QSize m_restoreSize;
    //移动窗口的变量
    bool m_isPressed;    //能否移动标志
    QPoint m_startMovePos;
    //标题栏跑马灯效果时钟
    QTimer m_titleRollTimer;
    //标题栏内容
    QString m_titleContent;
    //按钮类型
    ButtonType m_buttonType;
    //窗口边框宽度
    int m_windowBorderWidth;
    //标题栏是否透明
    bool m_isTransparent;
};

#endif // TITLEBAR_H
