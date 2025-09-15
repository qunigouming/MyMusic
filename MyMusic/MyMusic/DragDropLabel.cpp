#include "DragDropLabel.h"
#include <QFileInfo>
#include <QMimeData>
#include <QStringList>
#include <QDragEnterEvent>
#include <QDropEvent>

DragDropLabel::DragDropLabel(QWidget* parent)
    : QLabel(parent)
{
    setAcceptDrops(true);
}

void DragDropLabel::setSuffixs(const QStringList& suffixs)
{
    _suffixs = suffixs;
}

void DragDropLabel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        qDebug() << "DragDropLabel::dragEnterEvent" << urls;
        bool allSuffix = false;
        for (auto& url : urls) {
            if (hasSuffix(url)) {
                allSuffix = true;
                break;
            }
        }

        if (allSuffix) {
            event->acceptProposedAction();
        }
        else {
            event->ignore();
        }
    }
    else {
        event->ignore();
    }
}

void DragDropLabel::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        qDebug() << "DragDropLabel::dragMoveEvent" << urls;
        bool allSuffix = false;
        for (auto& url : urls) {
            if (hasSuffix(url)) {
                allSuffix = true;
                break;
            }
        }

        if (allSuffix) {
            event->acceptProposedAction();
        }
        else {
            event->ignore();
        }
    }
    else {
        event->ignore();
    }
}

void DragDropLabel::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        qDebug() << "DragDropLabel::dropEvent" << urls;
        if (urls.empty())   return;
        QString filePath = urls.first().toLocalFile();
        if (filePath.isEmpty())    return;
        emit fileDropped(filePath);
        event->acceptProposedAction();
    }
}

bool DragDropLabel::hasSuffix(const QUrl& url) const
{
    QString filePath = url.toLocalFile();
    QFileInfo fileInfo(filePath);
    QString fileSuffix = fileInfo.suffix().toLower();
    return _suffixs.contains(fileSuffix);
}
