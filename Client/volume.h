#ifndef VOLUME_H
#define VOLUME_H

#include <QDialog>

namespace Ui {
class Volume;
}

class Volume : public QDialog
{
    Q_OBJECT

public:
    explicit Volume(QWidget *parent = 0);
    ~Volume();

signals:
    void SendVolume(int volume);

public slots:
    void SetNoVolume(int volume);         //设置静音
    void SetVolume();           //恢复音量

private slots:
    void on_verticalSlider_valueChanged(int value);

private:
    int m_volume;           //保存静音前的值
    Ui::Volume *ui;
};

#endif // VOLUME_H
