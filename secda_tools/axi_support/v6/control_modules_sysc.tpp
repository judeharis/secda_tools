#ifdef SYSC

// ================================================================================
// ACC Control API
// ================================================================================

template <typename T>
axi4lite_ctrl<T>::axi4lite_ctrl() {}

template <typename T>
axi4lite_ctrl<T>::axi4lite_ctrl(int *base_addr) {}

template <typename T>
unsigned int axi4lite_ctrl<T>::read_reg(unsigned int offset) {
  cout << "This call should not be used in simulation" << endl;
  return 0;
}

template <typename T>
void axi4lite_ctrl<T>::write_reg(unsigned int offset, unsigned int val) {
  cout << "This call should not be used in simulation" << endl;
}

// ===============================================================================
// acc_ctrl
// ===============================================================================

template <typename T>
acc_ctrl<T>::acc_ctrl() {
  reg_base = nullptr;
  string name("ACC_CONTROL");
  ctrl = new ACC_CONTROL(&name[0]);
  ctrl_sigs = new ctrl_signals();
  ctrl->start(ctrl_sigs->sig_start);
  ctrl->done(ctrl_sigs->sig_done);
  reg_sigs = nullptr;
  sig_count = 0;
}

template <typename T>
void acc_ctrl<T>::init_sigs(int count) {
  reg_sigs = new reg_signals[count];
  for (int i = 0; i < count; i++) {
    reg_sigs[i].sig.write(0);
  }
  sig_count = count;
}

template <typename T>
void acc_ctrl<T>::start_acc() {
  start = true;
  ctrl->start_bool = start;
}

template <typename T>
void acc_ctrl<T>::wait_done() {
  sc_start();
  start = false;
  done = ctrl->done.read();
}

template <typename T>
bool acc_ctrl<T>::check_done() {
  wait_done();
  return true;
}

template <typename T>
unsigned int acc_ctrl<T>::get_reg(int reg) {
  if (reg < 0 || reg >= sig_count) {
    cerr << "Getting Invalid Signal Address" << endl;
    return 1;
  }
  return reg_sigs[reg].sig.read();
}

template <typename T>
void acc_ctrl<T>::set_reg(int reg, unsigned int val) {
  if (reg < 0 || reg >= sig_count) {
    cerr << "Setting Invalid Signal Address" << endl;
    return;
  }
  reg_sigs[reg].sig.write(val);
}

template <typename T>
void acc_ctrl<T>::print_reg_map(bool clear_console) {
  // Clear the console (works on most UNIX terminals)
  if (clear_console) std::cout << "\033[2J\033[1;1H";
  cout << "================================================" << endl;
  cout << "ACC Control Register Map" << endl;
  for (int i = 0; i < sig_count; i++) {
    cout << "Reg[" << (0x24 + i * 8) << "]: " << get_reg(i) << endl;
  }
  cout << "================================================" << endl;
}

// ================================================================================
// hwc_ctrl
// ================================================================================

template <typename T>
hwc_ctrl<T>::hwc_ctrl() {
  reg_base = nullptr;
  string name("HWC_RESETTER");
  hwc_resetter = new HWC_RESETTER(&name[0]);
};

template <typename T>
void hwc_ctrl<T>::init_hwc(int count) {
  ctrl = new hwc_signals[count];
  hwc_count = count;
}

template <typename T>
void hwc_ctrl<T>::reset_hwc() {
  hwc_resetter->hwc_reset_bool = true;
  sc_start();
}

template <typename T>
void hwc_ctrl<T>::set_target_state(int hwc, int target_state) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
  }
  ctrl[hwc].sts.write(target_state);
}

template <typename T>
unsigned int hwc_ctrl<T>::get_current_state(int hwc) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
    return 1;
  }
  return ctrl[hwc].so.read();
}

template <typename T>
unsigned int hwc_ctrl<T>::get_cycle_count(int hwc) {
  return ctrl[hwc].co.read();
}

template <typename T>
void hwc_ctrl<T>::print_hwc_map(bool clear_console) {
  // Clear the console (works on most UNIX terminals)
  if (clear_console) std::cout << "\033[2J\033[1;1H";
  cout << "================================================" << endl;
  cout << "HWC Control Register Map" << endl;
  for (int i = 0; i < hwc_count; i++) {
    int curr_target_state = ctrl[i].sts.read();
    cout << "HWC[" << (0x1C + (i * 0x18))
         << "] | Current State: " << get_current_state(i)
         << " | Cycle Count: " << get_cycle_count(i)
         << " | Target State: " << curr_target_state << endl;
  }
  cout << "================================================" << endl;
}

#endif