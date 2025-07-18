#ifndef SYSC

// ================================================================================
// AXI4Lite API
// ================================================================================

template <typename T>
acc_regmap<T>::acc_regmap(size_t base_addr, size_t length) {
  acc_addr = getAccBaseAddress<int>(base_addr, length);
}

template <typename T>
void acc_regmap<T>::writeAccReg(uint32_t offset, unsigned int val) {
  void *base_addr = (void *)acc_addr;
  *((volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset)) =
      val;
}

template <typename T>
unsigned int acc_regmap<T>::readAccReg(uint32_t offset) {
  void *base_addr = (void *)acc_addr;
  return *(
      (volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset));
}

// TODO: parse JSON file to load offset map for control and status registers
template <typename T>
void acc_regmap<T>::parseOffsetJSON() {}

// TODO: checks control and status register arrays to find the offsets for the
// register
template <typename T>
uint32_t acc_regmap<T>::findRegOffset(string reg_name) {
  uint32_t offset = 0;
  return offset;
}

template <typename T>
void acc_regmap<T>::writeToControlReg(string reg_name, unsigned int val) {
  writeAccReg(findRegOffset(reg_name), val);
}

template <typename T>
unsigned int acc_regmap<T>::readToControlReg(string reg_name) {
  return readAccReg(findRegOffset(reg_name));
}

// ================================================================================
// Memory Map API
// ================================================================================

// Make this into a struct based API

// ================================================================================
// AXIMM API
// ================================================================================
template <typename T>
int mm_buffer<T>::mm_id = 0;

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

// ================================================================================
// ACC Control API
// ================================================================================

template <typename T>
axi4lite_ctrl<T>::axi4lite_ctrl() {}

template <typename T>
axi4lite_ctrl<T>::axi4lite_ctrl(int *base_addr) {
  reg_base = base_addr;
}

template <typename T>
unsigned int axi4lite_ctrl<T>::read_reg(unsigned int offset) {
  void *base_addr = (void *)reg_base;
  return *(
      (volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset));
}

template <typename T>
void axi4lite_ctrl<T>::write_reg(unsigned int offset, unsigned int val) {
  void *base_addr = (void *)reg_base;
  *((volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset)) =
      val;
}

template <typename T>
acc_ctrl<T>::acc_ctrl() {}

template <typename T>
void acc_ctrl<T>::start_acc() {
  write_reg(0x14, 1);
}

template <typename T>
void acc_ctrl<T>::wait_done() {
  while (!read_reg(0x1C)) {
    msync(reg_base, PAGE_SIZE, MS_SYNC);
  }
  write_reg(0x14, 0);
  while (read_reg(0x1C)) {
    msync(reg_base, PAGE_SIZE, MS_SYNC);
  }
}

template <typename T>
hwc_ctrl<T>::hwc_ctrl(){};

template <typename T>
void hwc_ctrl<T>::init_hwc(int count) {hwc_count = count;}

template <typename T>
void hwc_ctrl<T>::reset_hwc() {
  write_reg(0x14, 1); // Reset HWC
  msync(reg_base, PAGE_SIZE, MS_SYNC);
  write_reg(0x14, 0); // Clear reset
}

template <typename T>
void hwc_ctrl<T>::set_target_state(int hwc, int target_state) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
    return;
  }
  write_reg(0x1C + (hwc * 0x18), target_state);
}

template <typename T>
unsigned int hwc_ctrl<T>::get_current_state(int hwc) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
    return -1;
  }
  return read_reg(0x20 + (hwc * 0x18));
}

template <typename T>
unsigned int hwc_ctrl<T>::get_cycle_count(int hwc) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
    return -1;
  }
  return read_reg(0x24 + (hwc * 0x18));
}

// ================================================================================
// Stream DMA API
// ================================================================================
template <int B, int T>
int stream_dma<B, T>::s_id = 0;
// int sr_id = 0;

template <int B, int T>
stream_dma<B, T>::stream_dma(unsigned int _dma_addr, unsigned int _input,
                             unsigned int _input_size, unsigned int _output,
                             unsigned int _output_size)
    : id(s_id++) {
  dma_init(_dma_addr, _input, _input_size, _output, _output_size);
}

template <int B, int T>
stream_dma<B, T>::stream_dma() : id(s_id++){};

template <int B, int T>
stream_dma<B, T>::~stream_dma() {
  // dma_free();
}

template <int B, int T>
void stream_dma<B, T>::dma_init(unsigned int _dma_addr, unsigned int _input,
                                unsigned int _input_size, unsigned int _output,
                                unsigned int _output_size) {
  dma_addr = mm_alloc_rw<unsigned int>(_dma_addr, PAGE_SIZE);

  // #ifdef KRIA
  cerr << "KRIA ALLOC" << endl;
  input = mm_alloc_rw<int>(_input, _input_size);
  output = mm_alloc_r<int>(_output, _output_size);
  input_size = _input_size;
  output_size = _output_size;
  input_addr = _input;
  output_addr = _output;
  // #else
  //   cout << "CMA ALLOC" << endl;
  //   input = cmap_alloc_rw<int>(_input_size);
  //   output = cmap_alloc_rw<int>(_output_size);
  //   int *input_buf = reinterpret_cast<int *>(input);
  //   int *output_buf = reinterpret_cast<int *>(output);
  //   input_addr = cma_get_phy_addr(input_buf);
  //   output_addr = cma_get_phy_addr(output_buf);
  // #endif

  initDMA(input_addr, output_addr);
  cerr << "DMA " << id << " | input_addr: " << HEX(input_addr)
       << " size: " << _input_size << " | output_addr: " << HEX(output_addr)
       << " size: " << _output_size << endl;
}

template <int B, int T>
void stream_dma<B, T>::writeMappedReg(uint32_t offset, unsigned int val) {
  void *base_addr = (void *)dma_addr;
  *((volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset)) =
      val;
}

template <int B, int T>
unsigned int stream_dma<B, T>::readMappedReg(uint32_t offset) {
  void *base_addr = (void *)dma_addr;
  return *(
      (volatile unsigned int *)(reinterpret_cast<char *>(base_addr) + offset));
}

template <int B, int T>
void stream_dma<B, T>::dma_mm2s_sync() {
  msync(dma_addr, PAGE_SIZE, MS_SYNC);
  unsigned int mm2s_status = readMappedReg(MM2S_STATUS_REGISTER);
  while (!(mm2s_status & 1 << 12) || !(mm2s_status & 1 << 1)) {
    msync(dma_addr, PAGE_SIZE, MS_SYNC);
    mm2s_status = readMappedReg(MM2S_STATUS_REGISTER);
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_s2mm_sync() {
  msync(dma_addr, PAGE_SIZE, MS_SYNC);
  unsigned int s2mm_status = readMappedReg(S2MM_STATUS_REGISTER);
  while (!(s2mm_status & 1 << 12) || !(s2mm_status & 1 << 1)) {
    msync(dma_addr, PAGE_SIZE, MS_SYNC);
    s2mm_status = readMappedReg(S2MM_STATUS_REGISTER);
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_change_start(int offset) {
  writeMappedReg(MM2S_START_ADDRESS, input_addr + offset);
}

template <int B, int T>
void stream_dma<B, T>::dma_change_start(unsigned int addr, int offset) {
  writeMappedReg(MM2S_START_ADDRESS, addr + offset);
}

template <int B, int T>
void stream_dma<B, T>::dma_change_end(int offset) {
  writeMappedReg(S2MM_DESTINATION_ADDRESS, output_addr + offset);
}

template <int B, int T>
void stream_dma<B, T>::initDMA(unsigned int src, unsigned int dst) {
  writeMappedReg(S2MM_CONTROL_REGISTER, 4);
  writeMappedReg(MM2S_CONTROL_REGISTER, 4);
  writeMappedReg(S2MM_CONTROL_REGISTER, 0);
  writeMappedReg(MM2S_CONTROL_REGISTER, 0);
  writeMappedReg(S2MM_DESTINATION_ADDRESS, dst);
  writeMappedReg(MM2S_START_ADDRESS, src);
  writeMappedReg(S2MM_CONTROL_REGISTER, 0xf001);
  writeMappedReg(MM2S_CONTROL_REGISTER, 0xf001);
}

template <int B, int T>
void stream_dma<B, T>::dma_free() {
  cout << "DMA: " << id << " freed " << endl;
  print_times();
  cma_free(input);
  cma_free(output);
  cma_munmap(dma_addr, PAGE_SIZE);
  // munlockall();
}

template <int B, int T>
int *stream_dma<B, T>::dma_get_inbuffer() {
  return input;
}

// template <int B, int T>
// vector<cma_buffer> stream_dma<B, T>::dma_get_inbuffers() {
//   return input_bufs;
// }

template <int B, int T>
int *stream_dma<B, T>::dma_get_outbuffer() {
  return output;
}

template <int B, int T>
void stream_dma<B, T>::dma_start_send(int length) {
#ifndef DISABLE_DMA
  prf_start_x(send_start);
#ifdef ACC_PROFILE
  data_transfered += length * (B / 8);
  data_send_count++;
#endif
  msync(input, input_size, MS_SYNC | MS_INVALIDATE);
  writeMappedReg(MM2S_LENGTH, length * (B / 8));
#endif
}

template <int B, int T>
void stream_dma<B, T>::dma_wait_send() {
#ifndef DISABLE_DMA
  dma_mm2s_sync();
  prf_end_x(0, send_start, send_wait);
#endif
}

template <int B, int T>
int stream_dma<B, T>::dma_check_send() {
  unsigned int mm2s_status = readMappedReg(MM2S_STATUS_REGISTER);
  bool done = !((!(mm2s_status & 1 << 12)) || (!(mm2s_status & 1 << 1)));
  return done ? 0 : -1;
}

template <int B, int T>
void stream_dma<B, T>::dma_start_recv(int length) {
#ifndef DISABLE_DMA
  writeMappedReg(S2MM_LENGTH, length * (B / 8));
#endif
}

template <int B, int T>
void stream_dma<B, T>::dma_wait_recv() {
#ifndef DISABLE_DMA
  // prf_start(0);
  dma_s2mm_sync();
  msync(output, output_size, MS_SYNC);
  // prf_end(0, recv_wait);
  prf_end_x(1, send_start, recv_wait);
#ifdef ACC_PROFILE
  data_transfered_recv += readMappedReg(S2MM_LENGTH);
  data_recv_count++;
#endif
#endif
}

template <int B, int T>
int stream_dma<B, T>::dma_check_recv() {
  unsigned int s2mm_status = readMappedReg(S2MM_STATUS_REGISTER);
  bool done = !((!(s2mm_status & 1 << 12)) || (!(s2mm_status & 1 << 1)));
  return done ? 0 : -1;
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
  cerr << "================================================" << endl;
  std::ofstream file("dma" + std::to_string(id) + ".csv", std::ios::out);
  // csv file header
  file << "Data Transfered,Data Transfered Recv,Send Time,Recv Time,Send "
          "Speed,Recv Speed,Data Send Count,Data Recv Count,Data per Send,Data "
          "per Recv"
       << std::endl;
  file << data_transfered << "," << data_transfered_recv << "," << sendtime
       << "," << recvtime << "," << (data_transfered_MB / sendtime) << ","
       << (data_recv_MB / recvtime) << "," << data_send_count << ","
       << data_recv_count << "," << data_per_send << "," << data_per_recv
       << std::endl;

#endif
}

// ================================================================================

template <int B, int T>
int stream_dma<B, T>::dma_alloc_input_buffer(int buffer_size) {
  void *input = cmap_alloc_rw<void *>(buffer_size);
  unsigned int phy_addr = cma_get_phy_addr(input);
  if (input == NULL) {
    cerr << "Failed to allocate DMA Buffer" << endl;
    return -1;
  }
  // std::stringstream ss;
  // ss << std::hex << phy_addr;
  // std::string res();
  // cout << "Allocated DMA Buffer " << cma_id << " with size " << buffer_size
  //  << " at address " << ss.str() << endl;

  cout << "Allocated DMA Buffer " << cma_id << " with size " << buffer_size
       << " at address " << HEX(phy_addr) << endl;

  input_bufs_id.push_back(cma_id);
  input_bufs_ptrs.push_back(input);
  input_bufs_phy_addr.push_back(phy_addr);
  input_bufs_size.push_back(buffer_size);
  input_bufs_allocated_size += buffer_size;
  return cma_id++;
}

template <int B, int T>
void stream_dma<B, T>::dma_dealloc_input_buffer(int id) {
  for (int i = 0; i < input_bufs_id.size(); i++) {
    if (input_bufs_id[i] == id) {
      cma_free(input_bufs_ptrs[i]);
      input_bufs_allocated_size -= input_bufs_size[i];
      input_bufs_id.erase(input_bufs_id.begin() + i);
      input_bufs_ptrs.erase(input_bufs_ptrs.begin() + i);
      input_bufs_phy_addr.erase(input_bufs_phy_addr.begin() + i);
      input_bufs_size.erase(input_bufs_size.begin() + i);
      break;
    }
  }
}

template <int B, int T>
int *stream_dma<B, T>::dma_get_input_buffer(int id) {
  for (int i = 0; i < input_bufs_id.size(); i++) {
    if (input_bufs_id[i] == id) {
      return (int *)input_bufs_ptrs[i];
    }
  }
  cout << "Input buffer not found" << endl;
  return NULL;
}

template <int B, int T>
void stream_dma<B, T>::dma_change_input_buffer(int id, int offset) {
  for (int i = 0; i < input_bufs_id.size(); i++) {
    if (input_bufs_id[i] == id) {
      // unsigned phy_addr = input_bufs_phy_addr[i];
      unsigned phy_addr = cma_get_phy_addr(input_bufs_ptrs[i]);
      dma_change_start(phy_addr, offset);
    }
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_default_input_buffer() {
  dma_change_start(0);
}

template <int B, int T>
void stream_dma<B, T>::dma_send_buffer(int id, int length, int offset) {
  dma_change_input_buffer(id, offset);
  dma_start_send(length);
}

template <int B, int T>
unsigned int stream_dma<B, T>::dma_pages_available() {
  return cma_pages_available();
}

// ================================================================================

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
  for (int i = 0; i < dma_count; i++) {
    dmas[i].dma_free();
  }
  delete[] dmas;
}

template <int B, int T>
void multi_dma<B, T>::multi_free_dmas() {}

template <int B, int T>
void multi_dma<B, T>::multi_dma_change_start(int offset) {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].dma_change_start(offset);
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_change_start_4(int offset) {
  dmas[0].dma_change_start(offset);
  dmas[1].dma_change_start(offset);
  dmas[2].dma_change_start(offset);
  dmas[3].dma_change_start(offset);
}

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
  for (int i = 0; i < dma_count; i++) dmas[i].dma_wait_send();
}

template <int B, int T>
int multi_dma<B, T>::multi_dma_check_send() {
  bool done = true;
  for (int i = 0; i < dma_count; i++)
    done = done && (dmas[i].dma_check_send() == 0);
  return done ? 0 : -1;
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
  for (int i = 0; i < dma_count; i++) dmas[i].dma_wait_recv();
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_wait_recv_4() {
  dmas[0].dma_wait_recv();
  dmas[1].dma_wait_recv();
  dmas[2].dma_wait_recv();
  dmas[3].dma_wait_recv();
}

template <int B, int T>
int multi_dma<B, T>::multi_dma_check_recv() {
  bool done = true;
  for (int i = 0; i < dma_count; i++)
    done = done && (dmas[i].dma_check_recv() == 0);
  return done ? 0 : -1;
}

template <int B, int T>
void multi_dma<B, T>::print_times() {
  for (int i = 0; i < dma_count; i++) {
    dmas[i].print_times();
  }
}

#endif