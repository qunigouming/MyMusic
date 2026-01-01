#include "ChatPage.h"

ChatPage::ChatPage(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
}

void ChatPage::setupUI()
{
	_mainLayout = new QHBoxLayout(this);

	_bubbleFrame = new BubbleFrame();
	_mainLayout->addWidget(_bubbleFrame);

	_chatInput = new ChatInputWidget();
    _mainLayout->addWidget(_chatInput);

	connect(_chatInput, &ChatInputWidget::sig_send_message, _bubbleFrame, &BubbleFrame::addMessage);
}

