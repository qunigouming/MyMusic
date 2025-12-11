#include "selectlocmusic_dlg.h"
#include "ui_selectlocmusic_dlg.h"
#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMenu>
#include "LocalDataManager.h"

SelectLocMusic_Dlg::SelectLocMusic_Dlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectLocMusic_Dlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::Dialog);
    setModal(true);     //设置为模态对话框
    ui->titleWidget->installEventFilter(this);
    ui->close_Btn->setText(QChar(0xe67d));

    initSelectLocalMusicDlg();
}

SelectLocMusic_Dlg::~SelectLocMusic_Dlg()
{
    delete ui;
}

bool SelectLocMusic_Dlg::eventFilter(QObject *obj, QEvent *event)
{
    static QPoint dragPosition;
    //对标题栏进行事件判断
    if (obj == ui->titleWidget){
        //鼠标按下时记录位置
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos();      //记录鼠标全局位置
#else
                dragPosition = e->globalPosition().toPoint();
#endif
                return true;
            }
        }
        //鼠标移动并且左键按下，移动窗口
        if (event->type() == QEvent::MouseMove){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QPoint tempPos = e->globalPos() - dragPosition;     //用一个临时位置记录鼠标移动变化
#else
                QPoint tempPos = e->globalPosition().toPoint() - dragPosition;
#endif
                move(this->pos() + tempPos);        //鼠标移动多少，窗口走动多少
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos();
#else
                dragPosition = e->globalPosition().toPoint();
#endif
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj,event);
}

void SelectLocMusic_Dlg::on_close_Btn_clicked()
{
    // 保存配置到文件
    LocalDataManager::GetInstance()->saveConfig();
    emit sig_selectDir(LocalDataManager::GetInstance()->getFilePaths());
    deleteLater();
}

void SelectLocMusic_Dlg::on_add_file_Btn_clicked()
{
    QString file_path = QFileDialog::getExistingDirectory(this, QDir::currentPath());
    if (file_path.isEmpty()) return;

    QCheckBox* new_CheckBox = new QCheckBox(file_path, this);
    ui->pathWidget->layout()->addWidget(new_CheckBox);

    LocalDataManager::GetInstance()->setPathStatus(file_path, false);
}

void SelectLocMusic_Dlg::on_showMenu(const QPoint &pos)
{
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (!checkbox)  return;
    QMenu contextMenu(this);
    QAction* deleteAction = new QAction(tr("删除"), this);
    connect(deleteAction, &QAction::triggered, this, [this, checkbox]{
        // 修改配置
        LocalDataManager::GetInstance()->delPath(checkbox->text());

        checkbox->deleteLater();
    });

    contextMenu.addAction(deleteAction);
    contextMenu.exec(checkbox->mapToGlobal(pos));
}

void SelectLocMusic_Dlg::initSelectLocalMusicDlg()
{
    QStringList paths = LocalDataManager::GetInstance()->getFilePaths();
    // 将json中的配置生成为QCheckBox
    for (const QString& path : paths) {
        QCheckBox* check_box = new QCheckBox(path, this);
        check_box->setChecked(LocalDataManager::GetInstance()->getPathStatus(path));
        check_box->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(check_box, &QCheckBox::customContextMenuRequested, this, &SelectLocMusic_Dlg::on_showMenu);
        connect(check_box, &QCheckBox::clicked, this, [this, path](bool status){
            LocalDataManager::GetInstance()->setPathStatus(path, status);
        });
        ui->pathWidget->layout()->addWidget(check_box);
    }
}

