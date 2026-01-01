#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>

/*
*  description: chat bubble frame, be used to show chat message with AI
*  create time: 2025/12/29
*/

class BubbleFrame  : public QWidget
{
	Q_OBJECT
public:
	explicit BubbleFrame(QWidget *parent = nullptr);
	~BubbleFrame() = default;

public slots:
	void addMessage(const QString& msg);

private:
	QScrollArea* _scrollArea;
	QHBoxLayout* _mainLayout;
};

