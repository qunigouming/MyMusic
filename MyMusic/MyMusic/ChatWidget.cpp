#include "ChatWidget.h"
#include <QShortcut>
#include <QMessageBox>
#include <QScrollBar>
#include "LogManager.h"

ChatWidget::ChatWidget(QWidget *parent)
	: QWidget(parent), m_client(new AIChatClient(this)), m_isStreaming(false)
{
	setupUI();
	setupConnections();
}

ChatWidget::~ChatWidget()
{}

void ChatWidget::onSendClicked()
{
	QString message = m_inputEdit->toPlainText().trimmed();
	if (message.isEmpty()) {
		return;
	}

	// 添加用户消息到显示
	appendMessage("您", message, true);

	// 清空输入框
	m_inputEdit->clear();

	// 发送请求
	if (m_streamCheckBox->isChecked()) {
		m_isStreaming = true;

		appendMessage("DeepSeek", "", false);
		// 创建一个带有占位符的消息用于流式更新
		// appendStreamingMessage();
		m_client->sendMessageStream(message);
	}
	else {
		m_client->sendMessage(message);
	}
}


void ChatWidget::onClearClicked()
{
	int result = QMessageBox::question(this, "确认", "确定要清空对话历史吗？",
		QMessageBox::Yes | QMessageBox::No);

	if (result == QMessageBox::Yes) {
		m_chatDisplay->clear();
		m_client->clearHistory();
		m_statusLabel->setText("对话历史已清空");
		m_statusLabel->setStyleSheet(
			"QLabel {"
			"  background-color: #fff3cd;"
			"  border: 1px solid #ffeaa7;"
			"  border-radius: 3px;"
			"  padding: 5px;"
			"  color: #856404;"
			"}"
		);

		QTimer::singleShot(3000, this, [this]() {
			m_statusLabel->setText("就绪");
			m_statusLabel->setStyleSheet(
				"QLabel {"
				"  background-color: #e8f5e8;"
				"  border: 1px solid #c8e6c9;"
				"  border-radius: 3px;"
				"  padding: 5px;"
				"  color: #2e7d32;"
				"}"
			);
		});
	}
}

void ChatWidget::onResponseReceived(const QString& response)
{
	appendMessage("DeepSeek", response, false);
	m_statusLabel->setText("响应完成");
}

void ChatWidget::onStreamChunkReceived(const QString& chunk, bool is_content)
{
	LOG(INFO) << "received UI chunk data: " << chunk.toStdString();

	appendStreamingMessage(chunk, is_content);
}

void ChatWidget::onStreamFinished()
{
	QTextCursor cursor = m_chatDisplay->textCursor();
	cursor.movePosition(QTextCursor::End);
	cursor.insertText("\n\n");
	m_isStreaming = false;
	m_statusLabel->setText("流式响应完成");
}

void ChatWidget::onErrorOccurred(const QString& errorMessage)
{
	m_statusLabel->setText("错误: " + errorMessage);
	m_statusLabel->setStyleSheet(
		"QLabel {"
		"  background-color: #f8d7da;"
		"  border: 1px solid #f5c6cb;"
		"  border-radius: 3px;"
		"  padding: 5px;"
		"  color: #721c24;"
		"}"
	);

	QTimer::singleShot(5000, this, [this]() {
		m_statusLabel->setText("就绪");
		m_statusLabel->setStyleSheet(
			"QLabel {"
			"  background-color: #e8f5e8;"
			"  border: 1px solid #c8e6c9;"
			"  border-radius: 3px;"
			"  padding: 5px;"
			"  color: #2e7d32;"
			"}"
		);
	});

	QMessageBox::warning(this, "请求错误", errorMessage);
}

void ChatWidget::onRequestStarted()
{
	m_sendButton->setEnabled(false);
	m_sendButton->setText("发送中...");
	m_statusLabel->setText("正在请求...");
	m_statusLabel->setStyleSheet(
		"QLabel {"
		"  background-color: #d1ecf1;"
		"  border: 1px solid #bee5eb;"
		"  border-radius: 3px;"
		"  padding: 5px;"
		"  color: #0c5460;"
		"}"
	);
}

void ChatWidget::onRequestFinished()
{
	m_sendButton->setEnabled(true);
	m_sendButton->setText("发送");
	updateSendButton();
}

void ChatWidget::updateSendButton()
{
	bool hasText = !m_inputEdit->toPlainText().trimmed().isEmpty();

	m_sendButton->setEnabled(hasText && !m_isStreaming);
}

void ChatWidget::setupUI()
{
	// 创建主布局
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	// 聊天显示区域
	m_chatDisplay = new QTextEdit(this);
	m_chatDisplay->setReadOnly(true);
	m_chatDisplay->setStyleSheet(
		"QTextEdit {"
		"  background-color: #f5f5f5;"
		"  border: 1px solid #ddd;"
		"  border-radius: 5px;"
		"  padding: 10px;"
		"  font-family: 'Segoe UI', Arial, sans-serif;"
		"}"
	);
	mainLayout->addWidget(m_chatDisplay, 4);

	// 状态栏
	m_statusLabel = new QLabel("就绪", this);
	m_statusLabel->setStyleSheet(
		"QLabel {"
		"  background-color: #e8f5e8;"
		"  border: 1px solid #c8e6c9;"
		"  border-radius: 3px;"
		"  padding: 5px;"
		"  color: #2e7d32;"
		"}"
	);
	mainLayout->addWidget(m_statusLabel);

	// 输入区域
	QHBoxLayout* inputLayout = new QHBoxLayout();

	m_inputEdit = new QTextEdit(this);
	m_inputEdit->setMaximumHeight(100);
	m_inputEdit->setPlaceholderText("输入您的问题... (按Ctrl+Enter发送)");
	m_inputEdit->setStyleSheet(
		"QTextEdit {"
		"  border: 2px solid #4CAF50;"
		"  border-radius: 5px;"
		"  padding: 8px;"
		"  font-family: 'Segoe UI', Arial, sans-serif;"
		"}"
	);
	inputLayout->addWidget(m_inputEdit, 4);

	// 发送按钮
	m_sendButton = new QPushButton("发送", this);
	m_sendButton->setStyleSheet(
		"QPushButton {"
		"  background-color: #4CAF50;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 5px;"
		"  padding: 10px 20px;"
		"  font-weight: bold;"
		"}"
		"QPushButton:hover {"
		"  background-color: #45a049;"
		"}"
		"QPushButton:disabled {"
		"  background-color: #cccccc;"
		"}"
	);
	inputLayout->addWidget(m_sendButton);

	mainLayout->addLayout(inputLayout);

	// 控制按钮区域
	QHBoxLayout* controlLayout = new QHBoxLayout();

	m_clearButton = new QPushButton("清空对话", this);

	m_streamCheckBox = new QCheckBox("流式响应", this);
	m_streamCheckBox->setChecked(true);

	controlLayout->addWidget(m_clearButton);
	controlLayout->addStretch();
	controlLayout->addWidget(m_streamCheckBox);

	mainLayout->addLayout(controlLayout);

	// 设置字体
	QFont font("Segoe UI", 10);
	m_chatDisplay->setFont(font);
	m_inputEdit->setFont(font);
}

void ChatWidget::setupConnections()
{
	connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
	connect(m_clearButton, &QPushButton::clicked, this, &ChatWidget::onClearClicked);

	connect(m_client, &AIChatClient::responseReceived, this, &ChatWidget::onResponseReceived);
	connect(m_client, &AIChatClient::streamChunkReceived, this, &ChatWidget::onStreamChunkReceived);
	connect(m_client, &AIChatClient::streamFinished, this, &ChatWidget::onStreamFinished);
	connect(m_client, &AIChatClient::errorOccurred, this, &ChatWidget::onErrorOccurred);
	connect(m_client, &AIChatClient::requestStarted, this, &ChatWidget::onRequestStarted);
	connect(m_client, &AIChatClient::requestFinished, this, &ChatWidget::onRequestFinished);

	connect(m_inputEdit, &QTextEdit::textChanged, this, &ChatWidget::updateSendButton);

	// 快捷键：Ctrl+Enter发送
	QShortcut* sendShortcut = new QShortcut(QKeySequence("Ctrl+Return"), this);
	connect(sendShortcut, &QShortcut::activated, this, &ChatWidget::onSendClicked);
}

void ChatWidget::appendMessage(const QString& sender, const QString& message, bool isUser)
{
	QTextCursor cursor = m_chatDisplay->textCursor();
	cursor.movePosition(QTextCursor::End);

	// 保存当前格式
	QTextCharFormat originalFormat = cursor.charFormat();

	// 创建时间戳
	QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");

	// 设置块格式（对齐）
	QTextBlockFormat blockFormat;
	if (isUser) {
		blockFormat.setAlignment(Qt::AlignRight);
		blockFormat.setRightMargin(20);  // 右边距
	}
	else {
		blockFormat.setAlignment(Qt::AlignLeft);
		blockFormat.setLeftMargin(20);   // 左边距
	}
	//blockFormat.setTopMargin(10);        // 上边距
	//blockFormat.setBottomMargin(10);     // 下边距
	cursor.setBlockFormat(blockFormat);

	// 设置发送者名称格式
	QTextCharFormat senderFormat;
	if (isUser) {
		senderFormat.setForeground(QColor("#2c3e50"));  // 用户消息颜色
	}
	else {
		senderFormat.setForeground(QColor("#2980b9"));  // AI消息颜色
	}
	senderFormat.setFontWeight(QFont::Bold);

	// 设置时间戳格式
	QTextCharFormat timeFormat;
	timeFormat.setForeground(QColor("#7f8c8d"));
	timeFormat.setFontPointSize(9);

	// 设置消息内容格式
	QTextCharFormat messageFormat;
	messageFormat.setForeground(QColor("#2c3e50"));

	// 插入发送者名称
	cursor.setCharFormat(senderFormat);
	cursor.insertText(sender + " ");

	// 插入时间戳
	cursor.setCharFormat(timeFormat);
	cursor.insertText("[" + timestamp + "]");

	// 换行
	cursor.insertText("\n");

	// 插入消息内容（可以有多行）
	cursor.setCharFormat(messageFormat);
	cursor.insertText(message);

	// 添加消息分隔
	cursor.insertText("\n\n");

	// 恢复原始格式
	cursor.setCharFormat(originalFormat);

	// 滚动到底部
	QScrollBar* scrollbar = m_chatDisplay->verticalScrollBar();
	scrollbar->setValue(scrollbar->maximum());
}

void ChatWidget::appendStreamingMessage(const QString& chunk, bool is_content)
{
	QTextCursor cursor = m_chatDisplay->textCursor();
	QTextCharFormat originalFormat = cursor.charFormat();
	cursor.movePosition(QTextCursor::End);
	if (!is_content) {
		QTextCharFormat inferMsgFormat;
		inferMsgFormat.setForeground(QColor("#6c6c6c"));
		inferMsgFormat.setFontPointSize(8);
		inferMsgFormat.setFontWeight(100);
		cursor.setCharFormat(inferMsgFormat);
	}
	else {
		QTextCharFormat contentFormat;
		contentFormat.setForeground(QColor("#2c3e50"));  // 正常文本颜色
		contentFormat.setFontPointSize(10);              // 正常字体大小
		contentFormat.setFontWeight(QFont::Normal);      // 正常字体粗细
		cursor.setCharFormat(contentFormat);
	}

	// 插入内容块
	cursor.insertText(chunk);

	cursor.setCharFormat(originalFormat);

	// 滚动到底部
	QScrollBar* scrollbar = m_chatDisplay->verticalScrollBar();
	scrollbar->setValue(scrollbar->maximum());
}