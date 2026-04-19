#include<ppe.hpp>
#include<cstdio>
#include<cstdlib>
#include<time.h>

namespace ppe {

    void getResourcePath(char* dir, const char* fileName) {
        #ifdef DEBUG
            std::sprintf(dir, "./resources/%s", fileName);
        #else
            std::sprintf(dir, "%s/.local/share/planetan/%s", getenv("HOME"), fileName);
        #endif
    }

    const char* errorString(int err) {
        switch(err) {
            case SUCCESS:
                return "success";
            case ERROR_OPEN_FILE:
                return "failed to open file";
            case ERROR_INVALID_FORMAT:
                return "invalid format";
            case ERROR_UNKNOWN_EVENT:
                return "unknown event";
            case ERROR_INVALID_VALUE:
                return "invalid value";
            case ERROR_UNKNOWN_ENUM:
                return "unknown enum key";
        }
        return "unknown error";
    }

    int parseTime(const char* str, double* time) {
        int hour = 0, min = 0;
        double sec = 0;
        if(sscanf(str, "%d:%d:%lf", &hour, &min, &sec) < 3) return 0;
        else {
            *time = hour * 3600 + min * 60 + sec;
            return 1;
        }
    }

    int parseDateTime(const char* str, double* dateTime) {
        struct tm time19;
        if(!strptime(str, "%Y-%m-%d %H:%M:%s %Z", &time19)) return 0;
        *dateTime = std::difftime(mktime(&time19), TIME_EPOCH) / (24.0 * 3600.0);
        return 1;
    }

    int parseChangeType(const char* str, int* changeType) {
        if(!std::strcmp(str, "linear")) *changeType = CHANGE_LINEAR;
        else if(!std::strcmp(str, "smoother")) *changeType = CHANGE_SMOOTHER;
        else return 0;
        return 1;
    }

    double getLevel(double x, int changeType) {
        if(x < 0) return 0;
        else if(x > 1) return 1;
        else switch(changeType) {
            case CHANGE_SMOOTHER:
                return 6.0 * std::pow(x, 5) - 15.0 * std::pow(x, 4) + 10.0 * std::pow(x, 3);

            default:
                return x;
        }
    }

}