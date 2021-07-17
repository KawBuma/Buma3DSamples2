#include "./VariableSizeAllocationsManager.h"

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <Utils/Definitions.h>

namespace buma
{

VariableSizeAllocationsManager::VariableSizeAllocationsManager(SizeT _page_size, SizeT _min_alignment)
    : page_size             { _page_size }
    , min_alignment         { _min_alignment }
    , free_size             { _page_size }
    , capable_alignment     {}
    , max_block_size        {}
    , free_blocks_by_offset {}
    , free_blocks_by_size   {}
{
    BUMA_ASSERT(util::IsPowOfTwo(min_alignment));
    Reset();
}

VariableSizeAllocationsManager::~VariableSizeAllocationsManager()
{
    assert(page_size == free_size && "VariableSizeAllocationsManager: memory leaked");
}

void VariableSizeAllocationsManager::Reset()
{
    ResetCapableAlignment();
    free_size = page_size;
    free_blocks_by_offset.clear();
    free_blocks_by_size.clear();
    max_block_size = page_size;
    AddNewBlock(0, page_size);
}

VariableSizeAllocationsManager::ALLOCATION
VariableSizeAllocationsManager::Allocate(SizeT _size, SizeT _alignment)
{
    BUMA_ASSERT(util::IsPowOfTwo(_alignment));

    _alignment = std::max(_alignment, min_alignment);
    auto aligned_size = util::AlignUp(_size, _alignment);

    if (!CheckAllocatable(aligned_size))
        return ALLOCATION{};

    // VariableSizeAllocationsManagerの記事にはありませんが、実際には、アライメントのために追加の実装が行われています。
    // https://github.com/DiligentGraphics/DiligentCore/blob/master/Graphics/GraphicsAccessories/interface/VariableSizeAllocationsManager.hpp#L188

    // |block_offset
    //        *alignment
    // |------|                      capable_alignment
    // ooooooo*--------------------* _alignment
    // |-------------|               alignment_reserve (_alignment - capable_alignment)
    // |--------------aligned_size---------------|
    // |------it_size_offset_capable_block-------|
    // 
    // block_offset が要求アライメントで整列されていない場合: 
    // |------|------|------|------|------|------|------|
    // ooooooo*--------------------*--------------------*
    // ooooooo|--------------aligned_size---------------|
    // |------it_size_offset_capable_block-------|xxxxxxx <-アライメントによってサイズ不足に
    //
    // block_offsetをアラインしても割り当て可能なブロックを取得する必要があります。
    // |------|------|------|------|------|------|------|
    // ooooooo*--------------------*--------------------*
    // ooooooo|--------------aligned_size---------------|
    // |-------------|------it_size_offset_capable_block-------|
    // |------------------------------------------------*------|

    auto alignment_reserve = _alignment > capable_alignment ? _alignment - capable_alignment : 0;
    auto&& it_size_capable_blocks = free_blocks_by_size.lower_bound(aligned_size + alignment_reserve);
    if (it_size_capable_blocks == free_blocks_by_size.end())
        return ALLOCATION{};

    auto it_size_offset_capable_block = it_size_capable_blocks->second;
    auto&& size_offset_capable_block = *it_size_offset_capable_block;
    OffsetT block_offset = size_offset_capable_block.first;
    SizeT   block_size   = size_offset_capable_block.second.size;

    // <--------------------------------block_size------------------------------>
    // |block_offset |aligned_offset                                            |
    // <---margin--->                                                           |
    // <------aligned_size------><---margin--->|***                             |
    // <----------------result----------------><-------------------------------->
    // 
    // |block_offset |aligned_offset           |                                |
    // |not use      |aligned_offset + size    |
    // <xxxxxxxxxxxx>|----result-------------->|

    auto aligned_offset         = util::AlignUp(block_offset, _alignment);
    auto margin                 = aligned_offset - block_offset;

    ALLOCATION result{ block_offset, aligned_size + margin };

    // ブロックの情報を更新
    free_blocks_by_size  .erase(it_size_capable_blocks);
    free_blocks_by_offset.erase(it_size_offset_capable_block);
    free_size -= result.size;
    auto new_block_size = block_size - result.size;
    if (new_block_size > 0)
        AddNewBlock(block_offset + result.size, new_block_size);

    // capable_alignment  |---------------|...
    //            align : |--------------->
    //        not align1: |------->       |        
    //        not align2: |---------------------->
    if (!util::IsAligned(aligned_size, capable_alignment))
    {
        // not po2: |-|---|--->---|...
        //     po2: |-|---|------->...
        if (util::IsPowOfTwo(aligned_size))// aligned_sizeが2のべき乗の場合、次回割当時により広いカバレッジのアライメントを使用できます(より小さなブロックをアライメントの考慮(alignment_reserve)をせずに割当可能になります)。
            capable_alignment = aligned_size;
        else
            capable_alignment = std::min(capable_alignment, _alignment);
    }

    if (block_size == max_block_size)
        max_block_size = new_block_size;

    return result;
}

void
VariableSizeAllocationsManager::Free(ALLOCATION& _allocation)
{
    assert(_allocation.size);

    if (free_blocks_by_offset.empty())
    {
        UpdateBlockInfo(_allocation.offset, _allocation.size, _allocation);
        return;
    }

    MapFreeBlocksByOffset::iterator it_next_block = free_blocks_by_offset.upper_bound(_allocation.offset);
    OffsetT offset = _allocation.offset;
    SizeT   size   = _allocation.size;

    if (GetNumFreeBlocks() > 1 &&
        it_next_block != free_blocks_by_offset.begin()) // begin()の場合it_prev_blockは存在しない
    {
        MapFreeBlocksByOffset::iterator it_prev_block = it_next_block;
        --it_prev_block;
        auto&& prev_block = *it_prev_block;
        if (_allocation.offset == prev_block.first + prev_block.second.size)
        {
            // prev_blockと隣接している
            // |prev_block_offset            |_allocation.offset            |~~
            // |<------prev_block_size------>|<------_allocation.size------>|~~
            offset = prev_block.first;
            size  += prev_block.second.size;
            free_blocks_by_size.erase(prev_block.second.it_order_by_size);
            free_blocks_by_offset.erase(it_prev_block);
        }
    }

    if (it_next_block != free_blocks_by_offset.end())
    {
        auto&& next_block = *it_next_block;
        if (_allocation.offset + _allocation.size == next_block.first)
        {
            // next_blockと隣接している
            // ~~|_allocation.offset            |next_block.first
            // ~~|<------_allocation.size------>|<-----next_block.second.size----->|
            size += next_block.second.size;
            free_blocks_by_size.erase(next_block.second.it_order_by_size);
            free_blocks_by_offset.erase(it_next_block);
        }
        else
        {
            // こんな場合は結合出来ない
            // |prev_block.first                  |使用中  |_allocation.offset              |使用中  |next_block.first                  |
            // |<-----prev_block.second.size----->|<xxxxxx>|<------_allocation.size-------->|<xxxxxx>|<-----next_block.second.size----->|
        }
    }

    UpdateBlockInfo(offset, size, _allocation);

    if (IsEmpty())
    {
        BUMA_ASSERT(GetNumFreeBlocks() == 1);
        ResetCapableAlignment();
    }

    _allocation.offset = 0;
    _allocation.size   = 0;
}

void VariableSizeAllocationsManager::UpdateBlockInfo(OffsetT _new_block_offset, SizeT _new_block_size, const ALLOCATION& _allocation)
{
    free_size += _allocation.size;
    assert(free_size <= page_size);

    max_block_size = std::max(max_block_size, _new_block_size);
    AddNewBlock(_new_block_offset, _new_block_size);
}

bool VariableSizeAllocationsManager::CheckAllocatable(SizeT _aligned_size) const
{
    // 割当可能かを簡易チェック
    return _aligned_size != 0 && _aligned_size <= max_block_size;
}

void VariableSizeAllocationsManager::AddNewBlock(OffsetT _offset, SizeT _size)
{
    auto&& it_new_block_offset = free_blocks_by_offset.emplace(_offset, FREE_BLOCK_INFO{ _size, {} }).first;
    auto&& it_new_block_size   = free_blocks_by_size.emplace(_size, it_new_block_offset);
    it_new_block_offset->second.it_order_by_size = it_new_block_size;
}

void VariableSizeAllocationsManager::ResetCapableAlignment()
{
    capable_alignment = min_alignment;
    while (capable_alignment * 2 <= page_size)
        capable_alignment *= 2;
}


}// namespace buma
