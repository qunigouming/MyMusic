#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonArray>

class AIChatClient  : public QObject
{
	Q_OBJECT

public:
	AIChatClient(QObject *parent);
	~AIChatClient();

	void sendMessage(const QString& message, const QJsonArray& history = QJsonArray());
	void sendMessageStream(const QString& message, const QJsonArray& history = QJsonArray());

	void clearHistory();

signals:
	void responseReceived(const QString& response);
	void streamChunkReceived(const QString& chunk, bool is_content = true);
	void streamFinished();
	void errorOccurred(const QString& error);
	void requestStarted();
	void requestFinished();

private slots:
	void onReadyRead();
	void onReplyFinished(QNetworkReply* reply);

private:
	QJsonObject buildRequestJson(const QString& message, const QJsonArray& history, bool stream = false);
	void processStreamData(const QByteArray& data);
	QStringList parseSSE(const QByteArray& data);


private:
	QNetworkAccessManager* _manager;
	QNetworkReply* _reply;
	QString _model;
	QString _apiKey;
	QString _requestUrl;
	QString _systemPrompt;
	QJsonArray _conversationHistory;

	QString _currentStreamResponse;
	bool _hasReceivedRole;
};

