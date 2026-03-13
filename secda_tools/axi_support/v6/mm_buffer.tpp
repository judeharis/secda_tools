#ifndef SYSC

// ================================================================================
// AXIMM API
// ================================================================================
template <typename T>
int mm_buffer<T>::mm_id = 0;

template <typename T>
mm_buffer<T>::mm_buffer(unsigned int _addr, unsigned int _size, string name)
    : id(mm_id++), name(name) {
  size = _size;
  addr = _addr / 4; // Convert to 32-bit words
  buffer = mm_alloc_rw<T>(_addr, _size * sizeof(T));
}

template <typename T>
mm_buffer<T>::mm_buffer(unsigned int _addr, unsigned int _size) : id(mm_id++) {
  size = _size;
  addr = _addr / 4; // Convert to 32-bit words
  buffer = mm_alloc_rw<T>(_addr, _size * sizeof(T));
}

template <typename T>
T *mm_buffer<T>::get_buffer() {
  return buffer;
}
template <typename T>
void mm_buffer<T>::sync_from_acc() {}

template <typename T>
void mm_buffer<T>::sync_to_acc() {}

#endif