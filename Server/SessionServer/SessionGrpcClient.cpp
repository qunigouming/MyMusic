#include "SessionGrpcClient.h"
#include "ConfigManager.h"

KickUserRsp SessionGrpcClient::NotifyKickUser(const std::string& server_ip, const KickUserReq& req)
{
    KickUserRsp rsp;
    Defer defer([&rsp, &req] {
        rsp.set_uid(req.uid());
    });

    auto iter = _pool.find(server_ip);
    if (iter == _pool.end()) {
        rsp.set_error(ErrorCodes::Success);
        return rsp;
    }
    auto& pool = iter->second;
    ClientContext context;
    auto stub = pool->getConnection();
    Defer conDefer([&pool, &stub] {
        pool->returnConnection(std::move(stub));
    });
    Status status = stub->NotifyKickUser(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    rsp.set_error(ErrorCodes::Success);
    return rsp;
}

SessionGrpcClient::SessionGrpcClient()
{
    auto& config = ConfigManager::GetInstance();
    auto server_list = config["PeerServer"]["Servers"];

    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    for (auto& word : words) {
        if (config[word]["Name"].empty()) continue;
        _pool[config[word]["Name"]] = std::make_unique<SessionConPool>(5, config[word]["Host"], config[word]["Port"]);
    }
}
