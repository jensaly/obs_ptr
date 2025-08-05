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

template <typename ContainerType, typename PointerType>
inline void RemoveSingleInstancePointerInContainer(std::set<ContainerType> &v, PointerType t)
{
    auto p = std::find(v.begin(), v.end(), t);
    (p != v.end()) ? v.erase(p) : p;
}

template <typename ContainerType, typename PointerType>
inline bool IsInstanceInContainer(std::set<ContainerType> const &v, PointerType const t)
{
    return std::find(v.begin(), v.end(), t) != v.end();
}

class IObserver;

class IObserved
{
    std::set<std::weak_ptr<IObserver>> m_observers; // Cannot have duplicate values
    void notify_all()
    {
        // All_observers is unchanging, containing all the observers at the start of destruction
        // Since some MemObservers are owned by other objects (e.g. vectors), the handling of one notification may impact others
        // We therefore compare if the pointer in all_observers still exists in observers before sending another notification.
        // This avoids sending notifications to destroyed objects, i.e. segfaults.
        auto all_observers = m_observers;
        for (auto wpObserver : all_observers)
        {
            if (m_observers.find(wpObserver) != m_observers.end())
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
    }
    void add_observer(std::weak_ptr<IObserver> wpObserver)
    {
        if (IsInstanceInContainer(m_observers, wpObserver))
        {
            m_observers.insert(pObserver);
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
        return IsInstanceInContainer(m_observers, pObserver);
    }
};