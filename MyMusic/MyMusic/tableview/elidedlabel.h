#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

/******************************************************************************
 *
 * @file       elidedlabel.h
 * @brief      The class be used to Elided Text, that can elide lengthier text.
 *
 * @author     qunigouming
 * @date       2025/07/18
 * @history
 *****************************************************************************/

#include <QLabel>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget* parent = nullptr);
    ElidedLabel(QString str, QWidget *parent = nullptr);
    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const { return m_elideMode; }

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void updateCachedTexts();
private:
    Qt::TextElideMode m_elideMode = Qt::ElideRight;
    QString m_cachedElidedText;
    QString m_cachedText;
};

#endif // ELIDEDLABEL_H
