#pragma once

#include <functional>
#include <map>

namespace buma
{

class Disposer
{
public:
    using Key = const void*;

    // Adds the given resource to the disposer and sets its reference count to 1.
    void CreateDisposable(Key _resource, std::function<void()> _destructor) noexcept;

    // Decrements the reference count.
    void RemoveReference(Key _resource) noexcept;

    // Increments the reference count and auto-decrements it after FRAMES_BEFORE_EVICTION frames.
    // This is used to indicate that the current command buffer has a reference to the resource.
    void Acquire(Key _resource) noexcept;

    // Invokes the destructor function for each disposable with a 0 refcount.
    void GC() noexcept;

    // Invokes the destructor function for all disposables, regardless of reference count.
    void Reset() noexcept;

private:
    struct Disposable
    {
        uint16_t ref_count = 1;
        uint16_t remaining_frames = 0;
        std::function<void()> destructor = []() {};
    };
    std::map<Key, Disposable> disposables;


};


} // namespace buma
