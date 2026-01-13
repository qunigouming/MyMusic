#include "LocalDataManager.h"
#include "LogManager.h"

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include "Common\Encrypt\Encrypt.h"

#define CONFIG_FILE_NAME "/config.json"

void LocalDataManager::saveConfig()
{
    QFile file(QDir::currentPath() + CONFIG_FILE_NAME);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QJsonDocument doc(_config);
    file.write(doc.toJson());
    file.close();
}

const QStringList LocalDataManager::getFilePaths()
{
    QStringList paths;
    QJsonObject selectPathObj = _config[KSELECT_PATH_KEY].toObject();
    for (auto& key : selectPathObj.keys()) {
        if (selectPathObj[key].toBool()) {
            paths.append(key);
        }
    }
    return paths;
}

void LocalDataManager::setPathStatus(QString path, bool status)
{
    QJsonObject selectPathObj = _config[KSELECT_PATH_KEY].toObject();
    selectPathObj[path] = status;
    _config[KSELECT_PATH_KEY] = selectPathObj;
    saveConfig();
}

bool LocalDataManager::getPathStatus(QString path)
{
    QJsonObject selectPathObj = _config[KSELECT_PATH_KEY].toObject();
    return selectPathObj[path].toBool();
}

void LocalDataManager::delPath(QString path)
{
    QJsonObject selectPathObj = _config[KSELECT_PATH_KEY].toObject();
    selectPathObj.remove(path);
    _config[KSELECT_PATH_KEY] = selectPathObj;
    saveConfig();
}

void LocalDataManager::setUserPwd(QString user_name, QString passwd)
{
    QJsonObject userDataObj = _config[KUSER_DATA_KEY].toObject();
    userDataObj["user_name"] = user_name;
    Encrypt encrypt;
    userDataObj["password"] = QString::fromStdString(encrypt.AES_Encrypt(passwd.toStdString()));
    _config[KUSER_DATA_KEY] = userDataObj;
    saveConfig();
}

const QString LocalDataManager::getUserName()
{
    QJsonObject userDataObj = _config[KUSER_DATA_KEY].toObject();
    return userDataObj["user_name"].toString();
}

const QString LocalDataManager::getPasswd()
{
    Encrypt encrypt;
    QJsonObject userDataObj = _config[KUSER_DATA_KEY].toObject();
    return QString::fromStdString(encrypt.AES_Decrypt(userDataObj["password"].toString().toStdString()));;
}

void LocalDataManager::setAutoFillIn(bool status)
{
    QJsonObject userDataObj = _config[KUSER_DATA_KEY].toObject();
    userDataObj["is_auto_fill_in"] = status;
    _config[KUSER_DATA_KEY] = userDataObj;
    saveConfig();
}

bool LocalDataManager::isAutoFillIn()
{
    QJsonObject userDataObj = _config[KUSER_DATA_KEY].toObject();
    if (userDataObj.isEmpty())  return false;
    return userDataObj["is_auto_fill_in"].toBool();
}

void LocalDataManager::setVolume(int volume)
{
    QJsonObject userDataObj = _config[KUSER_USE_CONFIG_KEY].toObject();
    userDataObj["volume"] = volume;
    _config[KUSER_USE_CONFIG_KEY] = userDataObj;
    saveConfig();           // TODO: 该值不应该频繁保存，此处只是为了测试，后续可能会添加定时保存功能
}

int LocalDataManager::getVolume()
{
    QJsonObject userDataObj = _config[KUSER_USE_CONFIG_KEY].toObject();
    if (userDataObj.isEmpty())  return 60;
    return userDataObj["volume"].toInt();
}

void LocalDataManager::setEQValues(QVector<int> values)
{
    QJsonObject userDataObj = _config[KUSER_USE_CONFIG_KEY].toObject();
    QJsonArray array;
    for (int value : values) {
        array.append(value);
    }
    userDataObj["CustomEQ"] = array;
    saveConfig();
}

QVector<int> LocalDataManager::getEQValues()
{
    QJsonObject userDataObj = _config[KUSER_USE_CONFIG_KEY].toObject();
    QVector<int> values(10, 0);
    if (userDataObj.isEmpty())  return values;
    QJsonArray array = userDataObj["CustomEQ"].toArray();
    if (array.size() == 0) return values;
    for (int i = 0; i < 10; ++i) {
        if (array[i].isDouble()) {
            values[i] = array[i].toInt();
        }
    }
    return values;
}

LocalDataManager::LocalDataManager()
{
    if (!readConfig()) {
        LOG(ERROR) << "Read Config File Failed!!!";
    }
}

bool LocalDataManager::readConfig()
{
    QFile file(QDir::currentPath() + CONFIG_FILE_NAME);
    // 存在文件则读取配置，否则创建文件并写入默认配置
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        createDefaultConfig();
        QJsonDocument doc(_config);
        file.write(doc.toJson());
        file.close();
    }
    else {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            if (doc.isObject()) {
                _config = doc.object();
            }
            else return false;
        }
        else return false;
    }
    return true;
}

void LocalDataManager::createDefaultConfig()
{
    _config = QJsonObject();
    QJsonObject selectPathObj;
    selectPathObj.insert(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), true);
    selectPathObj.insert(QStandardPaths::writableLocation(QStandardPaths::MusicLocation), true);

    _config.insert(KSELECT_PATH_KEY, selectPathObj);
}
