#ifndef TIMEOUT_H
#define TIMEOUT_H
class Timeout {
public:
    Timeout(unsigned long duration) :
        start(millis()), duration(duration) {

    }

    bool elapsed() {
        return millis() - start >= duration;
    }

private:
    unsigned long start, duration;
};

#endif // TIMEOUT_H
