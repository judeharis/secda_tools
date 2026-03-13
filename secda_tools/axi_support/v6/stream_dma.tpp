#ifndef SYSC

// ================================================================================
// Stream DMA API
// ================================================================================
template <int B, int T>
int stream_dma<B, T>::s_id = 0;

template <int B, int T>
int stream_dma<B, T>::ubuf_id = 0;

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
                                unsigned int _output_size, bool _sg_mode) {
  sg_mode = _sg_mode;
  dma_addr = mm_alloc_rw<unsigned int>(_dma_addr, PAGE_SIZE);
  if (sg_mode) {
    if (_input_size < MAX_DESC * DESC_SIZE ||
        _output_size < MAX_DESC * DESC_SIZE) {
      cerr << "Error: not enough buffer size for SG mode. Minimum size: "
           << MAX_DESC * DESC_SIZE << " bytes" << endl;
      exit(EXIT_FAILURE);
    }
    mm2s_bd_phy_addr = _input;
    s2mm_bd_phy_addr = _output;
    input_addr = _input + (MAX_DESC * DESC_SIZE);
    output_addr = _output + (MAX_DESC * DESC_SIZE);
    input_size = _input_size - (MAX_DESC * DESC_SIZE);
    output_size = _output_size - (MAX_DESC * DESC_SIZE);
#ifdef KRIA
    cerr << "KRIA ALLOC" << endl;
    mm2s_bd_addr = mm_alloc_rw<int>(_input, MAX_DESC * DESC_SIZE);
    s2mm_bd_addr = mm_alloc_rw<int>(_output, MAX_DESC * DESC_SIZE);
    input = mm_alloc_rw<int>(input_addr, input_size);
    output = mm_alloc_rw<int>(output_addr, output_size);

#else
    cout << "CMA ALLOC" << endl;
    mm2s_bd_addr = cmap_alloc_rw<int>(MAX_DESC * DESC_SIZE);
    s2mm_bd_addr = cmap_alloc_rw<int>(MAX_DESC * DESC_SIZE);
    input = cmap_alloc_rw<int>(input_size);
    output = cmap_alloc_rw<int>(output_size);
    int *mm2s_bd_buf = reinterpret_cast<int *>(mm2s_bd_addr);
    int *s2mm_bd_buf = reinterpret_cast<int *>(s2mm_bd_addr);
    int *input_buf = reinterpret_cast<int *>(input);
    int *output_buf = reinterpret_cast<int *>(output);

    mm2s_bd_phy_addr = cma_get_phy_addr(mm2s_bd_buf);
    s2mm_bd_phy_addr = cma_get_phy_addr(s2mm_bd_buf);
    input_addr = cma_get_phy_addr(input_buf);
    output_addr = cma_get_phy_addr(output_buf);
#endif

    initDMA(mm2s_bd_phy_addr, s2mm_bd_phy_addr, true);
    cerr << "DMA " << id << " | input_addr: " << HEX(input_addr)
         << " size: " << _input_size << " | output_addr: " << HEX(output_addr)
         << " size: " << _output_size << endl;

  } else {

#ifdef KRIA
    cerr << "KRIA ALLOC NO SG" << endl;
    // input = mm_alloc_rw<int>(_input, _input_size);
    // output = mm_alloc_r<int>(_output, _output_size);
    // input_size = _input_size;
    // output_size = _output_size;
    // input_addr = _input;
    // output_addr = _output;

    ubuf_id_in = ubuf_id++;
    ubuf_id_out = ubuf_id++;
    input = ubuf_mm_alloc_rw<int>(_input, _input_size, ubuf_id_in);
    output = ubuf_mm_alloc_rw<int>(_output, _output_size, ubuf_id_out);
    input_size = _input_size;
    output_size = _output_size;
    input_addr = ubuf_get_phy_addr(ubuf_id_in);
    output_addr = ubuf_get_phy_addr(ubuf_id_out);

#else
    cout << "CMA ALLOC NO SG" << endl;
    input = cmap_alloc_rw<int>(_input_size);
    output = cmap_alloc_rw<int>(_output_size);
    int *input_buf = reinterpret_cast<int *>(input);
    int *output_buf = reinterpret_cast<int *>(output);
    input_addr = cma_get_phy_addr(input_buf);
    output_addr = cma_get_phy_addr(output_buf);
#endif
    initDMA(input_addr, output_addr);
    cerr << "DMA " << id << " | input_addr: " << HEX(input_addr)
         << " size: " << _input_size << " | output_addr: " << HEX(output_addr)
         << " size: " << _output_size << endl;
  }
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

// template <int B, int T>
// void stream_dma<B, T>::initDMA(unsigned int src, unsigned int dst) {
//   writeMappedReg(S2MM_CONTROL_REGISTER, 4);
//   writeMappedReg(MM2S_CONTROL_REGISTER, 4);
//   writeMappedReg(S2MM_CONTROL_REGISTER, 0);
//   writeMappedReg(MM2S_CONTROL_REGISTER, 0);
//   writeMappedReg(S2MM_DESTINATION_ADDRESS, dst);
//   writeMappedReg(MM2S_START_ADDRESS, src);
//   writeMappedReg(S2MM_CONTROL_REGISTER, 0xf001);
//   writeMappedReg(MM2S_CONTROL_REGISTER, 0xf001);
// }

template <int B, int T>
void stream_dma<B, T>::initDMA(unsigned int src, unsigned int dst,
                               bool sg_mode) {
  if (sg_mode) {
    writeMappedReg(S2MM_CONTROL_REGISTER, 4);
    writeMappedReg(MM2S_CONTROL_REGISTER, 4);
    writeMappedReg(S2MM_CONTROL_REGISTER, 0);
    writeMappedReg(MM2S_CONTROL_REGISTER, 0);
    writeMappedReg(S2MM_CURDESC, dst);
    writeMappedReg(MM2S_CURDESC, src);
    writeMappedReg(S2MM_CONTROL_REGISTER, 0xf001);
    writeMappedReg(MM2S_CONTROL_REGISTER, 0xf001);
    mm2s_curdesc_phy_addr = src;
    mm2s_curdesc_offset = 0;
    s2mm_curdesc_phy_addr = dst;
    s2mm_curdesc_offset = 0;
  } else {
    writeMappedReg(S2MM_CONTROL_REGISTER, 4);
    writeMappedReg(MM2S_CONTROL_REGISTER, 4);
    writeMappedReg(S2MM_CONTROL_REGISTER, 0);
    writeMappedReg(MM2S_CONTROL_REGISTER, 0);
    writeMappedReg(S2MM_DESTINATION_ADDRESS, dst);
    writeMappedReg(MM2S_START_ADDRESS, src);
    writeMappedReg(S2MM_CONTROL_REGISTER, 0xf001);
    writeMappedReg(MM2S_CONTROL_REGISTER, 0xf001);
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_free() {
  cout << "DMA: " << id << " freed " << endl;
  print_times();
  cma_munmap(dma_addr, PAGE_SIZE);
#ifdef KRIA
  munmap(input, input_size);
  munmap(output, output_size);
  if (ubuf_id_in >= 0) ubuf_free(ubuf_id_in);
  if (ubuf_id_out >= 0) ubuf_free(ubuf_id_out);
#else
  cma_free(input);
  cma_free(output);
#endif
  // munlockall();
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
  if (sg_mode) {
    mm2s_curdesc_offset = 0;
    mm2s_curdesc_phy_addr = mm2s_bd_phy_addr;
  }
  prf_end_x(0, send_start, send_wait);
#endif
}

template <int B, int T>
int stream_dma<B, T>::dma_check_send() {
  unsigned int mm2s_status = readMappedReg(MM2S_STATUS_REGISTER);
  bool done = !((!(mm2s_status & 1 << 12)) || (!(mm2s_status & 1 << 1)));
  if (done && sg_mode) {
    mm2s_curdesc_offset = 0;
    mm2s_curdesc_phy_addr = mm2s_bd_phy_addr;
  }
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
  if (sg_mode) {
    s2mm_curdesc_offset = 0;
    s2mm_curdesc_phy_addr = s2mm_bd_phy_addr;
  }
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
  if (done && sg_mode) {
    s2mm_curdesc_offset = 0;
    s2mm_curdesc_phy_addr = s2mm_bd_phy_addr;
  }
  return done ? 0 : -1;
}

template <int B, int T>
void stream_dma<B, T>::dma_sg_mm2s_setup(unsigned int dst, unsigned int length,
                                         bool last) {

  unsigned next = mm2s_curdesc_offset + 0x40;
  if (last) next = 0x00;
  mm2s_bd_addr[(mm2s_curdesc_offset + 0x0) >> 2] = next;
  mm2s_bd_addr[(mm2s_curdesc_offset + 0x8) >> 2] = dst;
  mm2s_bd_addr[(mm2s_curdesc_offset + 0x18) >> 2] = length * (B / 8);
  if (!last) {
    mm2s_curdesc_offset += 0x40;
    mm2s_curdesc_phy_addr += 0x40;
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_sg_s2mm_setup(unsigned int src, unsigned int length,
                                         bool last) {
  unsigned next = s2mm_curdesc_offset + 0x40;
  if (last) next = 0x00;
  s2mm_bd_addr[(s2mm_curdesc_offset + 0x0) >> 2] = next;
  s2mm_bd_addr[(s2mm_curdesc_offset + 0x8) >> 2] = src;
  s2mm_bd_addr[(s2mm_curdesc_offset + 0x18) >> 2] = length * (B / 8);
  if (!last) {
    s2mm_curdesc_offset += 0x40;
    s2mm_curdesc_phy_addr += 0x40;
  }
}

template <int B, int T>
void stream_dma<B, T>::dma_sg_start_send() {
  writeMappedReg(MM2S_CURDESC, mm2s_curdesc_phy_addr);
}

template <int B, int T>
void stream_dma<B, T>::dma_sg_start_recv() {
  writeMappedReg(S2MM_CURDESC, s2mm_curdesc_phy_addr);
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
// Multi Stream DMA API
// ================================================================================

template <int B, int T>
multi_dma<B, T>::multi_dma(int _dma_count, unsigned int *_dma_addrs,
                           unsigned int *_dma_addrs_in,
                           unsigned int *_dma_addrs_out,
                           unsigned int _in_buffer_size,
                           unsigned int _out_buffer_size, bool _sg_mode) {
  dma_count = _dma_count;
  dmas = new stream_dma<B, T>[dma_count];
  dma_addrs = _dma_addrs;
  dma_addrs_in = _dma_addrs_in;
  dma_addrs_out = _dma_addrs_out;
  in_buffer_size = _in_buffer_size;
  out_buffer_size = _out_buffer_size;

  for (int i = 0; i < dma_count; i++)
    dmas[i].dma_init(dma_addrs[i], dma_addrs_in[i], in_buffer_size * 1,
                     dma_addrs_out[i], out_buffer_size * 1, _sg_mode);
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