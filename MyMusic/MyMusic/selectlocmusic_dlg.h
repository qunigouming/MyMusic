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
    void createDefaultConfig();         // 创建默认配置
    bool readConfigJson();              // 读取配置文件到配置信息中
    void saveToFile();                  // 保存配置信息到配置文件中
    const QStringList getFilePaths();  // 获取文件路径
    void changeStatus();
    void initSelectLocalMusicDlg();     // 初始化选择目录控件

private:
    Ui::SelectLocMusic_Dlg *ui;
    QJsonObject _config;
};

#endif // SELECTLOCMUSIC_DLG_H
