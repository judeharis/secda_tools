#ifdef SYSC

// ================================================================================
// Stream DMA API
// ================================================================================

template <int B, int T>
int stream_dma<B, T>::s_id = 0;

template <int B, int T>
stream_dma<B, T>::stream_dma(unsigned int _dma_addr, bool _sg_mode)
    : id(s_id++) {
  string name("SDMA" + to_string(id));
  dmad = new AXIS_ENGINE<B, AXI_TYPE>(&name[0]);
  dmad->id = id;
  dmad->input_len = 0;
  dmad->input_offset = 0;
  dmad->output_len = 0;
  dmad->output_offset = 0;
  dma_init(_dma_addr, _sg_mode);
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
stream_dma<B, T>::~stream_dma() {
  print_times();
  dma_free();
  delete dmad;
}

template <int B, int T>
void stream_dma<B, T>::dma_init(unsigned int _dma_addr, bool _sg_mode) {
  // Currently not using _sg_mode, but can be added later if needed
  // Used to initialize the DMA engine with the given address and mode
  // SystemC version does not require this initialization, but it is kept for
  // compatibility with other versions
}

template <int B, int T>
void stream_dma<B, T>::dma_bind_input_buffer(
    mm_buffer<int> *_input_buffer) {
  input_buffer = _input_buffer;
  dmad->DMA_input_buffer = (int *)input_buffer->buffer;
  input_size = input_buffer->size;
}

template <int B, int T>
void stream_dma<B, T>::dma_bind_output_buffer(
    mm_buffer<int> *_output_buffer) {
  output_buffer = _output_buffer;
  dmad->DMA_output_buffer = (int *)output_buffer->buffer;
  output_size = output_buffer->size;
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
void stream_dma<B, T>::initDMA(unsigned int src, unsigned int dst,
                               bool sg_mode) {}

template <int B, int T>
void stream_dma<B, T>::dma_free() {}

template <int B, int T>
int *stream_dma<B, T>::dma_get_inbuffer() {
  return (int *)input_buffer->buffer;
}

template <int B, int T>
int *stream_dma<B, T>::dma_get_outbuffer() {
  return (int *)output_buffer->buffer;
}

template <int B, int T>
void stream_dma<B, T>::dma_start_send(int length) {
  dmad->input_len = length * (B / 32);
  dmad->send = true;
#ifdef DMA_PROFILE
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
#ifdef DMA_PROFILE
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
#ifdef DMA_PROFILE
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

// ================================================================================
// Multi Stream DMA API
// ================================================================================
template <int B, int T>
multi_dma<B, T>::multi_dma(int _dma_count, unsigned int *_dma_addrs,
                           bool _sg_mode) {
  dma_count = _dma_count;
  dmas = new stream_dma<B, T>[dma_count];
  dma_addrs = _dma_addrs;
  for (int i = 0; i < dma_count; i++) dmas[i].dma_init(dma_addrs[i], _sg_mode);
}

template <int B, int T>
multi_dma<B, T>::~multi_dma() {
  print_times();
}

template <int B, int T>
void multi_dma<B, T>::multi_free_dmas() {
  for (int i = 0; i < dma_count; i++) dmas[i].dma_free();
}

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
      if (dmas[i].dmad->recv) dmas[i].dma_wait_recv();
      loop = loop || dmas[i].dmad->recv;
    }
  }
}

template <int B, int T>
void multi_dma<B, T>::multi_dma_wait_recv_4() {
  multi_dma_wait_recv();
}

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
#endif