#pragma once

#include <QWidget>
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
	Ui::UploadWidgetClass *ui;
	QString _file_path;
};

