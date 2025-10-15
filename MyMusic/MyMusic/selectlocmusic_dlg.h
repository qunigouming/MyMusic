#ifndef SELECTLOCMUSIC_DLG_H
#define SELECTLOCMUSIC_DLG_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class SelectLocMusic_Dlg;
}

class SelectLocMusic_Dlg : public QDialog
{
    Q_OBJECT

public:
    explicit SelectLocMusic_Dlg(QWidget *parent = nullptr);
    ~SelectLocMusic_Dlg();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void sig_selectDir(const QStringList& filePaths);

private slots:
    void on_close_Btn_clicked();

    void on_add_file_Btn_clicked();

    void on_showMenu(const QPoint &pos);

private:
    void initSelectLocalMusicDlg();     // 初始化选择目录控件

private:
    Ui::SelectLocMusic_Dlg *ui;
};

#endif // SELECTLOCMUSIC_DLG_H
