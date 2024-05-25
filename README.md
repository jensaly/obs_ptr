# Memory observer pattern through self-nulling smart pointer

This library introduces a smart pointer, obs_ptr. It is a hybrid owning and non-owning pointer to an object that inherits the IMemObservee-interface.

When the object is deallocated, its the IMemObservee destructor is called, which subsequently unsets all the obs_ptrs to the object.

If used in full, it ensures no dangling pointers are present automatically.

Delete can be called on the obs_ptr, which will delete the underlying object and subsequently notified all other obs_ptrs. In this way, it can serve as both an owning and non-owning pointer, in a flat hierarchy. The object is responsible for setting all pointers to it to null, and any pointer can call delete on it.
