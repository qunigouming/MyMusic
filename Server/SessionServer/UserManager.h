#pragma once

#include "Singleton.h"
#include <unordered_map>

class Session;

class UserManager : public Singleton<UserManager>
{
    friend class Singleton<UserManager>;
public:
    ~UserManager();
    std::shared_ptr<Session> GetSession(int uid);
    void AddSession(int uid, std::shared_ptr<Session> session);
    void RemoveSession(int uid, std::string session_id);
private:
    UserManager() = default;

private:
    std::mutex _session_mutex;
    std::unordered_map<int, std::shared_ptr<Session>> _sessions;
};

