#pragma once

class IObserved;

class IObserver
{
public:
    friend class IObserved;

    template <class Archive>
    void serialize(Archive &archive)
    {
    }

protected:
    virtual void handle_notification() = 0;
};