#ifndef __PPE_EVENT_HPP
#define __PPE_EVENT_HPP 0

namespace ppe {

    class Event {
    public:
        Event(double, double);
        virtual ~Event() {};

        // delete this pointer if returned 1
        virtual int call(double) = 0;
    protected:
        double _startTime = 0;
        double _endTime = 0;
    };

    template <typename T>
    class ChangeValueEvent : public Event {
    public:
        ChangeValueEvent(double, double, T*, T, int);
        ~ChangeValueEvent() override {};
        int call(double) override;
    protected:
        T* _target = nullptr;
        T _value = 0;
        T _initValue = -1;
        int _changeType = CHANGE_LINEAR;
    };

    template <typename T>
    ChangeValueEvent<T>::ChangeValueEvent(
        double start, double end,
        T* target, T value,
        int changeType
    ) : Event(start, end) {
        _target = target;
        _value = value;
        _changeType = changeType;
    }

    template <typename T>
    int ChangeValueEvent<T>::call(double frameTime) {
        if(frameTime < _startTime) return 0;
        else if(frameTime > _endTime) {
            *_target = _value;
            return 1;
        }else if(_initValue < 0) {
            _initValue = *_target;
            return 0;
        }else {
            T level = (T)getLevel((frameTime - _startTime) / (_endTime - _startTime), _changeType);
            *_target = level * _value + (1.0 - level) * _initValue;
            return 0;
        }
    }

}

#endif