#ifdef SYSC

// TODO Implement SystemC Signal Read/Write
// ================================================================================
// AXI4Lite API
// ================================================================================

template <typename T>
acc_regmap<T>::acc_regmap(size_t base_addr, size_t length) {}

template <typename T>
void acc_regmap<T>::writeAccReg(uint32_t offset, unsigned int val) {}

template <typename T>
unsigned int acc_regmap<T>::readAccReg(uint32_t offset) {
  return 0;
}

// TODO: parse JSON file to load offset map for control and status registers
template <typename T>
void acc_regmap<T>::parseOffsetJSON() {}

// TODO: checks control and status register arrays to find the offsets for the
// register
template <typename T>
uint32_t acc_regmap<T>::findRegOffset(string reg_name) {
  return 0;
}

template <typename T>
void acc_regmap<T>::writeToControlReg(string reg_name, unsigned int val) {}

template <typename T>
unsigned int acc_regmap<T>::readToControlReg(string reg_name) {
  return 0;
}

// ================================================================================
// Memory Map API
// ================================================================================

// Make this into a struct based API (for SystemC)

// ================================================================================
// Stream DMA API
// ================================================================================

template <int B, int T>
int stream_dma<B, T>::s_id = 0;
// int sr_id = 0;

template <int B, int T>
stream_dma<B, T>::stream_dma(unsigned int _dma_addr, unsigned int _input,
                             unsigned int _r_paddr, unsigned int _input_size,
                             unsigned int _output, unsigned int _w_paddr,
                             unsigned int _output_size)
    : id(s_id++) {
  string name("SDMA" + to_string(id));
  dmad = new AXIS_ENGINE<B, AXI_TYPE>(&name[0]);

  dmad->id = id;
  dmad->input_len = 0;
  dmad->input_offset = 0;
  dmad->output_len = 0;
  dmad->output_offset = 0;
  dmad->r_paddr = _r_paddr;
  dmad->w_paddr = _w_paddr;
  dma_init(_dma_addr, _input, _input_size, _output, _output_size);
}

template <int B, int T>
stream_dma<B, T>::stream_dma() : id(s_id++) {
  string name("MSDMA" + to_string(id));
  dmad = new AXIS_ENGINE<B, AXI_TYPE>(&name[0]);
  dmad->input_len = 0;
  dmad->input_offset = 0;
  dmad->output_len = 0;
  dmad->output_offset = 0;
  dmad->id = id;
};

template <int B, int T>
void stream_dma<B, T>::dma_init(unsigned int _dma_addr, unsigned int _input,
                                unsigned int _input_size, unsigned int _output,
                                unsigned int _output_size) {
  input = (int *)malloc(_input_size * sizeof(int));
  output = (int *)malloc(_output_size * sizeof(int));

  // Initialize with zeros
  for (int64_t i = 0; i < _input_size; i++) {
    *(input + i) = 0;
  }

  for (int64_t i = 0; i < _output_size; i++) {
    *(output + i) = 0;
  }
  input_size = _input_size;
  output_size = _output_size;
  dmad->DMA_input_buffer = (int *)input;
  dmad->DMA_output_buffer = (int *)output;
  dmad->r_paddr = _input;
  dmad->w_paddr = _output;
}

template <int B, int T>
void stream_dma<B, T>::writeMappedReg(uint32_t offset, unsigned int val) {}

template <int B, int T>
unsigned int stream_dma<B, T>::readMappedReg(uint32_t offset) {
  return 0;
}

template <int B, int T>
void stream_dma<B, T>::dma_mm2s_sync() {
#ifndef DISABLE_SIM
  sc_start();
#endif
}
template <int B, int T>
void stream_dma<B, T>::dma_s2mm_sync() {
#ifndef DISABLE_SIM
  sc_start();
#endif
}

template <int B, int T>
void stream_dma<B, T>::dma_change_start(int offset) {
  dmad->input_offset = offset / 4;
}

template <int B, int T>
void stream_dma<B, T>::dma_change_start(unsigned int addr, int offset) {
  dmad->input_offset = offset / 4;
}

template <int B, int T>
void stream_dma<B, T>::dma_change_end(int offset) {
  dmad->output_offset = offset / 4;
}

template <int B, int T>
void stream_dma<B, T>::initDMA(unsigned int src, unsigned int dst) {}

template <int B, int T>
void stream_dma<B, T>::dma_free() {
  free(input);
  free(output);
}

template <int B, int T>
int *stream_dma<B, T>::dma_get_inbuffer() {
  return input;
}

template <int B, int T>
int *stream_dma<B, T>::dma_get_outbuffer() {
  return output;
}

template <int B, int T>
void stream_dma<B, T>::dma_start_send(int length) {
  dmad->input_len = length * (B / 32);
  dmad->send = true;
#ifdef ACC_PROFILE
  data_transfered += length * (B / 8);
  data_send_count++;
#endif
}

template <int B, int T>
void stream_dma<B, T>::dma_wait_send() {
  prf_start(0);
  if (dmad->send) dma_mm2s_sync();
  prf_end(0, send_wait);
}

template <int B, int T>
int stream_dma<B, T>::dma_check_send() {
  return 0;
}

template <int B, int T>
void stream_dma<B, T>::dma_start_recv(int length) {
  dmad->output_len = length * (B / 32);
  dmad->recv = true;
}

template <int B, int T>
void stream_dma<B, T>::dma_wait_recv() {
  prf_start(0);
  if (dmad->recv) dma_s2mm_sync();
  prf_end(0, recv_wait);
#ifdef ACC_PROFILE
  data_transfered_recv += dmad->output_len * (B / 8);
  data_recv_count++;
#endif
}

template <int B, int T>
int stream_dma<B, T>::dma_check_recv() {
  return 0;
}

template <int B, int T>
void stream_dma<B, T>::print_times() {
#ifdef ACC_PROFILE
  cerr << "================================================" << endl;
  cerr << "-----------"
       << "DMA: " << id << "-----------" << endl;
  cerr << "Data Transfered: " << data_transfered << " bytes" << endl;
  cerr << "Data Transfered Recv: " << data_transfered_recv << " bytes" << endl;
  prf_out(TSCALE, send_wait);
  prf_out(TSCALE, recv_wait);
  float sendtime = (float)prf_count(TSCALE, send_wait) / 1000000;
  float recvtime = (float)prf_count(TSCALE, recv_wait) / 1000000;
  float data_transfered_MB = (float)data_transfered / 1000000;
  float data_recv_MB = (float)data_transfered_recv / 1000000;

  if (duration_cast<TSCALE>(send_wait).count() == 0) send_wait = nanoseconds(1);
  if (duration_cast<TSCALE>(recv_wait).count() == 0) recv_wait = nanoseconds(1);
  if (data_send_count == 0) data_send_count = 1;
  if (data_recv_count == 0) data_recv_count = 1;

  cerr << "Send speed: " << (data_transfered_MB / sendtime) << " MB/s" << endl;
  cerr << "Recv speed: " << (data_recv_MB / recvtime) << " MB/s" << endl;
  cerr << "Data Send Count: " << data_send_count << endl;
  cerr << "Data Recv Count: " << data_recv_count << endl;
  int data_per_send = data_transfered / data_send_count;
  int data_per_recv = data_transfered_recv / data_recv_count;
  cerr << "Data per Send: " << data_per_send << " bytes" << endl;
  cerr << "Data per Recv: " << data_per_recv << " bytes" << endl;
  // dmad->pattern_csv();
  cerr << "================================================" << endl;
#endif
}

// =========================== Multi DMAs
template <int B, int T>
multi_dma<B, T>::multi_dma(int _dma_count, unsigned int *_dma_addrs,
                           unsigned int *_dma_addrs_in,
                           unsigned int *_dma_addrs_out,
                           unsigned int _buffer_size) {
  dma_count = _dma_count;
  dmas = new stream_dma<B, T>[dma_count];
  dma_addrs = _dma_addrs;
  dma_addrs_in = _dma_addrs_in;
  dma_addrs_out = _dma_addrs_out;
  in_buffer_size = _buffer_size;
  out_buffer_size = _buffer_size;


  for (int i = 0; i < dma_count; i++)
    dmas[i].dma_init(dma_addrs[i], dma_addrs_in[i], in_buffer_size * 1,
                     dma_addrs_out[i], out_buffer_size * 1);
}

template <int B, int T>
multi_dma<B, T>::multi_dma(int _dma_count, unsigned int *_dma_addrs,
                           unsigned int *_dma_addrs_in,
                           unsigned int *_dma_addrs_out,
                           unsigned int _in_buffer_size,
                           unsigned int _out_buffer_size) {
  dma_count = _dma_count;
  dmas = new stream_dma<B, T>[dma_count];
  dma_addrs = _dma_addrs;
  dma_addrs_in = _dma_addrs_in;
  dma_addrs_out = _dma_addrs_out;
  in_buffer_size = _in_buffer_size;
  out_buffer_size = _out_buffer_size;

  for (int i = 0; i < dma_count; i++)
    dmas[i].dma_init(dma_addrs[i], dma_addrs_in[i], in_buffer_size * 1,
                     dma_addrs_out[i], out_buffer_size * 1);
}

template <int B, int T>
multi_dma<B, T>::~multi_dma() {
  print_times();
}

template <int B, int T>
void multi_dma<B, T>::multi_free_dmas() {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].dma_free();
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_change_start(int offset) {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].dma_change_start(offset);
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_change_start_4(int offset) {}

template <int B, int T>
void multi_dma<B, T>::multi_dma_change_end(int offset) {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].dma_change_end(offset);
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_start_send(int length) {
  for (int i = 0; i < dma_count; i++) dmas[i].dma_start_send(length);
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_wait_send() {
  bool loop = true;
  while (loop) {
    loop = false;
    for (int i = 0; i < dma_count; i++) {
      if (dmas[i].dmad->send) dmas[i].dma_wait_send();
      loop = loop || dmas[i].dmad->send;
    }
  }
}

template <int B, int T>
int multi_dma<B, T>::multi_dma_check_send() {
  return 0;
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_start_recv(int length) {
  for (int i = 0; i < dma_count; i++) dmas[i].dma_start_recv(length);
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_start_recv() {
  for (int i = 0; i < dma_count; i++)
    dmas[i].dma_start_recv(dmas[i].output_size);
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_wait_recv() {
  bool loop = true;
  while (loop) {
    loop = false;
    for (int i = 0; i < dma_count; i++) {
      bool s = dmas[i].dmad->recv;
      if (dmas[i].dmad->recv) dmas[i].dma_wait_recv();
      loop = loop || dmas[i].dmad->recv;
      bool e = dmas[i].dmad->recv;
      int k = 0;
    }
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_wait_recv_4() {}

template <int B, int T>
int multi_dma<B, T>::multi_dma_check_recv() {
  return 0;
}

template <int B, int T>
void multi_dma<B, T>::print_times() {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].print_times();
  }
}

#endif // SYSC


  // bool isNBitSignExtended(uint32_t data, const uint32_t &N) {
  //   int32_t shifted = static_cast<int32_t>(data << (32 - N)) >> (32 - N);
  //   uint32_t signed_data = static_cast<uint32_t>(shifted);
  //   return data == signed_data;
  // }

  // uint32_t compress(uint32_t data) {
  //   // // zero block
  //   // if (data == 0) {
  //   //   return zero_block_prefix;
  //   // }

  //   // Zero word
  //   if ((data & 0xFFFFFFFF) == 0) {
  //     return ZERO_WORD_PREFIX;
  //   }

  //   // Word with repeated bytes
  //   if ((data & 0xFF) == ((data >> 8) & 0xFF) &&
  //       (data & 0xFF) == ((data >> 16) & 0xFF) &&
  //       (data & 0xFF) == ((data >> 24) & 0xFF)) {
  //     return REPEATED_BYTE_PREFIX | ((data & 0xFF) << N_PREFIX_BITS);
  //   }

  //   // 4-bit sign-extended
  //   if (isNBitSignExtended(data, 4)) {
  //     uint32_t tmp = data & 0xF;
  //     uint32_t tmp2 = tmp << N_PREFIX_BITS;
  //     uint32_t tmp3 = SIGN_EXTENDED_4BIT_PREFIX | tmp2;
  //     return SIGN_EXTENDED_4BIT_PREFIX | ((data & 0xF) << N_PREFIX_BITS);
  //   }

  //   // One byte sign-extended
  //   if (isNBitSignExtended(data, 8)) {
  //     return ONE_BYTE_SIGN_EXTENDED_PREFIX | ((data & 0xFF) << N_PREFIX_BITS);
  //   }

  //   // Halfword sign-extended
  //   if (isNBitSignExtended(data, 16)) {
  //     return HALFWORD_SIGN_EXTENDED_PREFIX | ((data & 0xFFFF) << N_PREFIX_BITS);
  //   }

  //   // Halfword padded with zero
  //   if ((data & 0xFFFF) == 0) {
  //     return HALFWORD_PADDED_ZERO_PREFIX | ((data >> 16) << N_PREFIX_BITS);
  //   }

  //   // Two halfwords, each a byte sign-extended
  //   uint32_t upper = (data >> 16);
  //   uint32_t lower = (data & 0x00FF);
  //   if (isNBitSignExtended(upper, 8) && isNBitSignExtended(lower, 8)) {
  //     return TWO_HALFWORDS_BYTE_SIGN_EXTENDED_PREFIX |
  //            (((upper << 8) | ((lower))) << N_PREFIX_BITS);
  //   }

  //   // Uncompressed
  //   return data;
  // }

  /// Prefix | Pattern# | Encoded Pattern            | Original Data | Compressed Data | Total Data Size (data + metadata)
/// -------|----------|----------------------------|---------------|-----------------|-----------------------------------
/// ---    | 1        | Zero block                 | Z512          | -               |  0 + 3 bits
/// 001    | 2        | Zero word                  | Z32           | -    +M3        |  0 + 3 bits
/// 010    | 3        | Word with repeated bytes   | N8N8N8N8      | N8   +M3        |  8 + 3 bits
/// 011    | 4        | 4-bit sign-extended        | X29N3         | N4   +M3        |  4 + 3 bits
/// 100    | 5        | One byte sign-extended     | X25N7         | N8   +M3        |  8 + 3 bits
/// 101    | 6        | Halfword sign-extended     | X17N15        | N16  +M3        | 16 + 3 bits
/// 110    | 7        | Halfword padded with zero  | N16Z16        | N16  +M3        | 16 + 3 bits
/// 111    | 8        | Two halfw., each sign-ext. | X8N8X8N8      | N8N8 +M3        | 16 + 3 bits
/// ---    | 9        | Uncompressed               | N32           | N32  +M3        | 32 + 0 bits

// Z16N16
// N8X8N8X8