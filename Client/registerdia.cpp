#include "registerdia.h"
#include "ui_registerdia.h"

RegisterDia::RegisterDia(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDia)
{
    ui->setupUi(this);
}

RegisterDia::~RegisterDia()
{
    delete ui;
}
