#pragma once

extern "C" {
    // Log level enum that will be accessible to modules
    enum LoggingLevels {
        FATAL = 0,
        ERROR = 1,
        WARN  = 2,
        INFO  = 3,
        DEBUG = 4
    };
}