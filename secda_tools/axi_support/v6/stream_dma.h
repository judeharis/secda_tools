#ifndef STREAM_DMA_V6_H
#define STREAM_DMA_V6_H

#ifndef SYSC
extern "C" {
#include "../libxlnk_cma.h"
}
#endif

// TODO: Remove hardcode addresses, make it cleaner
#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_DESTINATION_ADDRESS 0x48
#define S2MM_LENGTH 0x58

// ================================================================================
// CMA Buffer || Private
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

// ================================================================================
// Stream DMA API || Private
// ================================================================================

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

  void writeMappedReg(uint32_t offset, unsigned int val);
  unsigned int readMappedReg(uint32_t offset);
  void dma_mm2s_sync();
  void dma_s2mm_sync();
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

#endif // STREAM_DMA_V6_H