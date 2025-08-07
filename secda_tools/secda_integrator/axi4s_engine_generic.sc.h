#ifndef AXIS_ENGINE_H2
#define AXIS_ENGINE_H2

#include "sysc_types.h"
#include <iostream>
#include <type_traits>
#include <typeinfo>

template <int B, template <int> class T>
SC_MODULE(AXIS_ENGINE) {
  sc_in<bool> clock;
  sc_in<bool> reset;
  sc_fifo_in<_BDATA<B, T>> dout1;
  sc_fifo_out<_BDATA<B, T>> din1;
  bool send = false;
  bool recv = false;
  int id = 0;

  int r_paddr = 0;
  int w_paddr = 0;

  void DMA_MMS2() {
    int initial_free = din1.num_free();
    while (1) {
      while (!send) wait();
      int packets_sent = 0;
      for (int i = 0; i < input_len;) {
        T<B> dO;
        for (int j = 0; j < B / 32; j++) {
          int d = DMA_input_buffer[(i++) + input_offset * (B / 32)];
          dO.range((j + 1) * 32 - 1, j * 32) = d;
          sc_uint<32> data = dO.range((j + 1) * 32 - 1, j * 32);
        }
        din1.write({dO, 1});
        packets_sent++;
        wait();
      }
      while (initial_free != din1.num_free()) wait();
      send = false;
      sc_pause();
      wait();
    }
  };

  void DMA_S2MM() {
    while (1) {
      while (!recv) wait();
      bool last = false;
      int i = 0;
      int packets_recv = 0;
      do {
        _BDATA<B, T> d = dout1.read();
        while (i >= output_len) wait();
        packets_recv++;
        last = d.tlast;
        for (int j = 0; j < B / 32; j++) {
          DMA_output_buffer[(i++) + output_offset * (B / 32)] =
              d.data.range((j + 1) * 32 - 1, j * 32).to_int();
        }
        wait();
      } while (!last);
      output_len = packets_recv;
      recv = false;
      // // To ensure wait_send() does not evoke the sc_pause
      // while (send)
      //   wait(2);
      wait();
      sc_pause();
      wait();
    }
  };

  SC_HAS_PROCESS(AXIS_ENGINE);

  AXIS_ENGINE(sc_module_name name_) : sc_module(name_) {
    SC_CTHREAD(DMA_MMS2, clock.pos());
    reset_signal_is(reset, true);

    SC_CTHREAD(DMA_S2MM, clock.pos());
    reset_signal_is(reset, true);
  }

  int *DMA_input_buffer;
  int *DMA_output_buffer;

  int input_len;
  int input_offset;

  int output_len;
  int output_offset;
};

#endif