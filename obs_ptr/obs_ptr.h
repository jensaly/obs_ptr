#pragma once 
#include "memobserver.h"
#include "memobservee.h"

template <class T>
struct obs_ptr : public IMemObserver {
private:
    T* _observee = nullptr;
    // Removes observer from the IMemObservee object
    inline void remove_observer() {
        if (_observee != nullptr)
            _observee->remove_observer(this);
    }

    // Adds observer from the IMemObservee object
    // We pass the new_observee object even though it is already set to the
    // _observee-variable, so we are explicit.
    inline void add_observer(IMemObservee* new_observee) {
        if (new_observee != nullptr)
            new_observee->add_observer(this);
    }

    // Mechanism which resets the pointer on destruction of target.
    // Called by notify_all() in IMemObservee, which is called on destruction.
    void handle_notification() override {
        _observee = nullptr;
    }
public:
    // Constructors
    // Initialized with an observee, no callback (pointer owner does not need notification)
    explicit obs_ptr(T* observee) : _observee(observee) {
        observee->add_observer(this);
    }
    // Move constructor
    obs_ptr(obs_ptr<T>&& value) : obs_ptr(value._observee) {

    }
    // Default construction with no initial observee.
    obs_ptr() {
        _observee = nullptr;
    }
    // Copy constructor
    obs_ptr(obs_ptr<T>& other) {
        _observee = other._observee;
        add_observer(other._observee);
    }

    // Destructor
    ~obs_ptr() {
        remove_observer();
    }

    // Dereference operators
    T& operator *() { return *_observee; };
    T* operator ->() { return _observee; };

    // Assignment operators
    obs_ptr<T>& operator=(T* new_observee) {
        remove_observer();
        _observee = new_observee;
        add_observer(new_observee);
        // Owner is unchanged, so callback is unchanged
        return *this;
    }
    obs_ptr<T>& operator=(const obs_ptr<T>& new_observee) {
        if (&new_observee == this) return *this;
        remove_observer();
        _observee = new_observee._observee;
        add_observer(_observee);
        // Owner is unchanged, so callback is unchanged
        return *this;
    }

    // Conversion operators
    operator T*() const { return _observee; }

    // Comparison operators
    bool operator==(const obs_ptr<T>& o_observee) const {
        return o_observee._observee == _observee;
    }
    bool operator==(T* o_observee) const {
        return o_observee == _observee;
    }
    bool operator!=(const obs_ptr<T>& o_observee) const {
        return o_observee._observee != _observee;
    }
    bool operator!=(const T* o_observee) const {
        return o_observee != _observee;
    }

    T* get() { return _observee; }
};