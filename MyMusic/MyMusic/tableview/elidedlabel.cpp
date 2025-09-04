#include "elidedlabel.h"
#include <QPaintEvent>

ElidedLabel::ElidedLabel(QWidget *parent) : QLabel(parent) {}

ElidedLabel::ElidedLabel(QString str, QWidget *parent) : QLabel(str, parent) {}

void ElidedLabel::setElideMode(Qt::TextElideMode elideMode)
{
    m_elideMode = elideMode;
    m_cachedText.clear();
    update();
}

void ElidedLabel::paintEvent(QPaintEvent *e)
{
    QRect updateRect = e->rect();
    if (updateRect.isEmpty())   return;
    if (m_elideMode == Qt::ElideNone)
        return QLabel::paintEvent(e);
    updateCachedTexts();
    QLabel::setText(m_cachedElidedText);
    QLabel::paintEvent(e);
    QLabel::setText(m_cachedText);
}

void ElidedLabel::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    m_cachedText.clear();
}

void ElidedLabel::updateCachedTexts()
{
    const auto txt = text();
    if (m_cachedText == txt)
        return;
    m_cachedText = txt;
    const QFontMetrics fm(fontMetrics());
    m_cachedElidedText = fm.elidedText(text(),
                                       m_elideMode,
                                       width(),
                                       Qt::TextShowMnemonic);
}
