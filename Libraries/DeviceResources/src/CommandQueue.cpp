#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/CommandQueue.h>

#include <Buma3DHelpers/B3DInit.h>
#include <Buma3DHelpers/Buma3DHelpers.h>

#include <Utils/Logger.h>

namespace buma
{

CommandQueue::CommandQueue(DeviceResources& _dr, buma3d::util::Ptr<buma3d::ICommandQueue> _que)
    : mutex           {}
    , dr              { _dr }
    , command_type    { _que->GetDesc().type }
    , command_queue   { _que }
    , execution_fence {}
    , fence_value     {}
    , chain           {}
{
    auto bmr = dr.GetDevice()->CreateFence(buma3d::init::TimelineFenceDesc(), &execution_fence);
    BMR_ASSERT(bmr);
    execution_fence->SetName("CommandQueue::execution_fence");
}

CommandQueue::~CommandQueue()
{
    WaitIdle();
}

uint64_t CommandQueue::Submit(util::SubmitDesc& _custom_submission)
{
    std::lock_guard lock(mutex);

    _custom_submission.AddNewSubmitInfo().AddSignalFence(execution_fence.Get(), fence_value++);

    auto bmr = command_queue->Submit(_custom_submission.Finalize().Get());
    BMR_ASSERT(bmr);

    return fence_value;
}

void CommandQueue::SubmitWait(buma3d::IFence* _fence, uint64_t _value)
{
    command_queue->SubmitWait({ buma3d::FENCE_SUBMISSION{ 1, &_fence, &_value } });
}

void CommandQueue::SubmitSignal(buma3d::IFence* _fence, uint64_t _value, buma3d::IFence* _fence_to_cpu)
{
    command_queue->SubmitSignal({ buma3d::FENCE_SUBMISSION{ 1, &_fence, &_value }, _fence_to_cpu });
}

void CommandQueue::WaitValueComplete(uint64_t _fence_value)
{
    auto bmr = execution_fence->Wait(_fence_value, UINT32_MAX);
    BMR_ASSERT(bmr);
}

uint64_t CommandQueue::SubmitFromDr()
{
    std::lock_guard lock(mutex);

    auto&& submit_desc = chain.Finalize().GetSubmitDesc();

    submit_desc.AddNewSubmitInfo().AddSignalFence(execution_fence.Get(), fence_value + 1).Finalize();

    auto bmr = command_queue->Submit(submit_desc.Finalize().Get());
    BMR_ASSERT(bmr);

    chain.Reset();
    fence_value++;
    return fence_value;
}

void CommandQueue::WaitIdle()
{
    auto bmr = command_queue->WaitIdle();
    BMR_ASSERT(bmr);
}


} // namespace buma
