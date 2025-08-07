#pragma once

class IObserved;

class IObserver
{
public:
    friend class IObserved;

protected:
    virtual void handle_notification() = 0;
};