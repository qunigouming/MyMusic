#ifndef SET_H
#define SET_H

#include <QWidget>

namespace Ui {
class Set;
}

class Set : public QWidget
{
    Q_OBJECT

public:
    explicit Set(QWidget *parent = 0);
    ~Set();

private:
    Ui::Set *ui;

};

#endif // SET_H
