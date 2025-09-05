#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "complexionwidget.h"
#include "selectlocmusic_dlg.h"
#include "tableview/tableview.h"
#include "FFPlayer/FFPlayer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_closeBtn_clicked();

    void on_minimizeBtn_clicked();

    void on_zoomBtn_clicked(bool checked);

    void on_complexionBtn_clicked();

    void changeSkinColor(QColor showcaseColor);

    void on_setBtn_clicked();

    void on_selectDir_Btn_clicked();

private:
    void initBaseFuncLWg();     //initialize Base function ListWidget
    void scanFileToTableView(QStringList list);
    float calculateLuminace(const QColor &color);
    void calculateColors(const QColor& showcaseColor, QColor& listColor, QColor& uiColor, QColor& bottomColor);
    void readLocalMusicConfig();

private:
    Ui::MainWindow *ui;
    complexionWidget* compWidget = nullptr;
    SelectLocMusic_Dlg* selectlocmusic_dlg = nullptr;
    TableView* _am_view = nullptr;
    TableView* _lm_view = nullptr;
};

#endif // MAINWINDOW_H
