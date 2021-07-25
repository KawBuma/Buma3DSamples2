#pragma once
#include <DeviceResources/CommandListChain.h>

#include <Utils/NonCopyable.h>

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <mutex>

namespace buma
{

class DeviceResources;

class CommandQueue : public util::NonCopyable
{
public:
    CommandQueue(DeviceResources& _dr, buma3d::util::Ptr<buma3d::ICommandQueue> _que);
    ~CommandQueue();

    buma3d::COMMAND_TYPE   GetCommandType()  const { return command_type; }
    buma3d::ICommandQueue* GetCommandQueue() const { return command_queue.Get(); }

    buma3d::IFence*        GetFence()      const { return execution_fence.Get(); }
    uint64_t               GetFenceValue() const { return fence_value; }

    // 送信するコマンドの追加

    void PrependCommandList(buma3d::ICommandList* _list)                       { chain.PrependCommandList(_list); }
    void PrependSignalFence(buma3d::IFence* _fence, uint64_t _fence_value = 0) { chain.PrependSignalFence(_fence, _fence_value); }
    void PrependWaitFence  (buma3d::IFence* _fence, uint64_t _fence_value = 0) { chain.PrependWaitFence(_fence, _fence_value); }
    void PrependSubmitInfo (const buma3d::SUBMIT_INFO& _info)                  { chain.PrependSubmitInfo(_info); }

    void AddCommandList(uint64_t _order, buma3d::ICommandList* _list)                       { chain.AddCommandList(_order, _list); }
    void AddSignalFence(uint64_t _order, buma3d::IFence* _fence, uint64_t _fence_value = 0) { chain.AddSignalFence(_order, _fence, _fence_value); }
    void AddWaitFence  (uint64_t _order, buma3d::IFence* _fence, uint64_t _fence_value = 0) { chain.AddWaitFence(_order, _fence, _fence_value); }
    void AddSubmitInfo (uint64_t _order, const buma3d::SUBMIT_INFO& _info)                  { chain.AddSubmitInfo(_order, _info); }

    bool HasPrependedCommand()             const { return chain.HasPrependedCommand(); }
    bool HasCommand(uint64_t _in_order)    const { return chain.HasCommand(_in_order); }

    bool HasPrependedSubmission()          const { return chain.HasPrependedSubmission(); }
    bool HasSubmission(uint64_t _in_order) const { return chain.HasSubmission(_in_order); }

    // 送信

    // ユーザー独自のSubmitDescを送信します。 _custom_submission に execution_fence のシグナルが追加されます。
    uint64_t Submit(util::SubmitDesc& _custom_submission);

    // 指定のフェンスを待機します。 execution_fence のシグナルは送信されません。
    void SubmitWait(buma3d::IFence* _fence, uint64_t _value);

    // 指定のフェンスをシグナルします。 execution_fence のシグナルは送信されません。
    void SubmitSignal(buma3d::IFence* _fence, uint64_t _value, buma3d::IFence* _fence_to_cpu = nullptr);

    void WaitValueComplete(uint64_t _fence_value);
    void WaitIdle();

public:
    // DeviceResourcesから呼び出される事を予期します。 
    uint64_t SubmitFromDr();

private:
    //mutable std::mutex                          mutex;
    mutable dummy_mutex                         mutex;
    DeviceResources&                            dr;
    buma3d::COMMAND_TYPE                        command_type;
    buma3d::util::Ptr<buma3d::ICommandQueue>    command_queue;
    buma3d::util::Ptr<buma3d::IFence>           execution_fence; // 送信したコマンドが完了したかどうかを通知するフェンスです。 
    uint64_t                                    fence_value; 
    buma::CommandListChain                      chain;

};


} // namespace buma
