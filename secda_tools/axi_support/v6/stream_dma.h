#ifndef STREAM_DMA_V6_H
#define STREAM_DMA_V6_H

#ifndef SYSC
extern "C" {
#include "../libxlnk_cma.h"
}
#endif

#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_CURDESC 0x08
#define MM2S_CURDESC_MSB 0x0C
#define MM2S_TAILDESC 0x10
#define MM2S_TAILDESC_MSB 0x14
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_CURDESC 0x38
#define S2MM_CURDESC_MSB 0x3C
#define S2MM_TAILDESC 0x40
#define S2MM_TAILDESC_MSB 0x44
#define S2MM_DESTINATION_ADDRESS 0x48
#define S2MM_LENGTH 0x58

#define SG_CTL 0x2C

#define MAX_DESC 256
#define DESC_SIZE 64

// #define DEF_UDMA
#ifdef DEF_UDMA
#define USE_UBUF
#endif

// #define DEF_CMA
#ifdef DEF_CMA
#define USE_CMA
#endif

// ================================================================================
// CMA Buffer API || Public
// ================================================================================

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
#else

template <typename T>
T *cmap_alloc_rw(unsigned int buffer_size) {
  return new T[buffer_size / sizeof(T)];
}

template <typename T>
T *cmap_map_rw(unsigned int address, unsigned int buffer_size) {
  return new T[buffer_size / sizeof(T)];
}
#endif

// ================================================================================
// Stream DMA API || Private
// ================================================================================

template <int B, int T>
struct stream_dma {

  bool sg_mode = false;
  unsigned int *dma_addr;
  int *mm2s_bd_addr;
  int *s2mm_bd_addr;
  int *input;
  int *output;

  unsigned int input_size;
  unsigned int output_size;

  // Physical addresses for DMA
  unsigned int dma_phy_addr;
  unsigned int mm2s_bd_phy_addr;
  unsigned int s2mm_bd_phy_addr;
  unsigned int input_addr;
  unsigned int output_addr;

  // SG Variables
  unsigned int mm2s_curdesc_offset;
  unsigned int mm2s_curdesc_phy_addr;

  unsigned int s2mm_curdesc_offset;
  unsigned int s2mm_curdesc_phy_addr;

  static int s_id;
  const int id;
  int cma_id = 0;

  int ubuf_id_in = -1;
  int ubuf_id_out = -1;
  static int ubuf_id;

  // Profiling variables
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

  // Constructors and Destructor
  stream_dma(unsigned int _dma_addr, unsigned int _input,
             unsigned int _input_size, unsigned int _output,
             unsigned int _output_size);

  stream_dma();

  ~stream_dma();

  void dma_init(unsigned int _dma_addr, unsigned int _input,
                unsigned int _input_size, unsigned int _output,
                unsigned int _output_size, bool _sg_mode = false);

  void initDMA(unsigned int src, unsigned int dst, bool sg_mode = false);

  void dma_free();

  // Underlying Functions
  void writeMappedReg(uint32_t offset, unsigned int val);
  unsigned int readMappedReg(uint32_t offset);
  void dma_mm2s_sync();
  void dma_s2mm_sync();

  // DMA Operations
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

  void dma_sg_mm2s_setup(unsigned int dst, unsigned int length, bool last);

  void dma_sg_s2mm_setup(unsigned int src, unsigned int length, bool last);

  void dma_sg_start_send();

  void dma_sg_start_recv();

  // Profiling and Utility
  void print_times();
};

// ================================================================================
// Multi Stream DMA API
// ================================================================================

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
            unsigned int in_buffer_size, unsigned int out_buffer_size,
            bool sg_mode = false);

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

#endif // STREAM_DMA_V6_H