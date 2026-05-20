//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_MSGTYPE_H
#define ATTA1_MSGTYPE_H

enum class MsgType : unsigned {
    Ping,
    Pong,
    Auth,
    Error
};

#endif //ATTA1_MSGTYPE_H