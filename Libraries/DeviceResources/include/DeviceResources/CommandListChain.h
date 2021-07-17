#pragma once

#include <Buma3D/Buma3D.h>

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <mutex>
#include <map>
#include <vector>
#include <deque>
#include <tuple>

namespace buma
{

class CommandListChain
{
    using FenceInfo = std::tuple<buma3d::IFence*, uint64_t>;
    template<template <typename T, typename Al = std::allocator<T>> class Container = std::vector>
    struct SUBMIT
    {
        Container<FenceInfo>              wait_fences;
        Container<FenceInfo>              signal_fences;
        Container<buma3d::ICommandList*>  command_lists;
    };

public:
    CommandListChain()
        : mutex             {}
        , prepend_submits   {}
        , submits           {}
        , submit_desc       {}
    {}
    CommandListChain(const CommandListChain& _c)
        : mutex             {}
        , prepend_submits   { _c.prepend_submits }
        , submits           { _c.submits }
        , submit_desc       { _c.submit_desc }
    {}
    CommandListChain& operator=(const CommandListChain& _c)
    {
        prepend_submits = _c.prepend_submits;
        submits         = _c.submits;
        submit_desc     = _c.submit_desc;
    }

    ~CommandListChain() {}

    void Reset()
    {
        prepend_submits.command_lists.clear();
        prepend_submits.signal_fences.clear();
        prepend_submits.wait_fences.clear();
        for (auto& [i_order, i_si] : submits)
        {
            i_si.command_lists.clear();
            i_si.signal_fences.clear();
            i_si.wait_fences.clear();
        }
        submit_desc.Reset();
    }

    void PrependCommandList(buma3d::ICommandList* _list)
    {
        std::lock_guard lock(mutex);
        prepend_submits.command_lists.emplace_front(_list);
    }
    void PrependSignalFence(buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        std::lock_guard lock(mutex);
        prepend_submits.signal_fences.emplace_front(_fence, _fence_value);
    }
    void PrependWaitFence(buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        std::lock_guard lock(mutex);
        prepend_submits.wait_fences.emplace_front(_fence, _fence_value);
    }

    void AddCommandList(uint64_t _order, buma3d::ICommandList* _list)
    {
        std::lock_guard lock(mutex);
        submits[_order].command_lists.emplace_back(_list);
    }
    void AddSignalFence(uint64_t _order, buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        std::lock_guard lock(mutex);
        submits[_order].signal_fences.emplace_back(_fence, _fence_value);
    }
    void AddWaitFence(uint64_t _order, buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        std::lock_guard lock(mutex);
        submits[_order].wait_fences.emplace_back(_fence, _fence_value);
    }

    bool HasPrependedCommand() const
    {
        std::lock_guard lock(mutex);
        return !prepend_submits.command_lists.empty();
    }
    bool HasCommand(uint64_t _in_order) const
    {
        std::lock_guard lock(mutex);
        if (auto found = submits.find(_in_order); found == submits.end())
            return false;
        else
            return !found->second.command_lists.empty();
    }

    bool HasPrependedSubmission() const
    {
        std::lock_guard lock(mutex);
        return !prepend_submits.command_lists.empty() ||
               !prepend_submits.signal_fences.empty() ||
               !prepend_submits.wait_fences.empty();
    }
    bool HasSubmission(uint64_t _in_order) const
    {
        std::lock_guard lock(mutex);
        if (auto found = submits.find(_in_order); found == submits.end())
            return false;
        else
        {
            auto&& s = found->second;
            return !s.command_lists.empty() ||
                   !s.signal_fences.empty() ||
                   !s.wait_fences.empty();
        }
    }

    CommandListChain& Finalize()
    {
        auto AddSubmits = [&](auto&& _submits) {
            auto&& si = submit_desc.AddNewSubmitInfo();
            for (auto& [fence, val] : _submits.wait_fences)   si.AddWaitFence(fence, val);
            for (auto& [fence, val] : _submits.signal_fences) si.AddSignalFence(fence, val);
            for (auto& cmd : _submits.command_lists)          si.AddCommandList(cmd);
            si.Finalize();
        };
        if (HasPrependedSubmission())
            AddSubmits(prepend_submits);
        for (auto& [i_order, i_si] : submits)
            if (HasSubmission(i_order))
                AddSubmits(i_si);
        return *this;
    }
    buma::util::SubmitDesc& GetSubmitDesc()
    {
        return submit_desc;
    }

private:
    mutable std::mutex              mutex;
    SUBMIT<std::deque>              prepend_submits;
    std::map<uint64_t, SUBMIT<>>    submits;
    util::SubmitDesc                submit_desc;

};


}// namespace buma
