#pragma once

class IObserved;

class IObserver
{
public:
    virtual void handle_notification(IObserved *source) = 0;
};