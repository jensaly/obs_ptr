#pragma once

class IMemObserver {
public:
    virtual void handle_notification() = 0;

};