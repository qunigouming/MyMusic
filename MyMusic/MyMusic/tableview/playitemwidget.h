#ifndef PLAYITEMWIDGET_H
#define PLAYITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include "elidedlabel.h"

class PlayItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayItemWidget(QWidget *parent = nullptr);

    void setSongIcon(const QPixmap& pixmap);
    void setSongName(const QString& name);
    void setAuthorName(const QString& name);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:

private:
    void updateLayout();
    void updateTextWidth();

    QLabel* _songIcon = nullptr;
    ElidedLabel* _songName = nullptr;
    QLabel* _songTip = nullptr;
    ElidedLabel* _authorName = nullptr;

    QLabel* _downLoad = nullptr;
    QLabel* _save = nullptr;
    QLabel* _comment = nullptr;
    QLabel* _more = nullptr;

    QWidget* _textContainer = nullptr;
    QWidget* _funcContainer = nullptr;

    bool _showText = true;
    int _minTextWidth = 80;
    int _iconWidth = 40;
    int _funcWidth = 120;
};

#endif // PLAYITEMWIDGET_H
