#include "AIChatClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QNetworkReply>

#include "LogManager.h"

AIChatClient::AIChatClient(QObject* parent)
	: QObject(parent), _systemPrompt("You are a helpful AI assistant."), _hasReceivedRole(false)
{
	QSettings settings(":/config.ini", QSettings::IniFormat);
	_model = settings.value("AIChat/Model").toString();
	_apiKey = settings.value("AIChat/ApiKey").toString();
	_requestUrl = settings.value("AIChat/Url").toString();

	_manager = new QNetworkAccessManager(this);
	connect(_manager, &QNetworkAccessManager::finished, this, &AIChatClient::onReplyFinished);
}

AIChatClient::~AIChatClient()
{
	if (_reply) {
		_reply->deleteLater();
	}
}

void AIChatClient::sendMessage(const QString & message, const QJsonArray & history)
{
	emit requestStarted();

	if (!history.isEmpty()) {
		_conversationHistory = history;
	}

	QJsonObject json;
	json["role"] = "user";
	json["content"] = message;
	_conversationHistory.append(json);
    
	QJsonObject requestJson = buildRequestJson(message, _conversationHistory, false);

	QNetworkRequest request(_requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer " + _apiKey).toUtf8());

	if (_reply) {
		_reply->deleteLater();
	}
	_reply = _manager->post(request, QJsonDocument(requestJson).toJson());
}

void AIChatClient::sendMessageStream(const QString& message, const QJsonArray& history)
{
	emit requestStarted();

	if (!history.isEmpty()) {
		_conversationHistory = history;
	}

	// 添加用户消息到历史
	QJsonObject userMessage;
	userMessage["role"] = "user";
	userMessage["content"] = message;
	_conversationHistory.append(userMessage);

	// 构建请求
	QJsonObject requestJson = buildRequestJson(message, _conversationHistory, true);

	// 创建网络请求
	QNetworkRequest request(_requestUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(_apiKey).toUtf8());
	request.setRawHeader("Accept", "text/event-stream");
	request.setRawHeader("Cache-Control", "no-cache");

	if (_reply) {
		_reply->abort();
		_reply->deleteLater();
	}
	QByteArray requestData = QJsonDocument(requestJson).toJson(QJsonDocument::Compact);
	_reply = _manager->post(request, requestData);
	_reply->setProperty("isStream", true);
	connect(_reply, &QNetworkReply::readyRead, this, &AIChatClient::onReadyRead);
	connect(_reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError error) {
		LOG(ERROR) << "网络错误: " << error << " - " << _reply->errorString().toStdString();
		emit errorOccurred(QString("网络错误: %1").arg(_reply->errorString()));
	});
}

void AIChatClient::clearHistory()
{
	_conversationHistory = QJsonArray();
}

void AIChatClient::onReadyRead()
{
	if (!_reply) {
		return;
	}

	QByteArray newData = _reply->readAll();
	LOG(INFO) << "ReadyRead newData: " << QString::fromUtf8(newData).toStdString();
	processStreamData(newData);
}

void AIChatClient::onReplyFinished(QNetworkReply* reply)
{
	if (reply != _reply) {
		return;
	}

	if (reply->error() == QNetworkReply::NoError) {
		// 非流式响应处理
		if (!reply->property("isStream").toBool()) {
			QByteArray responseData = reply->readAll();
			QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

			if (!jsonDoc.isNull()) {
				QJsonObject jsonObj = jsonDoc.object();
				if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
					QJsonArray choices = jsonObj["choices"].toArray();
					if (!choices.isEmpty()) {
						QJsonObject choice = choices[0].toObject();
						if (choice.contains("message")) {
							QJsonObject message = choice["message"].toObject();
							QString content = message["content"].toString().trimmed();

							// 添加到历史
							QJsonObject assistantMessage;
							assistantMessage["role"] = "assistant";
							assistantMessage["content"] = content;
							_conversationHistory.append(assistantMessage);

							emit responseReceived(content);
						}
					}
				}
				else if (jsonObj.contains("error")) {
					QString errorMsg = jsonObj["error"].toObject()["message"].toString();
					emit errorOccurred(QString("API Error: %1").arg(errorMsg));
				}
			}
			else {
				emit errorOccurred("Failed to parse API response");
			}
		}

		emit requestFinished();
	}
	else {
		// 处理错误
		QString errorString;
		if (reply->error() == QNetworkReply::OperationCanceledError) {
			errorString = "Request cancelled";
		}
		else {
			errorString = reply->errorString();

			// 尝试读取错误响应
			QByteArray errorData = reply->readAll();
			if (!errorData.isEmpty()) {
				QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
				if (!errorDoc.isNull() && errorDoc.object().contains("error")) {
					QJsonObject errorObj = errorDoc.object()["error"].toObject();
					errorString = QString("API Error: %1").arg(errorObj["message"].toString());
				}
			}
		}

		emit errorOccurred(errorString);
		emit requestFinished();
	}

	reply->deleteLater();
	if (reply == _reply) {
		_reply = nullptr;
	}
}

QJsonObject AIChatClient::buildRequestJson(const QString& message, const QJsonArray& history, bool stream /*= false*/)
{
	QJsonArray messages;

	if (!_systemPrompt.isEmpty()) {
        QJsonObject systemMessage;
		systemMessage["role"] = "system";
        systemMessage["content"] = _systemPrompt;
        messages.append(systemMessage);
	}

	for (const auto& msg : history) {
		messages.append(msg.toObject());
	}

	QJsonObject requestJson;
	requestJson["model"] = _model;
	requestJson["messages"] = messages;
	requestJson["stream"] = stream;

	requestJson["thinking"] = QJsonObject{ {"type", "enabled"} };
	requestJson["max_tokens"] = 2048;

	return requestJson;
}

void AIChatClient::processStreamData(const QByteArray& data)
{
	QStringList events = parseSSE(data);

	LOG(INFO) << "解析到事件数量:" << events.size();

	for (int i = 0; i < events.size(); ++i) {
		const QString& event = events[i];

		if (event.startsWith("data: ")) {
			QString jsonStr = event.mid(6).trimmed();

			if (jsonStr == "[DONE]") {
				LOG(INFO) << "收到流结束标记[DONE]";

				// 添加最终响应到历史
				if (!_currentStreamResponse.isEmpty()) {
					QJsonObject assistantMessage;
					assistantMessage["role"] = "assistant";
					assistantMessage["content"] = _currentStreamResponse;
					_conversationHistory.append(assistantMessage);

					LOG(INFO) << "完整响应内容:" << _currentStreamResponse.toStdString();
				}

				// 清空当前响应
				_currentStreamResponse.clear();
				emit streamFinished();
				return;
			}

			QJsonParseError parseError;
			QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);

			if (parseError.error != QJsonParseError::NoError) {
				LOG(WARNING) << "JSON解析错误:" << parseError.errorString().toStdString() << "在位置" << parseError.offset;
				LOG(WARNING) << "问题JSON:" << jsonStr.toStdString();
				continue;
			}

			if (!jsonDoc.isNull()) {
				QJsonObject jsonObj = jsonDoc.object();

				if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
					QJsonArray choices = jsonObj["choices"].toArray();
					if (!choices.isEmpty()) {
						QJsonObject choice = choices[0].toObject();

						if (choice.contains("delta")) {
							QJsonObject delta = choice["delta"].toObject();

							// 处理常规content - 修复：检查是否是字符串类型
							if (delta.contains("content")) {
								QJsonValue contentValue = delta["content"];
								// content可能是字符串或null
								if (contentValue.isString()) {
									QString chunk = contentValue.toString();
									LOG(INFO) << "收到内容块:" << chunk.toStdString() << "(长度:" << chunk.length() << ")";

									if (!chunk.isEmpty()) {
										_currentStreamResponse += chunk;
										emit streamChunkReceived(chunk);
									}
								}
								else if (contentValue.isNull()) {
									// content为null是正常的，特别是在R1模型的开始
									LOG(INFO) << "content为null，跳过";
								}
							}

							// 处理reasoning_content
							if (delta.contains("reasoning_content")) {
								QJsonValue reasoningValue = delta["reasoning_content"];
								if (reasoningValue.isString()) {
									QString reasoningChunk = reasoningValue.toString();
									if (!reasoningChunk.isEmpty()) {
										LOG(INFO) << "收到推理内容块:" << reasoningChunk.toStdString();
										// 发送推理内容到UI
										emit streamChunkReceived(reasoningChunk, false);
									}
								}
							}

							// 处理role（只打印一次）
							if (delta.contains("role") && !_hasReceivedRole) {
								QString role = delta["role"].toString();
								LOG(INFO) << "角色:" << role.toStdString();
								_hasReceivedRole = true;
							}
						}

						// 处理finish_reason
						if (choice.contains("finish_reason")) {
							QJsonValue finishReasonValue = choice["finish_reason"];
							if (finishReasonValue.isString()) {
								QString finishReason = finishReasonValue.toString();
								LOG(INFO) << "完成原因:" << finishReason.toStdString();
							}
						}
					}
				}
				else if (jsonObj.contains("error")) {
					QString errorMsg = jsonObj["error"].toObject()["message"].toString();
					LOG(WARNING) << "流式响应中的错误:" << errorMsg.toStdString();
					emit errorOccurred(QString("流式错误: %1").arg(errorMsg));
					return;
				}
			}
		}
	}
}

QStringList AIChatClient::parseSSE(const QByteArray& data)
{
	QStringList events;
	QString buffer = QString::fromUtf8(data);

	QStringList lines = buffer.split("\n");
	QString currentEvent;

	for (const QString& line : lines) {
		if (line.isEmpty()) {
			if (!currentEvent.isEmpty()) {
				events.append(currentEvent);
				currentEvent.clear();
			}
		}
		else {
			if (!currentEvent.isEmpty()) {
				currentEvent += "\n";
			}
			currentEvent += line;
		}
	}

	if (!currentEvent.isEmpty()) {
		events.append(currentEvent);
	}

	return events;
}

