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
  sc_fifo_in<BDATA<B, T>> dout1;
  sc_fifo_out<BDATA<B, T>> din1;
  bool send = false;
  bool recv = false;
  int id = 0;

  int r_paddr = 0;
  int w_paddr = 0;

  // unsigned int pattern_count[9] = {};

  // void pattern_csv () {
  //   std::cerr << "Pattern count: " << std::to_string(id) << "\n";
  //   for (int i = 0; i < 9; i++) {
  //     std::cerr <<  "p" << i << "," << pattern_count[i] << "\n";
  //   }
  // }

  // bool isNBitSignExtended(uint32_t data, const uint32_t &N) {
  //   int32_t shifted = static_cast<int32_t>(data << (32 - N)) >> (32 - N);
  //   uint32_t signed_data = static_cast<uint32_t>(shifted);
  //   return data == signed_data;
  // }

  // uint32_t pattern_check(sc_uint<32> data) {
  //   sc_uint<8> byte0 = data.range(7, 0);
  //   sc_uint<8> byte1 = data.range(15, 8);
  //   sc_uint<8> byte2 = data.range(23, 16);
  //   sc_uint<8> byte3 = data.range(31, 24);
  //   sc_uint<16> half_word0 = data.range(15, 0);
  //   sc_uint<16> half_word1 = data.range(31, 16);

  //   sc_uint<29> x29 = data.range(28, 0);
  //   sc_uint<3> n3 = data.range(31, 29);

  //   sc_uint<17> x17 = data.range(16, 0);
  //   sc_uint<15> n15 = data.range(31, 17);

  //   uint32_t udata = data;
  //   uint32_t ubyte0 = byte0;
  //   uint32_t ubyte1 = byte1;
  //   uint32_t ubyte2 = byte2;
  //   uint32_t ubyte3 = byte3;

  //   uint32_t uhalf_word0 = half_word0;
  //   uint32_t uhalf_word1 = half_word1;

  //   // std::cout << "Pattern count: " << std::to_string(id) << " ";

  //   if (data.range(31, 0) == 0) {
  //     return 0;
  //   } else if (byte0 == byte1 && byte2 == byte3 && byte0 == byte2) {
  //     return 1;
  //   } else if (isNBitSignExtended(udata, 4)) {
  //     return 2;
  //   } else if (isNBitSignExtended(udata, 8)) {
  //     return 3;
  //   } else if (isNBitSignExtended(udata, 16)) {
  //     return 4;
  //   } else if (uhalf_word0 == 0) {
  //     return 5;
  //   } else if (isNBitSignExtended(uhalf_word1, 8) && isNBitSignExtended(uhalf_word0, 8)) {
  //     return 6;
  //   }
  //   return 7;
  // };

  void DMA_MMS2() {
    int initial_free = din1.num_free();
    while (1) {
      while (!send) wait();
      int packets_sent = 0;
      for (int i = 0; i < input_len;) {
        // int d = DMA_input_buffer[i + input_offset];
        T<B> dO;
        for (int j = 0; j < B / 32; j++) {
          int d = DMA_input_buffer[(i++) + input_offset * (B / 32)];
          dO.range((j + 1) * 32 - 1, j * 32) = d;
          sc_uint<32> data = dO.range((j + 1) * 32 - 1, j * 32);
          // pattern_count[pattern_check(data)]++;
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
        BDATA<B, T> d = dout1.read();
        while (i >= output_len) wait();
        packets_recv++;
        last = d.tlast;
        for (int j = 0; j < B / 32; j++) {
          DMA_output_buffer[(i++) + output_offset * (B / 32)] =
              d.data.range((j + 1) * 32 - 1, j * 32).to_int();
          // pattern_count[pattern_check(d.data)]++;

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

  // template class AXIS_ENGINE<32>;
  // template class AXIS_ENGINE<64>;

#endif