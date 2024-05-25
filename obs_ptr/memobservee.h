#pragma once
#include <set>
#include <algorithm>
#include "memobserver.h"
#include "obs_ptr.h"

template<typename T> inline void RemoveSingleInstancePointerInContainer(std::set<T*>& v, T* t) {
    auto p = std::find(v.begin(), v.end(), t);
    (p != v.end()) ? v.erase(p) : p;
}

template<typename T> inline bool IsInstanceInContainer(std::set<T*>& v, T* t) {
    return std::find(v.begin(), v.end(), t) != v.end();
}

class IMemObserver;

class IMemObservee {
    template<typename T> friend class obs_ptr;
    std::set<IMemObserver*> observers; // Cannot have duplicate values
    void notify_all() {
        // All_observers is unchanging, containing all the observers at the start of destruction
        // Since some MemObservers are owned by other objects (e.g. vectors), the handling of one notification may impact others
        // We therefore compare if the pointer in all_observers still exists in observers before sending another notification.
        // This avoids sending notifications to destroyed objects, i.e. segfaults.
        auto all_observers = observers; 
        for (auto o : all_observers) {
            if (observers.find(o) != observers.end()) {
                o->handle_notification();
            }
        }
    }
    void add_observer(IMemObserver* observer) {
        observers.insert(observer);
    }
    void remove_observer(IMemObserver* observer) {
        RemoveSingleInstancePointerInContainer(observers, observer);
    }
protected:
    virtual ~IMemObservee() {
        notify_all();
        observers.clear();
    }
    // Protected so derived classes have access to an automatically instantiate the set.
    IMemObservee() = default;
public:
    size_t totalObservers() {
        return observers.size();
    }
    bool isObserver(IMemObserver* observer) {
        return IsInstanceInContainer(observers, observer);
    }
    bool isObserver(IMemObserver& observer) {
        return IsInstanceInContainer(observers, &observer);
    }
};