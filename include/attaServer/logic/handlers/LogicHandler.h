//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_LOGICHANDLER_H
#define ATTA1_LOGICHANDLER_H

class LogicHandler {
public:
    static void handleAuth(std::shared_ptr<Session> session, Reader& rd) {
        auto login = rd.readString();
        auto pass = rd.readString();

        Logger::info("User {} logged in", login);
        session->send(Protocol::make_packet(Protocol::MsgType::AuthOk));
    }
};

#endif //ATTA1_LOGICHANDLER_H