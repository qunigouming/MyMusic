#include "registerdia.h"
#include "ui_registerdia.h"

RegisterDia::RegisterDia(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDia)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
}

RegisterDia::~RegisterDia()
{
    delete ui;
}
