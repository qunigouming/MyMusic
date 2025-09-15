#pragma once

#include <QLabel>
class DragDropLabel : public QLabel
{
    Q_OBJECT
public:
    DragDropLabel(QWidget* parent = nullptr);

    void setSuffixs(const QStringList& suffixs);

signals:
    void fileDropped(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    bool hasSuffix(const QUrl& url) const;

private:
    QStringList _suffixs;
};

