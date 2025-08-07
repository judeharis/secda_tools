#ifndef AXI_API_V6_H
#define AXI_API_V6_H

#ifdef SYSC
#include "../../secda_integrator/axi4s_engine_generic.sc.h"
#include "../../secda_integrator/sysc_types.h"
#endif

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../secda_profiler/profiler.h"
#define TSCALE microseconds
#define SEC seconds
#define TSCAST duration_cast<nanoseconds>
#define HEX(X) std::hex << X << std::dec

using namespace std;
#define PAGE_SIZE getpagesize()

// ================================================================================
// Memory Map API || Public
// ================================================================================

template <typename T>
T *mm_alloc_rw(unsigned int base_addr, unsigned int buffer_size) {
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  size_t virt_base = base_addr & ~(getpagesize() - 1);
  size_t virt_offset = base_addr - virt_base;
  void *addr = mmap(NULL, buffer_size + virt_offset, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, virt_base);
  close(fd);
  if (addr == (void *)-1) exit(EXIT_FAILURE);
  T *acc = reinterpret_cast<T *>(addr);
  return acc;
}

template <typename T>
T *mm_alloc_r(unsigned int address, unsigned int buffer_size) {
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  size_t virt_base = address & ~(getpagesize() - 1);
  size_t virt_offset = address - virt_base;
  void *addr = mmap(NULL, buffer_size + virt_offset, PROT_READ, MAP_SHARED, fd,
                    virt_base);
  close(fd);
  if (addr == (void *)-1) exit(EXIT_FAILURE);
  T *acc = reinterpret_cast<T *>(addr);
  return acc;
}

template <typename T>
vector<T> multi_mm_alloc_rw(unsigned int address, unsigned int size_needed) {
  unsigned buffer_size = 1024 * 64 * 4;
  unsigned bufs_needed = size_needed / buffer_size;
  size_t virt_base = address & ~(getpagesize() - 1);
  size_t virt_offset = address - virt_base;
  vector<T> accs;

  for (unsigned i = 0; i < bufs_needed; i++) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    void *addr = mmap(NULL, buffer_size + virt_offset, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, virt_base);
    close(fd);
    if (addr == (void *)-1) exit(EXIT_FAILURE);
    T acc = reinterpret_cast<T>(addr);
    accs.push_back(acc);
    virt_base += buffer_size;
  }
  return accs;
}

template <typename T>
void writeMappedReg(int *acc, uint32_t offset, T val) {
  void *base_addr = (void *)acc;
  *((volatile T *)(reinterpret_cast<char *>(base_addr) + offset)) = val;
}

template <typename T>
T readMappedReg(int *acc, uint32_t offset) {
  void *base_addr = (void *)acc;
  return *((volatile T *)(reinterpret_cast<char *>(base_addr) + offset));
}

template <typename T>
T *getAccBaseAddress(size_t base_addr, size_t length) {
  return mm_alloc_rw<T>(base_addr, length);
}

template <typename T>
void setReg(int *acc, uint32_t offset) {
  writeMappedReg<char>(acc, offset, 1);
}
template <typename T>
char getReg(int *acc, uint32_t offset) {
  return readMappedReg<unsigned char>(acc, offset);
}

#ifndef SYSC
template <typename T>
T *cmap_alloc_rw(unsigned int buffer_size) {
  void *buf = cma_alloc(buffer_size, 0);
  if (buf == NULL) {
    cerr << "Failed to allocate CMA Buffer" << endl;
    return NULL;
  }
  T *acc = reinterpret_cast<T *>(buf);
  return acc;
}

template <typename T>
T *cmap_map_rw(unsigned int address, unsigned int buffer_size) {
  size_t virt_base = address & ~(getpagesize() - 1);
  size_t virt_offset = address - virt_base;
  T *acc = reinterpret_cast<T *>(cma_mmap(virt_base, buffer_size));
  return acc;
}
#endif

#include "control_modules.h"
#include "mm_buffer.h"
#include "stream_dma.h"

#ifdef SYSC
#include "axi_api_sysc.tpp"
#else
#include "axi_api.tpp"
#endif

#endif // AXI_API_V6_H