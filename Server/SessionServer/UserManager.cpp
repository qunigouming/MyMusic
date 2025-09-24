#include "UserManager.h"
#include "Session.h"

UserManager::~UserManager()
{
    _sessions.clear();
}

std::shared_ptr<Session> UserManager::GetSession(int uid)
{
    std::lock_guard<std::mutex> lock(_session_mutex);
    auto iter = _sessions.find(uid);
    if (iter != _sessions.end())    return iter->second;
    return nullptr;
}

void UserManager::AddSession(int uid, std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(_session_mutex);
    _sessions[uid] = session;
}

void UserManager::RemoveSession(int uid, std::string session_id)
{
    std::lock_guard<std::mutex> lock(_session_mutex);
    auto iter = _sessions.find(uid);
    if (iter == _sessions.end())    return;
    auto session_uid = iter->second->GetSessionId();
    // 若不是代表其他端登录
    if (session_uid != session_id) return;
    _sessions.erase(uid);
}
