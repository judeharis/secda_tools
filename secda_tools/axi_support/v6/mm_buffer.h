#ifndef MM_BUFFER_V6_H
#define MM_BUFFER_V6_H

// ================================================================================
// MM_Buffer API || Public
// ================================================================================

template <typename T>
struct mm_buffer {
  T *buffer;
  unsigned int addr;
  unsigned int size;
  static int mm_id;
  const int id;
  string name;

#ifdef SYSC
  hls_bus_chn<T> buffer_chn;
#endif

  mm_buffer(unsigned int _addr, unsigned int _size, string name);

  mm_buffer(unsigned int _addr, unsigned int _size);

  ~mm_buffer();

  T *get_buffer();

  void sync_from_acc();

  void sync_to_acc();
};


#ifdef SYSC
#include "mm_buffer_sysc.tpp"
#else
#include "mm_buffer.tpp"
#endif

#endif // MM_BUFFER_V6_H