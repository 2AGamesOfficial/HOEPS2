#pragma once
#include <tamtypes.h>
#include <new>

namespace Tyra {

class FrameArena {
 public:
  static const u32 ARENA_SIZE = 65536;  // 64 KB
  FrameArena() : offset(0) {}
  void reset() { offset = 0; }
  void* allocRaw(u32 size, u32 align = 8) {
    u32 mask = align - 1;
    offset = (offset + mask) & ~mask;
    if (offset + size > ARENA_SIZE) return nullptr;
    void* p = buffer + offset;
    offset += size;
    return p;
  }
  template<typename T, typename... Args>
  T* alloc(Args&&... args) {
    void* mem = allocRaw(sizeof(T), alignof(T));
    if (!mem) return nullptr;
    return new (mem) T(static_cast<Args&&>(args)...);
  }
  template<typename T>
  T* allocArray(u32 count) {
    void* mem = allocRaw(sizeof(T) * count, alignof(T));
    if (!mem) return nullptr;
    T* arr = static_cast<T*>(mem);
    for (u32 i = 0; i < count; i++) new (&arr[i]) T();
    return arr;
  }
  u32 getUsed() const { return offset; }
 private:
  alignas(16) u8 buffer[ARENA_SIZE];
  u32 offset;
};

extern FrameArena g_frameArena;

}  // namespace Tyra
