#pragma once
#include "IObserved.h"
#include "IObserver.h"

#include <memory>

template <class T>
class obs_ptr : public IObserver, public std::enable_shared_from_this<obs_ptr<T>>
{
public:
    obs_ptr()
    {
        // Does nothing
    }

    ~obs_ptr()
    {
        remove_on_destruction();
    }

    obs_ptr(const obs_ptr<T> &pOther) = delete;

    obs_ptr(obs_ptr<T> &&other) noexcept = delete;

    bool operator!=(std::nullptr_t) const noexcept
    {
        return m_wpObserved.lock() != nullptr;
    }

    bool operator!=(std::shared_ptr<T> sp) const noexcept
    {
        return m_wpObserved.lock() != sp;
    }

    bool operator!=(const obs_ptr<T> &other) const noexcept
    {
        return m_wpObserved.lock() != other.m_wpObserved.lock();
    }

    bool operator==(std::nullptr_t) const noexcept
    {
        return m_wpObserved.lock() == nullptr;
    }

    bool operator==(std::shared_ptr<T> sp) const noexcept
    {
        return m_wpObserved.lock() == sp;
    }

    bool operator==(const obs_ptr<T> &other) const noexcept
    {
        return m_wpObserved.lock() == other.m_wpObserved.lock();
    }

    void set(const std::shared_ptr<T> &pOther)
    {
        add_observer(pOther);
    }

    void unset()
    {
        remove_observer();
    }

    bool is_set() const
    {
        return !m_wpObserved.expired() && m_wpObserved.lock() != nullptr;
    }

    obs_ptr<T> &get_obs()
    {
        return *this;
    }

    template <class U>
    friend std::shared_ptr<obs_ptr<U>> make_observer(std::shared_ptr<U> spObserved);

    template <class U>
    friend std::shared_ptr<obs_ptr<U>> make_observer(std::shared_ptr<obs_ptr<U>> spObserver);

    template <class U>
    friend std::shared_ptr<obs_ptr<U>> move_observer(std::shared_ptr<obs_ptr<U>> spObserver);

protected:
    void handle_notification() override
    {
        m_wpObserved.reset();
    }

private:
    void add_observer(std::shared_ptr<T> spNewObserved)
    {
        if (spNewObserved == m_wpObserved.lock())
        {
            // Do not set to observe same object
            return;
        }
        remove_observer();
        if (spNewObserved == nullptr)
        {
            // Nothing more if set to nullptr
            return;
        }
        auto spThis = this->shared_from_this();
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
        m_wpObserved.reset();
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

    std::weak_ptr<T> m_wpObserved;
};

// nullptr on lhs
template <class T>
inline bool operator==(std::nullptr_t, const obs_ptr<T> &rhs) noexcept
{
    return rhs == nullptr;
}

template <class T>
inline bool operator!=(std::nullptr_t, const obs_ptr<T> &rhs) noexcept
{
    return rhs != nullptr;
}

// shared_ptr<T> on lhs
template <class T>
inline bool operator==(const std::shared_ptr<T> &lhs, const obs_ptr<T> &rhs) noexcept
{
    return rhs == lhs;
}

template <class T>
inline bool operator!=(const std::shared_ptr<T> &lhs, const obs_ptr<T> &rhs) noexcept
{
    return rhs != lhs;
}

template <class T>
std::shared_ptr<obs_ptr<T>> make_observer(std::shared_ptr<T> spObserved = nullptr)
{
    auto pObserver = std::make_shared<obs_ptr<T>>();
    pObserver->add_observer(spObserved);
    return std::move(pObserver);
}

template <class T>
std::shared_ptr<obs_ptr<T>> make_observer(std::shared_ptr<obs_ptr<T>> spObserver)
{
    auto pObserver = make_observer(spObserver->m_wpObserved.lock());
    return std::move(pObserver);
}

template <class T>
std::shared_ptr<obs_ptr<T>> move_observer(std::shared_ptr<obs_ptr<T>> spObserver)
{
    auto pObserver = make_observer(spObserver->m_wpObserved.lock());
    spObserver->unset();
    return std::move(pObserver);
}

template <class Archive, class T>
void save(Archive &archive,
          obs_ptr<T> const &m)
{
    archive(m.x, m.y, m.z);
}

template <class Archive>
void load(Archive &archive,
          obs_ptr<T> &m)
{
    archive(m.x, m.y, m.z);
}