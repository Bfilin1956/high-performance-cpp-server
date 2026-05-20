//
// Created by r13x on 5/6/26.
//

#ifndef ATTA1_LOG_LEVEL_H
#define ATTA1_LOG_LEVEL_H

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,

    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,

    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    error = 4,
    fatal = 5
};

#endif //ATTA1_LOG_LEVEL_H