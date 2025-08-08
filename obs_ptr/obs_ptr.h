#pragma once
#include "IObserved.h"
#include "IObserver.h"

#include <memory>

template <class T>
class obs_ptr : public IObserver, std::enable_shared_from_this<obs_ptr<T>>
{
public:
    /*
    explicit obs_ptr(std::shared_ptr<IObserved> spObserved)
    {
        m_wpObserved = spObserved;
        // Register
    }
    */

    obs_ptr()
    {
        // Does nothing
    }

    ~obs_ptr()
    {
        remove_on_destruction();
    }

    bool operator==(std::nullptr_t) const noexcept
    {
        return m_wpObserved.lock() == nullptr;
    }

    bool operator!=(std::nullptr_t) const noexcept
    {
        return m_wpObserved.lock() != nullptr;
    }

    bool operator==(std::shared_ptr<T> sp) const noexcept
    {
        return m_wpObserved.lock() == sp;
    }

    template <class U>
    friend std::shared_ptr<obs_ptr<U>> make_observer(std::shared_ptr<U> spObserved);

protected:
    void handle_notification() override
    {
        m_wpObserved.reset();
    }

private:
    void add_observer(std::shared_ptr<T> spNewObserved)
    {
        remove_observer();
        if (spNewObserved == nullptr)
        {
            // Nothing more if set to nullptr
            return;
        }
        auto spThis = this->std::enable_shared_from_this<obs_ptr<T>>::shared_from_this();
        spNewObserved->add_observer(spThis);
        m_wpObserved = spNewObserved;
    }

    void remove_observer()
    {
        auto spObserved = m_wpObserved.lock();
        if (spObserved == nullptr)
        {
            return;
        }
        spObserved->remove_observer(this->std::enable_shared_from_this<obs_ptr<T>>::shared_from_this());
    }

    // Destructor-safe
    void remove_on_destruction()
    {
        auto pObserved = m_wpObserved.lock();
        if (pObserved == nullptr)
        {
            return;
        }
        // Only remove expired pointers, does not forward a shared_from_this
        pObserved->remove_destructed_observer();
    }

    std::weak_ptr<IObserved> m_wpObserved;
};

template <class T>
std::shared_ptr<obs_ptr<T>> make_observer(std::shared_ptr<T> spObserved = nullptr)
{
    std::shared_ptr<obs_ptr<T>> pObserver;
    pObserver->add_observer(spObserved);
    return std::move(pObserver);
}