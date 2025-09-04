#ifndef COMPLEXIONWIDGET_H
#define COMPLEXIONWIDGET_H

/******************************************************************************
 *
 * @file       complexionwidget.h
 * @brief      complexion Widget Function, be used to show complexion change functional
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

#include <QDialog>

class MainWindow;

namespace Ui {
class complexionWidget;
}

class complexionWidget : public QDialog
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit complexionWidget(QWidget *parent = nullptr);
    ~complexionWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::complexionWidget *ui;
    QColor _skinColor;

signals:
    void changeColor(QColor showcaseColor);
};

#endif // COMPLEXIONWIDGET_H
