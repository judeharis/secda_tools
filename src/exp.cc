#include "secda_tools/secda_profiler/profiler.h"
#include "secda_tools/secda_integrator/systemc_integrate.h"

#include <iostream>
#include <unistd.h>

using namespace std;
#define TSCALE microseconds
#define TSCAST duration_cast<nanoseconds>


// Simple Hello World module
SC_MODULE (hello_world) {
  SC_CTOR (hello_world) {
  }
  void say_hello() {
    cout << "Hello World.\n";
  }
};


int main() {
  static struct Profile profile;
  duration_ns cpu_total;
  // cpu_total = duration_ns::zero();

  prf_start(0);
  // cout << "Hello, World!" << endl;
  hello_world hello("HELLO");
  for (int i = 0; i < 1; i++) {
      hello.say_hello();
  }
  
  prf_end(0, cpu_total);
  prf_out(TSCALE, cpu_total);
  return 0;
}
#include <systemc.h>

