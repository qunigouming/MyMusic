#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>

/*
*  description: The widget for chat input
*  create time: 2025/12/29
*/

class ChatInputWidget  : public QWidget
{
	Q_OBJECT

public:
	explicit ChatInputWidget(QWidget *parent = nullptr);
	~ChatInputWidget() = default;

signals:
	void sig_send_message(const QString& charMsg);

private:
	void setupUi();

private:
	QHBoxLayout* _mainLayout = nullptr;

	QVBoxLayout* _topLayout = nullptr;
	QPushButton* _clearBtn = nullptr;


	QTextEdit* _textEdit = nullptr;

	QVBoxLayout* _bottomLayout = nullptr;
	QPushButton* _sendBtn = nullptr;
	QPushButton* _emojiBtn = nullptr;
};

