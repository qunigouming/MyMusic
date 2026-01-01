#include "ChatInputWidget.h"

ChatInputWidget::ChatInputWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi();
}

void ChatInputWidget::setupUi()
{
	this->setMinimumHeight(100);

	// create main layout
	_mainLayout = new QHBoxLayout(this);

	// create top layout
	_topLayout = new QVBoxLayout;
	_clearBtn = new QPushButton("清除");
	_topLayout->addStretch();
	_topLayout->addWidget(_clearBtn);

	_mainLayout->addLayout(_topLayout);

	// create middle layout
	_textEdit = new QTextEdit;
	_mainLayout->addWidget(_textEdit);

	// create bottom layout
	_bottomLayout = new QVBoxLayout;
	_sendBtn = new QPushButton("发送");
	_emojiBtn = new QPushButton("表情");
	_bottomLayout->addWidget(_emojiBtn);
	_bottomLayout->addStretch();
	_bottomLayout->addWidget(_sendBtn);
	_mainLayout->addLayout(_bottomLayout);

	// style
	this->setStyleSheet(R"(QWidget {
		background-color: rgb(255, 255, 255);
		border: 1px solid rgb(200, 200, 200);
		border-radius: 5px;
	})");

	connect(_sendBtn, &QPushButton::clicked, this, [this] {
		emit sig_send_message(_textEdit->toPlainText());
        _textEdit->clear();
	});
}

