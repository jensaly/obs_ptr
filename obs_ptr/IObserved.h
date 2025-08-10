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

class IObserver;

class IObserved
{
    std::vector<std::weak_ptr<IObserver>> m_observers; // Cannot have duplicate values
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
            pObserver->handle_notification();
        }
    }
    void add_observer(std::shared_ptr<IObserver> spObserver)
    {
        m_observers.push_back(spObserver);
    }

    void remove_observer(std::shared_ptr<IObserver> spObserver)
    {
        for (auto it = m_observers.begin(); it != m_observers.end();)
        {
            if (auto sp = it->lock())
            {
                if (sp == spObserver)
                {
                    it = m_observers.erase(it);
                    return;
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                // Remove expired weak_ptrs while we're here
                it = m_observers.erase(it);
            }
        }
    }

    void remove_destructed_observer()
    {
        auto isExpired = [](const std::weak_ptr<IObserver> &wp)
        {
            return wp.expired();
        };

        auto expiredCount = std::count_if(m_observers.begin(), m_observers.end(), isExpired);
        if (expiredCount > 1)
        {
            throw std::runtime_error("More than one expired weak_ptr found");
        }

        m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(), isExpired), m_observers.end());
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

    // https://stackoverflow.com/questions/12301916/how-can-you-efficiently-check-whether-two-stdweak-ptr-pointers-are-pointing-to
    bool IsObserver(const std::shared_ptr<IObserver> pObserver) const
    {
        std::weak_ptr<IObserver> wpOtherObserver = pObserver;
        return std::find_if(m_observers.begin(), m_observers.end(), [&wpOtherObserver](const std::weak_ptr<IObserver> &wpObserver)
                            { return !wpOtherObserver.owner_before(wpObserver) && !wpObserver.owner_before(wpOtherObserver); }) != m_observers.end();
    }

    template <class T>
    friend class obs_ptr;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(m_observers);
    }
};