#pragma once

#include <QWidget>
#include <QPointer>
#include <QProgressDialog>
#include "ui_UploadWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UploadWidgetClass; };
QT_END_NAMESPACE

class UploadWidget : public QWidget
{
	Q_OBJECT

public:
	UploadWidget(QWidget *parent = nullptr);
	~UploadWidget();

private:
	void ShowUploadProgressDialog();
	void UpdateUploadProgressDialog(int transSize, int totalSize);
	void CloseUploadProgressDialog();

private:
	Ui::UploadWidgetClass *ui;
	QString _file_path;
	QPointer<QProgressDialog> _upload_progress_dlg;
};

