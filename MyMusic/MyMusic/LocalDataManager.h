#pragma once

#include "Singleton.h"
#include <QJsonObject>

class LocalDataManager : public Singleton<LocalDataManager>
{
	friend class Singleton<LocalDataManager>;

public:
	~LocalDataManager() = default;

	void saveConfig();

	const QStringList getFilePaths();
	void setPathStatus(QString path, bool status);
	bool getPathStatus(QString path);
	void delPath(QString path);

	void setUserPwd(QString user_name, QString passwd);
    const QString getUserName();
    const QString getPasswd();
	void setAutoFillIn(bool status);		// 设置下次是否自动填充用户名密码
	bool isAutoFillIn();

	void setVolume(int volume);
	int getVolume();

	void setEQValues(QVector<int> values);
	QVector<int> getEQValues();
private:
    LocalDataManager();
	bool readConfig();
	void createDefaultConfig();
private:
	QJsonObject _config;
	const QString KSELECT_PATH_KEY = "SelectPath";			// 本地音乐选择路径
	const QString KUSER_DATA_KEY = "UserData";				// 用户个人数据
	const QString KUSER_USE_CONFIG_KEY = "UserUseConfig";	// 用户使用配置
};

