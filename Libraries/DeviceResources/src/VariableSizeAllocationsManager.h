#pragma once

#include <map>

namespace buma
{

/**
 * @brief メモリ割当におけるオフセットとサイズ管理のみを目的とした割り当てマネージャ
 * @note 割り当てのアルゴリズムについて: Variable Size Memory Allocations Manager: http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
*/
class VariableSizeAllocationsManager
{
public:
    using OffsetT   = size_t;
    using SizeT     = size_t;
    struct ALLOCATION
    {
        operator bool()       { return size != 0; }
        operator bool() const { return size != 0; }
        OffsetT offset;
        SizeT   size;
    };

public:
    VariableSizeAllocationsManager(SizeT _page_size, SizeT _min_alignment = 8);
    VariableSizeAllocationsManager(const VariableSizeAllocationsManager&) = delete;
    ~VariableSizeAllocationsManager();

    void Reset();

    // 戻り値のoffsetはアラインされていない可能性がありますが、offsetを_alignment値で切り上げた際にsizeが不足しない事は保証されます: 
    //      (AlignUp(ALLOCATION::offset, _alignment) + _size) <= ALLOCATION::size
    ALLOCATION Allocate (SizeT _size, SizeT _alignment);
    void       Free     (ALLOCATION& _allocation);

    bool    IsEmpty()          const { return page_size == free_size; }
    SizeT   GetPageSize()      const { return page_size; }
    SizeT   GetRemainingSize() const { return free_size; }
    size_t  GetNumFreeBlocks() const { return free_blocks_by_offset.size(); }
    SizeT   GetMaxBlockSize()  const { return max_block_size; }

private:
    void UpdateBlockInfo(OffsetT _offset, SizeT _size, const ALLOCATION& _allocation);
    bool CheckAllocatable(SizeT _aligned_size) const;
    void AddNewBlock(OffsetT _offset, SizeT _size);
    void ResetCapableAlignment();

private:
    struct FREE_BLOCK_INFO;
    using MapFreeBlocksByOffset = std::map<OffsetT, FREE_BLOCK_INFO>;
    using MapFreeBlocksBySize   = std::multimap<SizeT, MapFreeBlocksByOffset::iterator>;

    struct FREE_BLOCK_INFO
    {
        OffsetT                       size;             // ブロックサイズ(割り当てサイズ用の予約スペースなし)
        MapFreeBlocksBySize::iterator it_order_by_size; // ブロックサイズでソートされたマルチマップでこのブロックを参照するイテレータ
    };

private:
    const SizeT             page_size;
    const SizeT             min_alignment;
    SizeT                   free_size;
    SizeT                   capable_alignment;
    SizeT                   max_block_size;

    MapFreeBlocksByOffset   free_blocks_by_offset;
    MapFreeBlocksBySize     free_blocks_by_size;

};


}// namespace buma
