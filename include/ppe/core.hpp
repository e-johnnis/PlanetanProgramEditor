#ifndef __PPE_CORE_HPP
#define __PPE_CORE_HPP 0

#ifdef DEBUG
    #define PPE_VERSION_STR "*** Planetan: Program Editor DEBUG ***"
#else
    #define PPE_VERSION_STR "*** Planetan: Program Editor v0.0.0.0 ***"
#endif

#define PPE_CHAR_MAX 128

// unix time of 2000-01-01 00:00:00 +0000
#define TIME_EPOCH 946684800

namespace ppe {

    enum ErrorCode {
        SUCCESS = 0,
        ERROR_OPEN_FILE,
        ERROR_INVALID_FORMAT,
        ERROR_UNKNOWN_EVENT,
        ERROR_INVALID_VALUE,
        ERROR_UNKNOWN_ENUM
    };

    enum ValueChangeType {
        CHANGE_LINEAR = 0,
        CHANGE_SMOOTHER,
        CHANGE_DOUBLE,
        CHANGE_QUAD,
        CHANGE_OCT
    };

    void getResourcePath(char*, const char*);
    const char* errorString(int);
    int parseTime(const char*, double*);
    int parseDateTime(const char*, double*);
    int parseChangeType(const char*, int*);
    double getLevel(double, int);
}

#endif