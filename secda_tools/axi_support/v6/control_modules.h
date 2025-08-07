#ifndef CONTROL_MODULES_V6_H
#define CONTROL_MODULES_V6_H

// ================================================================================
// axi4lite_ctrl || Private
// ================================================================================

template <typename T>
struct axi4lite_ctrl {
  int *reg_base;

  axi4lite_ctrl();
  axi4lite_ctrl(int *base_addr);
  unsigned int read_reg(unsigned int offset);
  void write_reg(unsigned int offset, unsigned int val);
};

// ===============================================================================
// acc_ctrl || Public
// ===============================================================================

template <typename T>
struct acc_ctrl : public axi4lite_ctrl<T> {

#ifdef SYSC
  ACC_CONTROL *ctrl;
  ctrl_signals *ctrl_sigs;
  reg_signals *reg_sigs;
#endif

  bool start = false;
  bool done = false;
  int sig_count;

  acc_ctrl();

  void init_sigs(int count);
  void start_acc();
  void wait_done();
  bool check_done();

  unsigned int get_reg(int addr);
  void set_reg(int addr, unsigned int val);

  void print_reg_map(bool clear_console);

private:
  using axi4lite_ctrl<T>::reg_base;
  using axi4lite_ctrl<T>::write_reg;

public:
  using axi4lite_ctrl<T>::read_reg;
  using axi4lite_ctrl<T>::axi4lite_ctrl;
};

// ================================================================================
// hwc_ctrl || Public
// ================================================================================

template <typename T>
struct hwc_ctrl : public axi4lite_ctrl<T> {
#ifdef SYSC
  HWC_RESETTER *hwc_resetter;
  hwc_signals *ctrl;
  sc_signal<bool> reset;
#endif
  int hwc_count;

  hwc_ctrl();

  void init_hwc(int count);

  void reset_hwc();

  void set_target_state(int hwc, int target_state);

  unsigned int get_current_state(int hwc);

  unsigned int get_cycle_count(int hwc);

  void print_hwc_map(bool clear_console);

private:
  using axi4lite_ctrl<T>::reg_base;

public:
  using axi4lite_ctrl<T>::read_reg;
  using axi4lite_ctrl<T>::write_reg;
  using axi4lite_ctrl<T>::axi4lite_ctrl;
};

#endif // CONTROL_MODULES_V6_H