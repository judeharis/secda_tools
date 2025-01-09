#ifndef SECDA_HW_UTILS_SC_H
#define SECDA_HW_UTILS_SC_H

#include <systemc.h>

template <typename T, unsigned int W>
struct sbram {
  T data[W];
  int size;
  int access_count;
  int idx;
  sbram() {
    size = W;
    access_count = 0;
  }
  T &operator[](int i) {
    idx = i;
    return data[i];
  }
  int &operator=(int val) {
    data[idx] = val;
    return data[idx];
  }
  void write(int i, T val) { data[i] = val; }
  T read(int i) { return data[i]; }
};

// sbam<sc_int<32>, 32> sram;

#endif // SECDA_HW_UTILS_SC_H