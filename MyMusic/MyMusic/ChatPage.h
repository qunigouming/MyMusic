#pragma once

#include <QWidget>
#include "BubbleFrame.h"
#include "ChatInputWidget.h"

/*
*  description: The Class be used to show the chat with robot
*  create time: 2025/12/29
*/
class ChatPage : public QWidget
{
	Q_OBJECT
public:
	explicit ChatPage(QWidget *parent = nullptr);
	~ChatPage() = default;

private:
	void setupUI();

private:
	QHBoxLayout* _mainLayout;
    BubbleFrame* _bubbleFrame;
    ChatInputWidget* _chatInput;
};

