#include "volume.h"
#include "ui_volume.h"

Volume::Volume(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Volume)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
}

Volume::~Volume()
{
    delete ui;
}

//滑块值改变时发送信号
void Volume::on_verticalSlider_valueChanged(int value)
{
    emit SendVolume(value);
}

//设置静音，静音前保存值
void Volume::SetNoVolume(int volume)
{
    m_volume = ui->verticalSlider->value();
    ui->verticalSlider->setValue(volume);
}

//恢复静音
void Volume::SetVolume()
{
    ui->verticalSlider->setValue(m_volume);
}
