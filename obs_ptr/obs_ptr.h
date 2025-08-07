#pragma once
#include "IObserved.h"
#include "IObserver.h"

template <typename T>
class Observer;

template <typename T>
using obs_ptr = std::shared_ptr<Observer<T>>;

template <typename T>
obs_ptr<T> make_observer(std::shared_ptr<T> observed);

template <class Observed>
class Observer : public IObserver, public std::weak_ptr <
{
private:
    std::weak_ptr<IObserved> m_observed;
    // Removes observer from the IObserved object
    inline void remove_observer()
    {
        auto pObserved = m_observed.lock();
        if (pObserved != nullptr)
        {
            pObserved->remove_observer(this);
        }
    }

    // Adds observer from the IObserved object
    // We pass the new m_observed object even though it is already set to the
    // m_observed-variable, so we are explicit.
    inline void add_observer(std::shared_ptr<IObserved> object)
    {
        if (object != nullptr)
        {
            object->add_observer(std::enable_shared_from_this<Observer<Observed>>::shared_from_this());
        }
    }

    // Mechanism which resets the pointer on destruction of target.
    // Called by notify_all() in IObserved, which is called on destruction.
    void handle_notification(IObserved *observed) override
    {
        m_observed.reset();
    }

protected:
public:
    // Constructors
    // Initialized with an observee, no callback (pointer owner does not need notification)
    explicit Observer(std::shared_ptr<Observed> observed) : m_observed(observed)
    {
    }
    // Move constructor
    Observer(Observer<Observed> &&value) : Observer(value.m_observed)
    {
    }
    // Default construction with no initial observee.
    Observer()
    {
        // weak_ptr, no logic
    }
    // Copy constructor
    Observer(Observer<Observed> &other)
    {
        m_observed = other.m_observed;
        add_observer(other.m_observed);
    }

    // Destructor
    ~Observer()
    {
        remove_observer();
    }

    // Assignment operators
    Observer<Observed> &operator=(std::shared_ptr<Observed> pOther)
    {
        remove_observer();
        m_observed = pOther;
        add_observer(pOther);
        // Owner is unchanged, so callback is unchanged
        return *this;
    }

    // Comparison operators
    bool operator==(const Observer<Observed> &pOther) const
    {
        return pOther.m_observed == m_observed;
    }
    bool operator==(const std::shared_ptr<Observed> &pOther) const
    {
        return pOther == m_observed.lock();
    }

    /*
    bool operator!=(const Observer<T> &om_observed) const
    {
        return o_observed.m_observed != m_observed;
    }
    bool operator!=(const T *om_observed) const
    {
        return o_observed != m_observed;
    }
        */

    Observed *get_raw() { return (m_observed.expired() ? nullptr : m_observed.lock().get()); }

    // Wrapped weak_ptr members

    bool expired() const noexcept
    {
        return m_observed.expired();
    }

    std::shared_ptr<Observed> lock() const noexcept
    {
        return m_observed.lock();
    }

    void set(std::shared_ptr<Observed> observed)
    {
        if (observed == m_observed.lock())
        {
            return;
        }
        remove_observer();
        m_observed = observed;
        add_observer(observed);
    }

    void unset()
    {
        set(nullptr);
    }

    friend obs_ptr<Observed> make_observer<Observed>(std::shared_ptr<Observed> observed);
};

template <typename T>
obs_ptr<T> make_observer(std::shared_ptr<T> observed)
{
    auto pObserver = std::make_shared<Observer<T>>(observed);
    pObserver->add_observer(observed);
    return pObserver;
}

template <typename T>
obs_ptr<T> make_empty_observer()
{
    return std::make_shared<Observer<T>>();
}
