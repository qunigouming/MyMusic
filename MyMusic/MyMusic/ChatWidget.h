#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <QScrollArea>
#include <QTimer>

#include "AIChatClient.h"

class ChatWidget  : public QWidget
{
	Q_OBJECT

public:
	explicit ChatWidget(QWidget *parent = nullptr);
	~ChatWidget();

private slots:
	void onSendClicked();
	void onClearClicked();
	void onResponseReceived(const QString& response);
	void onStreamChunkReceived(const QString& chunk, bool is_content);
	void onStreamFinished();
	void onErrorOccurred(const QString& errorMessage);
	void onRequestStarted();
	void onRequestFinished();
	void updateSendButton();

private:
	void setupUI();
	void setupConnections();
	void appendMessage(const QString& sender, const QString& message, bool isUser = true);
	void appendStreamingMessage(const QString& chunk, bool is_content);

private:
	// UI组件
	QTextEdit* m_chatDisplay;
	QTextEdit* m_inputEdit;
	QPushButton* m_sendButton;
	QPushButton* m_clearButton;
	QCheckBox* m_streamCheckBox;
	QLabel* m_statusLabel;

	// API客户端
	AIChatClient* m_client;

	// 状态
	bool m_isStreaming;
};

