#ifndef ALLOCATOR_H
#define ALLOCATOR_H

struct free_memory
{
    memsize Size;
    free_memory *Next;
};

struct memory_region
{
    memsize Size;
    u8 *Base;
    memsize Used;
    free_memory *FreeList;
};

inline void CreateRegion(memory_region *Region, memsize Size, u8 *Base)
{
    Assert(Size > sizeof(free_memory));

    Region->Size = Size;
    Region->Base = Base;
    Region->Used = 0;
    Region->FreeList = (free_memory *)Base;
    Region->FreeList->Size = Size;
    Region->FreeList->Next = nullptr;
}

#define AllocStruct(region, type) (type *)AllocSize(region, sizeof(type))
#define AllocArray(region, count, type) (type *)AllocSize(region, (count)*sizeof(type))
inline void *AllocSize(memory_region *Region, memsize Size)
{
    Assert(Region->Used + Size <= Region->Size);
    Assert(Size > 0);
    
    // NOTE: Make sure there's enough space to cast to free_memory on Dealloc
    if(Size < sizeof(free_memory))
    {
        Size = sizeof(free_memory);
    }

    free_memory *Prev = nullptr;
    free_memory *Curr = Region->FreeList;
    memsize TotalSize = Size;

    while(Curr != nullptr)
    {
        // NOTE: If current region's size is too small, save current to prev and continue to next
        if(Curr->Size < Size)
        {
            Prev = Curr;
            Curr = Curr->Next;
            continue;
        }

        // NOTE: If there isn't enough space left for a new free region then consume the entire region
        //       else allocate, then create a new free region and add it to the list
        if(Curr->Size - Size < sizeof(free_memory))
        {
            TotalSize = Curr->Size;

            if(Prev != nullptr) { Prev->Next = Curr->Next; }
            else { Region->FreeList = Curr->Next; }
        }
        else
        {
            free_memory *Free = (free_memory *)((uptr)Curr + Size);
            Free->Size = Curr->Size - Size;
            Free->Next = Curr->Next;

            if(Prev != nullptr) { Prev->Next = Free; }
            else { Region->FreeList = Free; }
        }

        Region->Used += TotalSize;
        return (void *)Curr;
    }

    Assert(!"Region out of memory!");

    return nullptr;
}

inline void DeallocSize(memory_region *Region, void *Base, memsize Size)
{
    Assert(Base != nullptr);

    free_memory *Prev = nullptr;
    free_memory *Curr = Region->FreeList;

    while(Curr != nullptr)
    {
        // NOTE: Check if the current free region address is past the region to the deallocated
        if((uptr)Curr >= ((uptr)Base + Size)) { break; }

        Prev = Curr;
        Curr = Curr->Next;
    }

    // NOTE: If no previous, current is past the given base address, so free and add to freelist
    //       else if previous is adjacent to the given base, just increase previous' size to overlap
    //       else create a new free region and place it inbetween prev and prev's next
    if(Prev == nullptr)
    {
        Prev = (free_memory *)Base;
        Prev->Size = Size;
        Prev->Next = Region->FreeList;
        Region->FreeList = Prev;
    }
    else if(((uptr)Prev + Prev->Size) == (uptr)Base)
    {
        Prev->Size += Size;
    }
    else
    {
        free_memory *Free = (free_memory *)Base;
        Free->Size = Size;
        Free->Next = Prev->Next;
        Prev->Next = Free;
        Prev = Free; // NOTE: This is so that the check below updates the correct node!
    }

    // NOTE: If current isn't null and it's adjacent to the given base, grow prev and update link
    if((Curr != nullptr) && ((uptr)Curr == ((uptr)Base + Size)))
    {
        Prev->Size += Curr->Size;
        Prev->Next = Curr->Next;
    }
    
    Region->Used -= Size;
}

inline void ZeroSize(memsize Size, void *Base)
{
    u8 *Byte = (u8 *)Base;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

#endif

