//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_DISPATCHER_H
#define ATTA1_DISPATCHER_H
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <unordered_map>

#include "../protocol/Reader.h"
#include "../protocol/MsgType.h"
#include "handlers/LogicHandler.h"
#include "../logger/Logger.h"
class Session;

class MessageDispatcher {
    using HandlerFunc = std::function<void(std::shared_ptr<Session>, Reader&)>;
    std::unordered_map<uint16_t, HandlerFunc> handlers_;

public:
    MessageDispatcher() {
        handlers_[static_cast<uint16_t>(MsgType::Auth)] = LogicHandler::handleAuth;
        // handlers_[static_cast<uint16_t>(MsgType::Move)] = LogicHandler::handleMove;
    }

    void dispatch(std::shared_ptr<Session> session, uint16_t type, std::span<const std::byte> body) {
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            Reader rd(body);
            it->second(session, rd);
        } else {
            Logger::warn("No handler for msg type: {}", type);
        }
    }
};

#endif //ATTA1_DISPATCHER_H