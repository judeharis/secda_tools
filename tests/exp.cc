#include "secda_tools/axi_support/v5/axi_api_v5.h"
#include "secda_tools/secda_profiler/profiler.h"
#ifdef SYSC
#include "secda_tools/secda_integrator/systemc_integrate.h"
#endif

#include <iostream>
#include <unistd.h>

using namespace std;
#define TSCALE microseconds
#define TSCAST duration_cast<nanoseconds>

int main() {
  static struct Profile profile;
  duration_ns cpu_total;

  prf_start(0);
  cout << "Hello, World!" << endl;

  prf_end(0, cpu_total);
  prf_out(TSCALE, cpu_total);
  return 0;
}
