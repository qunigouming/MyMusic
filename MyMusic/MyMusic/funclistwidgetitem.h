#ifndef FUNCLISTWIDGETITEM_H
#define FUNCLISTWIDGETITEM_H

#include <QLabel>
#include <QWidget>

class FuncListWidgetItem : public QWidget
{
    Q_OBJECT
public:
    explicit FuncListWidgetItem(QChar icon, QString text, QWidget *parent = nullptr);

private:
    QLabel* icon_Lab = nullptr;
    QLabel* title_Lab = nullptr;
};

#endif // FUNCLISTWIDGETITEM_H
