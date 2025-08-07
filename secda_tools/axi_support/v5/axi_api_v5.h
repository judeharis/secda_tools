#ifndef AXI_API_V5_H
#define AXI_API_V5_H

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

#ifndef SYSC
extern "C" {
#include "../libxlnk_cma.h"
}
#endif

#include "../../secda_profiler/profiler.h"
#define TSCALE microseconds
#define SEC seconds
#define TSCAST duration_cast<nanoseconds>
#define HEX(X) std::hex << X << std::dec

// TODO: Remove hardcode addresses, make it cleaner
using namespace std;
#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_DESTINATION_ADDRESS 0x48
#define S2MM_LENGTH 0x58
#define PAGE_SIZE getpagesize()

// TODO: Clean up code and seperate to AXI4Lite, AXI4S, AXI4MM

// ================================================================================
// AXI4Lite API
// ================================================================================
template <typename T>
T *getAccBaseAddress(size_t base_addr, size_t length) {
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  size_t virt_base = base_addr & ~(getpagesize() - 1);
  size_t virt_offset = base_addr - virt_base;
  void *addr = mmap(NULL, length + virt_offset, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, virt_base);
  close(fd);
  if (addr == (void *)-1) exit(EXIT_FAILURE);
  T *acc = reinterpret_cast<T *>(addr);
  return acc;
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
void setReg(int *acc, uint32_t offset) {
  void *base_addr = (void *)acc;
  *((volatile char *)(reinterpret_cast<char *>(base_addr) + offset)) = 1;
}
template <typename T>
char getReg(int *acc, uint32_t offset) {
  void *base_addr = (void *)acc;
  return *(
      (volatile unsigned char *)(reinterpret_cast<char *>(base_addr) + offset));
}

template <typename T>
struct acc_regmap {
  int *acc_addr;

  uint32_t *control_registers_offset;
  uint32_t *status_registers_offset;

  string *control_registers;
  string *status_registers;

  acc_regmap(size_t base_addr, size_t length);

  void writeAccReg(uint32_t offset, unsigned int val);

  unsigned int readAccReg(uint32_t offset);

  // TODO: parse JSON file to load offset map for control and status registers
  void parseOffsetJSON();

  // TODO: checks control and status register arrays to find the offsets for the
  // register
  uint32_t findRegOffset(string reg_name);

  void writeToControlReg(string reg_name, unsigned int val);

  unsigned int readToControlReg(string reg_name);
};
// ================================================================================
// Memory Map API
// ================================================================================

template <typename T>
T *mm_alloc_rw(unsigned int address, unsigned int buffer_size) {
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  size_t virt_base = address & ~(getpagesize() - 1);
  size_t virt_offset = address - virt_base;
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

#ifndef SYSC
template <typename T>
T *cmap_alloc_rw(unsigned int buffer_size) {
  // T *acc = reinterpret_cast<T *>(cma_alloc(buffer_size, 1));
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

// template <typename T>
// T *cmap_alloc_rw(unsigned int buffer_size) {
//   T *acc = reinterpret_cast<T *>(cma_alloc(buffer_size, 1));
//   cout << acc << endl;
//   T *mmaped = cmap_map_rw<T>(cma_get_phy_addr(acc), buffer_size);
//   cout << mmaped << endl;
//   return mmaped;
// }

#endif

// ================================================================================
// ACC Control API
// ================================================================================

template <typename T>
struct axi4lite_ctrl {
  int *reg_base;

  axi4lite_ctrl();
  axi4lite_ctrl(int *base_addr);
  unsigned int read_reg(unsigned int offset);
  void write_reg(unsigned int offset, unsigned int val);
};

template <typename T>
struct acc_ctrl : public axi4lite_ctrl<T> {

#ifdef SYSC
  ACC_CONTROL *ctrl;
  ctrl_signals *ctrl_sigs;
  reg_signals *reg_sigs;
#endif

  bool start = false;
  bool done = false;
  int sig_count;

  acc_ctrl();

  void init_sigs(int count);
  void start_acc();
  void wait_done();
  bool check_done();

  unsigned int get_reg(int addr);
  void set_reg(int addr, unsigned int val);

  void print_reg_map(bool clear_console);

private:
  using axi4lite_ctrl<T>::reg_base;
  using axi4lite_ctrl<T>::write_reg;

public:
  using axi4lite_ctrl<T>::read_reg;
  using axi4lite_ctrl<T>::axi4lite_ctrl;
};

template <typename T>
struct hwc_ctrl : public axi4lite_ctrl<T> {
#ifdef SYSC
  HWC_RESETTER *hwc_resetter;
  hwc_signals *ctrl;
  sc_signal<bool> reset;
#endif
  int hwc_count;

  hwc_ctrl();

  void init_hwc(int count);

  void reset_hwc();

  void set_target_state(int hwc, int target_state);

  unsigned int get_current_state(int hwc);

  unsigned int get_cycle_count(int hwc);

  void print_hwc_map(bool clear_console);

private:
  using axi4lite_ctrl<T>::reg_base;

public:
  using axi4lite_ctrl<T>::read_reg;
  using axi4lite_ctrl<T>::write_reg;
  using axi4lite_ctrl<T>::axi4lite_ctrl;
};

// ================================================================================
// AXIMM API
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

  T *get_buffer();

  void sync_from_acc();

  void sync_to_acc();
};

// template <typename T>
// struct multi_mm_buffer {
//   vector<mm_buffer<T>> *mm_bufs;
//   unsigned int *addrs;
//   unsigned int *sizes;
//   int mm_buf_count;

//   multi_mm_buffer(unsigned int _addr, unsigned int _size);

//   mm_buffer<int> *get_buffer(int id);

//   void sync_from_acc();

//   void sync_to_acc();
// };

// ================================================================================
// Stream DMA API
// ================================================================================
struct cma_buffer {
  void *data;
  int id;
  int size;
  unsigned int phy_addr;

  cma_buffer(void *_data, int _id, int _size)
      : data(_data), id(_id), size(_size) {}

  cma_buffer(void *_data, int _id, int _size, unsigned int _phy_addr)
      : data(_data), id(_id), size(_size), phy_addr(_phy_addr) {}

  cma_buffer() {
    data = NULL;
    id = -1;
    size = 0;
  }

#ifndef SYSC
  ~cma_buffer() { cma_free(data); }
#else
  ~cma_buffer() { free(data); }
#endif

#ifndef SYSC
  void dealloc() { cma_free(data); }
#else
  void dealloc() { free(data); }
#endif
};

template <int B, int T>
struct stream_dma {
  unsigned int *dma_addr;
  int *input;
  int *output;

  vector<cma_buffer> input_bufs;

  vector<void *> input_bufs_ptrs;
  vector<unsigned int> input_bufs_phy_addr;
  vector<unsigned int> input_bufs_size;
  vector<int> input_bufs_id;

  unsigned int input_bufs_allocated_size = 0;

  unsigned int input_size;
  unsigned int output_size;

  unsigned int input_addr;
  unsigned int output_addr;

  static int s_id;
  const int id;
  int cma_id = 0;

  unsigned int data_transfered = 0;
  unsigned int data_transfered_recv = 0;
  unsigned int data_send_count = 0;
  unsigned int data_recv_count = 0;
  duration_ns send_wait;
  duration_ns recv_wait;
  chrono::high_resolution_clock::time_point send_start;

#ifdef SYSC
  AXIS_ENGINE<B, AXI_TYPE> *dmad;
#endif

  stream_dma(unsigned int _dma_addr, unsigned int _input,
             unsigned int _input_size, unsigned int _output,
             unsigned int _output_size);

  stream_dma(unsigned int _dma_addr, unsigned int _input, unsigned int _r_paddr,
             unsigned int _input_size, unsigned int _output,
             unsigned int _w_paddr, unsigned int _output_size);

  stream_dma();

  ~stream_dma();

  void dma_init(unsigned int _dma_addr, unsigned int _input,
                unsigned int _input_size, unsigned int _output,
                unsigned int _output_size);

  void initDMA(unsigned int src, unsigned int dst);

  void dma_free();

  void dma_change_start(int offset);

  void dma_change_start(unsigned int addr, int offset);

  void dma_change_end(int offset);

  int *dma_get_inbuffer();

  int *dma_get_outbuffer();

  void dma_start_send(int length);

  void dma_wait_send();

  int dma_check_send();

  void dma_start_recv(int length);

  void dma_wait_recv();

  int dma_check_recv();

  void print_times();

  int dma_alloc_input_buffer(int buffer_size);

  void dma_dealloc_input_buffer(int id);

  int *dma_get_input_buffer(int id);

  void dma_change_input_buffer(int id, int offset);

  void dma_default_input_buffer();

  void dma_send_buffer(int id, int length, int offset);

  unsigned int dma_pages_available();
  //********************************** Unexposed Functions
  //**********************************

  void writeMappedReg(uint32_t offset, unsigned int val);
  unsigned int readMappedReg(uint32_t offset);
  void dma_mm2s_sync();
  void dma_s2mm_sync();
};

template <int B, int T>
struct multi_dma {
  struct stream_dma<B, T> *dmas;
  unsigned int *dma_addrs;
  unsigned int *dma_addrs_in;
  unsigned int *dma_addrs_out;
  unsigned int in_buffer_size;
  unsigned int out_buffer_size;
  int dma_count;

  ~multi_dma();

  multi_dma(int _dma_count, unsigned int *_dma_addrs,
            unsigned int *_dma_addrs_in, unsigned int *_dma_addrs_out,
            unsigned int buffer_size);

  multi_dma(int _dma_count, unsigned int *_dma_addrs,
            unsigned int *_dma_addrs_in, unsigned int *_dma_addrs_out,
            unsigned int in_buffer_size, unsigned int out_buffer_size);

  void multi_free_dmas();

  void multi_dma_change_start(int offset);

  void multi_dma_change_start_4(int offset);

  void multi_dma_change_end(int offset);

  void multi_dma_start_send(int length);

  void multi_dma_wait_send();

  int multi_dma_check_send();

  void multi_dma_start_recv(int length);

  void multi_dma_start_recv();

  void multi_dma_wait_recv();

  void multi_dma_wait_recv_4();

  int multi_dma_check_recv();

  void print_times();
};

#ifdef SYSC
#include "axi_api_sysc_v5.tpp"
#else
#include "axi_api_v5.tpp"
#endif

#endif // AXI_API_V5_H