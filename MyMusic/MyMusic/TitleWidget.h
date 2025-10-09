#pragma once

#include <QWidget>

class TitleWidget  : public QWidget
{
	Q_OBJECT

public:
	TitleWidget(const QString& title, QWidget *parent = nullptr);
	~TitleWidget();
};

