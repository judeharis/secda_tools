#ifndef AXI4LITE_CONTROL_H
#define AXI4LITE_CONTROL_H

#include "sysc_types.h"

// SC_MODULE(AXI4LITE_CONTROL) {
//   sc_in<bool> clock;
//   sc_in<bool> reset;

//   sc_in<bool> start;
//   sc_in<bool> done;

//   void HandShake() {
//     while (1) {
//       while (!start) wait();
//       while (!done) wait();
//       sc_pause();
//       wait();
//       while (done) wait();
//     }
//   };

//   SC_HAS_PROCESS(AXI4LITE_CONTROL);

//   AXI4LITE_CONTROL(sc_module_name name_) : sc_module(name_) {
//     SC_CTHREAD(HandShake, clock.pos());
//     reset_signal_is(reset, true);
//   }
// };

#endif // AXI4LITE_CONTROL_H