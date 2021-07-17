#pragma once

#include <memory>

namespace buma
{

struct IEventArgs
{
    virtual ~IEventArgs() {}
};

template<typename EventT>
using TEventPtr = std::shared_ptr<EventT>;

template<typename EventT>
using TEventWPtr = std::weak_ptr<EventT>;

struct IEvent : std::enable_shared_from_this<IEvent>
{
    virtual ~IEvent() {}
    virtual void Execute(IEventArgs* _args) = 0;
    void operator()(IEventArgs* _args) { Execute(_args); }

    template<typename EventT, typename... Types>
    static TEventPtr<EventT> Create(Types&&... _args) { return std::make_shared<EventT>(std::forward<Types>(_args)...); }

};
using EventPtr = std::shared_ptr<IEvent>;
using EventWPtr = std::weak_ptr<IEvent>;

struct IDelegate
{
    virtual ~IDelegate() {}

    static std::shared_ptr<IDelegate> CreateDefaultDelegate();

    virtual void RegisterEvent(const EventWPtr& _event) = 0;
    virtual void RemoveEvent(const EventWPtr& _event) = 0;
    virtual void ExecuteEvents(IEventArgs* _args) = 0;

    void RegisterEvent(const EventPtr& _event) { RegisterEvent(_event->weak_from_this()); }
    void RemoveEvent(const EventPtr& _event) { RemoveEvent(_event->weak_from_this()); }

};

struct IDelegateUnsafe
{
    virtual ~IDelegateUnsafe() {}

    static std::shared_ptr<IDelegateUnsafe> CreateDefaultDelegate();

    virtual void RegisterEvent(IEvent* _event) = 0;
    virtual void RemoveEvent(IEvent* _event) = 0; // イベントは破棄される前にRemoveEvent()を呼び出す必要があります。 
    virtual void ExecuteEvents(IEventArgs* _args) = 0;

};

template<typename EventArgsT = IEventArgs, std::enable_if_t<std::is_base_of_v<IEventArgs, EventArgsT>, int> = 0>
class LazyDelegate
{
public:
    using EventArgsType = EventArgsT;

public:
    LazyDelegate() : dg{}, pdg{} { dg = IDelegate::CreateDefaultDelegate(); pdg = dg.get(); }
    ~LazyDelegate() { dg.reset(); pdg = nullptr; }

    void RegisterEvent(IEvent*          _event) { pdg->RegisterEvent(_event->weak_from_this()); }
    void RegisterEvent(const EventWPtr& _event) { pdg->RegisterEvent(_event); }
    void RegisterEvent(const EventPtr&  _event) { pdg->RegisterEvent(_event); }
    void operator+=   (IEvent*          _event) { pdg->RegisterEvent(_event->weak_from_this()); }
    void operator+=   (const EventWPtr& _event) { pdg->RegisterEvent(_event); }
    void operator+=   (const EventPtr&  _event) { pdg->RegisterEvent(_event); }

    void RemoveEvent(IEvent*            _event) { pdg->RemoveEvent(_event->weak_from_this()); }
    void RemoveEvent(const EventWPtr&   _event) { pdg->RemoveEvent(_event); }
    void RemoveEvent(const EventPtr&    _event) { pdg->RemoveEvent(_event); }
    void operator-= (IEvent*            _event) { pdg->RemoveEvent(_event->weak_from_this()); }
    void operator-= (const EventWPtr&   _event) { pdg->RemoveEvent(_event); }
    void operator-= (const EventPtr&    _event) { pdg->RemoveEvent(_event); }

    void ExecuteEvents(EventArgsT*      _args)  { pdg->ExecuteEvents(_args); }
    void operator()(EventArgsT*         _args)  { pdg->ExecuteEvents(_args); }

private:
    std::shared_ptr<IDelegate> dg;
    IDelegate* pdg;

};


template<typename EventArgsT = IEventArgs, std::enable_if_t<std::is_base_of_v<IEventArgs, EventArgsT>, int> = 0>
class LazyDelegateUnsafe
{
public:
    using EventArgsType = EventArgsT;

public:
    LazyDelegateUnsafe() : dg{}, pdg{} { dg = IDelegateUnsafe::CreateDefaultDelegate(); pdg = dg.get(); }
    ~LazyDelegateUnsafe() { dg.reset(); pdg = nullptr; }

    void RegisterEvent(IEvent*          _event) { pdg->RegisterEvent(_event); }
    void operator+=   (IEvent*          _event) { pdg->RegisterEvent(_event); }

    void RemoveEvent(IEvent*            _event) { pdg->RemoveEvent(_event); }
    void operator-= (IEvent*            _event) { pdg->RemoveEvent(_event); }

    void ExecuteEvents(EventArgsT*      _args)  { pdg->ExecuteEvents(_args); }
    void operator()(EventArgsT*         _args)  { pdg->ExecuteEvents(_args); }

private:
    std::shared_ptr<IDelegateUnsafe> dg;
    IDelegateUnsafe* pdg;

};


}// namespace buma
