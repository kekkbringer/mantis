//
// Created by dominik on 2/25/26.
//

#ifndef UNTITLED_ARENA_ALLOCATOR_HPP
#define UNTITLED_ARENA_ALLOCATOR_HPP

#include <cstddef>
#include <utility>
#include <vector>

/**
 * A bump pointer arena allocator, manages memory in slabs of increasing size. Individual allocations are all made
 * sequentially within the slabs. The size of the slabs double each time up to a fixed limit. A newly created slab is at
 * least as large as needed for the requested allocation.
 * All objects allocated in the arena are required to be trivially destructible.
 */
class ArenaAllocator {
private:
    /**
     * One slab of memory, only used within the arena allocator itself.
     */
    struct Slab {
        std::byte* data; ///< pointer to the start of the memory
        size_t capacity; ///< total size of the slab
        size_t used;     ///< how much memory is already used

        explicit Slab(const size_t cap) : capacity(cap), used(0) {
            // just call memory allocation, without any constructor
            // use maximum fundamental alignment, should be 16 bytes on x86-64
            data = static_cast<std::byte*>(::operator new(cap, std::align_val_t{alignof(std::max_align_t)}));
        }

        ~Slab() {
            // only free memory, no destructor called
            ::operator delete(data, std::align_val_t{alignof(std::max_align_t)});
        }

        // diable copy constructor and assignment
        Slab(const Slab&) = delete;
        Slab& operator=(const Slab&) = delete;
    };

    // size of the first slab that's allocated by the arena, size will grow with each slab
    static constexpr size_t initial_slab_size = 64 * 1024;        // 64 KB
    // maximal size of a slab
    static constexpr size_t max_slab_size     =  4 * 1024 * 1024; // 4MB

    std::vector<Slab*> slabs; ///< keeps pointer to all slabs

    /**
     * Allocate memory in a slab and return a pointer to the address. If the new allocation is small enough to fit in
     * the last slab (including alignment), it is put there, otherwise a new slab is created and the memory is allocated
     * in the new slab instead. The new slab will be twice the size of the last slab up to the size specified by
     * max_slab_size. If the new slab would still be too small, its size is increased to perfectly fit the object to be
     * allocated.
     * @param size size of memory to be allocated
     * @param alignment alignment requirement of the object
     * @return pointer to allocated memory
     */
    void* allocate(const size_t size, const size_t alignment) {
        // try to put into the latest slab first
        Slab* current = slabs.back();

        // calculate the next alignment offset
        size_t aligned_offset = (current->used + alignment - 1) & ~(alignment - 1);

        // try to fit into the current slab
        if (aligned_offset + size <= current->capacity) {
            void* ptr = current->data + aligned_offset;
            current->used = aligned_offset + size;
            return ptr;

        // does not fit -> new slab
        } else {
            // new slab will be double the size of the old one (up to max_slab_size)
            // additionally, guarantee that the new slab is large enough for the allocation of the object
            const size_t new_slab_size = std::max(
                std::min(2 * current->capacity, max_slab_size), // twice the last size
                size + alignment                                  // at least enough for the object
            );
            // this allocation could throw std::bad_alloc
            // we don't catch here, since compilation can continue anyway, so just throw the error to the user
            slabs.emplace_back(new Slab(new_slab_size));

            // allocate in the new slab
            // since the constructor of Slab takes care of the alignment, we can always start at 0 offset
            Slab* new_slab = slabs.back();
            // calculate offset just to be sure
            aligned_offset = (new_slab->used + alignment - 1) & ~(alignment - 1);
            void* ptr = new_slab->data + aligned_offset;
            new_slab->used = aligned_offset + size;
            return ptr;
        }
    }

public:
    ArenaAllocator() {
        slabs.emplace_back(new Slab(initial_slab_size));
    }

    ~ArenaAllocator() {
        // simply delete each Slab, no destructor is called
        // therefore, trivial destructibility is required.
        for (const Slab* slab : slabs) delete slab;
    }

    // disable copy constructor, copy assignment, move constructor and move assignment
    ArenaAllocator(const ArenaAllocator&)            = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&&)                 = delete;
    ArenaAllocator& operator=(ArenaAllocator&&)      = delete;

    /**
     * This function allocates and constructs an object of type T in the arena, using the provided arguments in its
     * construction. T must be trivially destructible.
     * @tparam T type of the object to be allocated
     * @tparam Args argument types of T's constructor
     * @param args arguments forwarded to T's constructor
     * @return pointer to the constructed object
     */
    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        // make sure T is trivially destructible
        static_assert(std::is_trivially_destructible_v<T>, "ArenaAllocator requires trivially destructible types");

        // call private allocate function
        void* ptr = allocate(sizeof(T), alignof(T));

        // construct the object by forwarding all arguments and return its address
        return new(ptr) T(std::forward<Args>(args)...);
    }

    /**
     * Allocates a contiguous array of objects of type T in the arena. The objects are not constructed. The array is
     * intended to store pointers or other trivially destructible types. T must be trivially destructible.
     * @tparam T type of the objects in the array to be allocated
     * @param count number of objects in the array
     * @return pointer to the start of the allocated array
     */
    template<typename T>
    T* allocate_array(const size_t count) {
        // make sure T is trivially destructible
        static_assert(std::is_trivially_destructible_v<T>, "ArenaAllocator requires trivially destructible types");

        // request for zero objects -> return null pointer
        if (count == 0) return nullptr;

        // call private allocate function
        void* ptr = allocate(count * sizeof(T), alignof(T));

        // cast pointer and return, no construction
        return static_cast<T*>(ptr);
    }
};

#endif //UNTITLED_ARENA_ALLOCATOR_HPP