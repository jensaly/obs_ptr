#pragma once

// ========================
// Standard Library Includes
// ========================
#include <set>
#include <algorithm>
#include <memory>
#include <cassert>

// ========================
// Third-Party Library Includes
// ========================

// ========================
// Local Project Includes
// ========================
#include "IObserver.h"

// ========================
// Namespace Usings
// ========================

// ========================
// Forward Declarations
// ========================
template <typename T>
class Observer;

class IObserved;

inline void RemoveSingleInstancePointerInContainer(
    std::set<std::weak_ptr<IObserver>, std::owner_less<std::weak_ptr<IObserver>>> &v,
    std::weak_ptr<IObserver> t)
{
    auto p = v.find(t); // uses owner_less to compare
    if (p != v.end())
    {
        v.erase(p);
    }
}

inline bool IsInstanceInContainer(
    std::set<std::weak_ptr<IObserver>, std::owner_less<std::weak_ptr<IObserver>>> const &v,
    std::weak_ptr<IObserver> const t)
{
    return v.find(t) != v.end();
}

class IObserver;

class IObserved
{
    std::set<std::weak_ptr<IObserver>, std::owner_less<std::weak_ptr<IObserver>>> m_observers; // Cannot have duplicate values
    void notify_all()
    {
        // All_observers is unchanging, containing all the observers at the start of destruction
        // Since some MemObservers are owned by other objects (e.g. vectors), the handling of one notification may impact others
        // We therefore compare if the pointer in all_observers still exists in observers before sending another notification.
        // This avoids sending notifications to destroyed objects, i.e. segfaults.
        auto all_observers = m_observers;
        for (auto wpObserver : all_observers)
        {
            auto pObserver = wpObserver.lock();
            if (pObserver == nullptr)
            {
                // Should never happen under any circumstances. Observer was deallocated without notifying observed.
                assert(false);
            }
            pObserver->handle_notification(this);
        }
    }
    void add_observer(std::weak_ptr<IObserver> wpObserver)
    {
        if (IsInstanceInContainer(m_observers, wpObserver))
        {
            m_observers.insert(wpObserver);
        }
    }
    void remove_observer(std::shared_ptr<IObserver> pObserver)
    {
        std::weak_ptr<IObserver> wpObserver = pObserver;
        RemoveSingleInstancePointerInContainer(m_observers, wpObserver);
    }

protected:
    virtual ~IObserved()
    {
        notify_all();
        m_observers.clear();
    }
    // Protected so derived classes have access to an automatically instantiate the set.
    IObserved() = default;

public:
    size_t Observers() const
    {
        return m_observers.size();
    }
    bool IsObserver(std::shared_ptr<IObserver> pObserver) const
    {
        std::weak_ptr<IObserver> wpObserver = pObserver;
        return IsInstanceInContainer(m_observers, wpObserver);
    }

    template <typename T>
    friend class Observer;
};