#ifndef REGISTERDIA_H
#define REGISTERDIA_H

#include <QDialog>

namespace Ui {
class RegisterDia;
}

class RegisterDia : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDia(QWidget *parent = 0);
    ~RegisterDia();

private:
    Ui::RegisterDia *ui;
};

#endif // REGISTERDIA_H
