#ifndef SYSC

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

// ===============================================================================
// AXI4Lite acc_ctrl
// ===============================================================================

template <typename T>
acc_ctrl<T>::acc_ctrl() {}

template <typename T>
void acc_ctrl<T>::init_sigs(int count) {
  sig_count = count;
}

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
bool acc_ctrl<T>::check_done() {
  if (read_reg(0x1C)) {
    write_reg(0x14, 0);
    while (read_reg(0x1C)) {
      msync(reg_base, PAGE_SIZE, MS_SYNC);
    }
    return true;
  }
  return false;
}

template <typename T>
unsigned int acc_ctrl<T>::get_reg(int reg) {
  if (reg < 0 || reg >= sig_count) {
    cerr << "Getting Invalid Signal Address" << endl;
    return 0;
  }
  msync(reg_base, PAGE_SIZE, MS_SYNC);
  unsigned int val = read_reg(0x24 + reg * 8);
  return val;
}

template <typename T>
void acc_ctrl<T>::set_reg(int reg, unsigned int val) {
  if (reg < 0 || reg >= sig_count) {
    cerr << "Setting Invalid Signal Address" << endl;
    return;
  }
  cout << "Setting reg[" << (0x24 + reg * 8) << "] = " << val << endl;
  write_reg(0x24 + reg * 8, val);
  msync(reg_base, PAGE_SIZE, MS_SYNC);
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
// AXI4Lite hwc_ctrl
// ================================================================================

template <typename T>
hwc_ctrl<T>::hwc_ctrl(){};

template <typename T>
void hwc_ctrl<T>::init_hwc(int count) {
  hwc_count = count;
}

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
  return read_reg(0x2C + (hwc * 0x18));
}

template <typename T>
unsigned int hwc_ctrl<T>::get_cycle_count(int hwc) {
  if (hwc < 0 || hwc >= hwc_count) {
    cerr << "HWC index out of bounds: " << hwc << endl;
    return -1;
  }
  return read_reg(0x24 + (hwc * 0x18));
}

template <typename T>
void hwc_ctrl<T>::print_hwc_map(bool clear_console) {
  // Clear the console (works on most UNIX terminals)
  if (clear_console) std::cout << "\033[2J\033[1;1H";
  cout << "================================================" << endl;
  cout << "HWC Control Register Map" << endl;
  for (int i = 0; i < hwc_count; i++) {
    int curr_target_state = read_reg(0x1C + (i * 0x18));
    cout << "HWC[" << (0x1C + (i * 0x18))
         << "] | Current State: " << get_current_state(i)
         << " | Cycle Count: " << get_cycle_count(i)
         << " | Target State: " << curr_target_state << endl;
  }
  cout << "================================================" << endl;
}
#endif