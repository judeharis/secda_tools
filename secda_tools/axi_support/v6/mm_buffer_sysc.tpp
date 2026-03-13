#ifdef SYSC

// ================================================================================
// AXIMM API
// ================================================================================
template <typename T>
int mm_buffer<T>::mm_id = 0;
// int mm_id = 0;

template <typename T>
mm_buffer<T>::mm_buffer(unsigned int _addr, unsigned int _size, string name)
    : id(mm_id++), buffer_chn(("mm_buffer_chn_" + std::to_string(mm_id) + "_" +
                               std::to_string(_addr) + "_" + name)
                                  .c_str(),
                              0, _size - 1) {
  size = _size;
  addr = 0;
  buffer = (T *)malloc(_size * sizeof(T));
  // Initialize with zeros
  for (unsigned int i = 0; i < _size; i++) *(buffer + i) = 0;
}

template <typename T>
mm_buffer<T>::mm_buffer(unsigned int _addr, unsigned int _size)
    : id(mm_id++), buffer_chn(("mm_buffer_chn_" + std::to_string(mm_id) + "_" +
                               std::to_string(_addr))
                                  .c_str(),
                              0, _size - 1) {
  size = _size;
  addr = 0;
  buffer = (T *)malloc(_size * sizeof(T));
  // Initialize with zeros
  for (unsigned int i = 0; i < _size; i++) *(buffer + i) = 0;
  buffer_chn.burst_write(0, size, (T *)&buffer[0]);
}

template <typename T>
T *mm_buffer<T>::get_buffer() {
  return buffer;
}

template <typename T>
void mm_buffer<T>::sync_from_acc() {
  buffer_chn.burst_read(0, size, (T *)&buffer[0]);
}

template <typename T>
void mm_buffer<T>::sync_to_acc() {
  buffer_chn.burst_write(0, size, (T *)&buffer[0]);
}

#endif