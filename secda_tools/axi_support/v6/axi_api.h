#ifndef AXI_API_H
#define AXI_API_H

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

static unsigned int ubuf_alloced[8] = {0};

template <typename T>
T *ubuf_mm_alloc_rw(unsigned int address, unsigned int buffer_size,
                    int buffer_id) {
  string cmd = "echo 'create udmabuf" + to_string(buffer_id) + " " +
               to_string(buffer_size) + "' > /dev/u-dma-buf-mgr";
  ubuf_alloced[buffer_id] = buffer_size;
  // cerr << "Allocating u-dmabuf" << buffer_id << " of size " << buffer_size
  //      << " bytes" << endl;
  cerr << cmd << endl;
  int ret = system(cmd.c_str());
  if (ret != 0) {
    cerr << "Failed to create udmabuf" << buffer_id << " of size "
         << buffer_size << endl;
    exit(EXIT_FAILURE);
  }
  string path = "/dev/udmabuf" + to_string(buffer_id);
  void *addr;
  int fd;
  if ((fd = open(path.c_str(), O_RDWR | O_SYNC)) != -1) {
    addr = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
  }
  if (addr == (void *)-1) exit(EXIT_FAILURE);
  T *acc = reinterpret_cast<T *>(addr);
  return acc;
}

template <typename T>
unsigned long ubuf_get_phy_addr(int buffer_id) {
  char attr[1024];
  unsigned long phys_addr = 0;
  string path =
      "/sys/class/u-dma-buf/udmabuf" + to_string(buffer_id) + "/phys_addr";
  int fd;
  if ((fd = open(path.c_str(), O_RDONLY)) != -1) {
    ssize_t r = read(fd, attr, sizeof(attr) - 1);
    if (r > 0) {
      attr[r] = '\0';
      sscanf(attr, "%lx", &phys_addr);
    }
    close(fd);
  }
  return phys_addr;
}

template <typename T>
unsigned int ubuf_free(int buffer_id) {
  if (buffer_id < 0 || buffer_id >= 8 || ubuf_alloced[buffer_id] == 0) {
    cerr << "Invalid buffer ID" << endl;
    return -1;
  }
  string cmd =
      "echo 'delete udmabuf" + to_string(buffer_id) + "' > /dev/u-dma-buf-mgr";
  ubuf_alloced[buffer_id] = 0;
  return system(cmd.c_str());
}

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
void *mm_dealloc(T *buffer, unsigned int buffer_size) {
  if (munmap(buffer, buffer_size) == -1) {
    cerr << "Failed to unmap memory" << endl;
  }
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

#include "control_modules.h"
#include "mm_buffer.h"
#include "stream_dma.h"

// #ifdef SYSC
// // #include "axi_api_sysc.tpp"
// #include "control_modules_sysc.tpp"
// #include "mm_buffer_sysc.tpp"
// #include "stream_dma_sysc.tpp"
// #else
// // #include "axi_api.tpp"
// #include "control_modules.tpp"
// #include "mm_buffer.tpp"
// #include "stream_dma.tpp"
// #endif

#endif // AXI_API_H