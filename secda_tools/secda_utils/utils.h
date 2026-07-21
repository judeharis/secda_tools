#ifndef SECDA_TOOLS_UTILS
#define SECDA_TOOLS_UTILS

#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <assert.h>

using namespace std;

struct DMATensor {
  unsigned int TID;

  char* data;
  unsigned int ranks = 0;
  unsigned int bitwidth = 0;
  unsigned int offset = 0;
  unsigned int size = 0;

  bool mmapped = false;
  bool alloced = false;
  bool isStatic = false;
  bool isExternalInput = false;
  bool isExternalOutput = false;

  vector<unsigned int> dims;
  vector<unsigned int> input_nodes;
  vector<unsigned int> output_nodes;

  DMATensor(unsigned int _TID, unsigned int _ranks, vector<unsigned int> _dims,
            unsigned int _bitwidth, bool _isStatic, bool _isExternalInput,
            bool _isExternalOutput) {
    ranks = _ranks;
    bitwidth = _bitwidth;
    TID = _TID;
    isStatic = _isStatic;
    isExternalInput = _isExternalInput;
    isExternalOutput = _isExternalOutput;
    dims = _dims;
    input_nodes = {};
    output_nodes = {};
  }
  DMATensor() {}

  void print_info() {
    cout << "Tensor ID: " << TID << ", Ranks: " << ranks
         << ", Bitwidth: " << bitwidth << ", Offset: " << offset
         << ", Size: " << size << ", Static: " << (isStatic ? "Yes" : "No")
         << ", Allocated: " << (alloced ? "Yes" : "No")
         << ", Mapped: " << (mmapped ? "Yes" : "No")
         << ", Ext-Input: " << (isExternalInput ? "Yes" : "No")
         << ", Ext-Output: " << (isExternalOutput ? "Yes" : "No") << endl;
    cout << "Dims: [";
    for (size_t i = 0; i < dims.size(); ++i) {
      cout << dims[i];
      if (i < dims.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
  }
};

struct DMATensorManager {

  map<unsigned int, DMATensor> tensors;

  int *buffer;              // Pointer to the buffer for tensor data
  unsigned int buffer_size; // Size of the buffer in bytes
  unsigned int current_offset;

  DMATensorManager() {}

  void set_buffer(int *buf, unsigned int size) {
    buffer = buf;
    buffer_size = size;
    current_offset = 0;
  }

  void allocator() {
    for (auto &pair : tensors) {
      DMATensor *tensor = &pair.second;
      if (tensor->mmapped) continue; // Skip already mmapped tensors

      unsigned int tensor_size = 1;
      for (const auto &dim : tensor->dims) {
        tensor_size *= dim;
      }
      tensor_size *= (tensor->bitwidth / 8); // Convert bitwidth to bytes
      if (current_offset + tensor_size > buffer_size) {
        cout << "Buffer size exceeded while allocating tensor ID: "
             << tensor->TID << endl;
        continue;
      }
      tensor->offset = current_offset;
      current_offset += tensor_size;
      tensor->mmapped = true;
    }
  }

  char *get_tensor_data(unsigned int TID) {
    if (tensors.find(TID) != tensors.end()) {
      const DMATensor &tensor = tensors[TID];
      if (!tensor.mmapped) {
        cout << "Tensor ID: " << TID << " is not mmapped." << endl;
        return nullptr;
      }
      return reinterpret_cast<char *>(buffer) + tensor.offset;
    } else {
      cout << "Tensor ID: " << TID << " not found in tensor offsets." << endl;
      return nullptr;
    }
  }

  void add_tensor(DMATensor tensor) { tensors[tensor.TID] = tensor; }

  void add_allocate_tensor(DMATensor new_tensor, char *data = nullptr) {
    add_tensor(new_tensor);
    DMATensor &tensor = tensors[new_tensor.TID];
    unsigned int tensor_size = 1;
    for (const auto &dim : tensor.dims) tensor_size *= dim;
    tensor_size *= (tensor.bitwidth / 8); // Convert bitwidth to bytes
    tensor.size = tensor_size;            // Store the size of the tensor
    if (current_offset + tensor_size > buffer_size) {
      cout << "Buffer size exceeded while allocating tensor ID: " << tensor.TID
           << endl;
      return;
    }
    tensor.offset = current_offset;
    tensor.alloced = true;
    tensor.data = reinterpret_cast<char *>(buffer) + tensor.offset; // Set the data pointer
    current_offset += tensor_size;
    if (tensor.isStatic) {
      char *tensor_data_ptr = tensor.data; // Pointer to the tensor's data in the buffer
      if (data != nullptr) {
        memcpy(tensor_data_ptr, data,
               tensor_size);   // Copy the data into the tensor
        tensor.mmapped = true; // Mark the tensor as mmapped after copying data
      } else {
        cout << "No data provided for static tensor ID: " << tensor.TID << endl;
      }
    }
  }

  bool check_exists(unsigned int TID) {
    return tensors.find(TID) != tensors.end();
  }

  DMATensor *get_tensor(unsigned int TID) {
    if (check_exists(TID)) {
      return &tensors[TID];
    } else {
      return nullptr;
    }
  }

  void copy_to_mmap(unsigned int TID, char *data) {
    if (check_exists(TID)) {
      DMATensor &tensor = tensors[TID];
      if (!tensor.alloced) {
        cout << "Tensor ID: " << TID << " is not allocated." << endl;
        return;
      }
      char *tensor_data_ptr = tensor.data; // Pointer to the tensor's data in the buffer
      memcpy(tensor_data_ptr, data, tensor.size);
      tensor.mmapped = true; // Mark the tensor as mmapped after copying data
      return;
    } else {
      cout << "Tensor ID: " << TID << " not found in tensor offsets." << endl;
      return;
    }
  }

  void copy_from_mmap(unsigned int TID, char *data) {
    if (check_exists(TID)) {
      DMATensor &tensor = tensors[TID];
      if (!tensor.alloced) {
        cout << "Tensor ID: " << TID << " is not allocated." << endl;
        return;
      }
      char *tensor_data_ptr = tensor.data; // Pointer to the tensor's data in the buffer
      memcpy(data, tensor_data_ptr, tensor.size);
      tensor.mmapped = true; // Mark the tensor as mmapped after copying data
      return;
    } else {
      cout << "Tensor ID: " << TID << " not found in tensor offsets." << endl;
      return;
    }
  }

  void print_tensors() {
    for (const auto &pair : tensors) {
      const DMATensor &tensor = pair.second;
      cout << "T" << tensor.TID << " | r=" << tensor.ranks << " bw=" << tensor.bitwidth
           << " off=" << tensor.offset << " sz=" << tensor.size
           << " st=" << (tensor.isStatic ? "Y" : "N")
           << " in=" << (tensor.isExternalInput ? "Y" : "N")
           << " out=" << (tensor.isExternalOutput ? "Y" : "N")
           << " al=" << (tensor.alloced ? "Y" : "N")
           << " mm=" << (tensor.mmapped ? "Y" : "N")
           << " d=[";

      for (size_t i = 0; i < tensor.dims.size(); ++i) {
        if (i) cout << "x";
        cout << tensor.dims[i];
      }

      cout << "] in=[";
      for (size_t i = 0; i < tensor.input_nodes.size(); ++i) {
        if (i) cout << ",";
        cout << tensor.input_nodes[i];
      }
      cout << "] out=[";
      for (size_t i = 0; i < tensor.output_nodes.size(); ++i) {
        if (i) cout << ",";
        cout << tensor.output_nodes[i];
      }
      cout << "]" << endl;
    }
  }
};

typedef struct int_data_pointers {
  int *W1;
  int *W2;
  int *W3;
  int *W4;
  int *R1;
  int *R2;
  int *R3;
  int *R4;

  int_data_pointers(int *_W1, int *_W2, int *_W3, int *_W4, int *_R1, int *_R2,
                    int *_R3, int *_R4) {
    W1 = _W1;
    W2 = _W2;
    W3 = _W3;
    W4 = _W4;
    R1 = _R1;
    R2 = _R2;
    R3 = _R3;
    R4 = _R4;
  }

} INT_DP;

struct int8_params {
  int8_t *data;
  const int8_t *immutable_data;
  int order;
  int rows;
  int cols;
  int depth;
  int zero_point;

  void Init(int8_t *data_, int order_, int row_, int cols_, int zero_point_) {
    data = data_;
    order = order_;
    rows = row_;
    cols = cols_;
    depth = 0;
    zero_point = zero_point_;
  }

  void Init(const int8_t *data_, int order_, int row_, int cols_, int depth_,
            int zero_point_) {
    immutable_data = data_;
    order = order_;
    rows = row_;
    cols = cols_;
    depth = depth_;
    zero_point = zero_point_;
  }
};

struct int32_params {
  int32_t *data;
  const int32_t *immutable_data;
  int order;
  int rows;
  int cols;
  int depth;
  int zero_point;

  void Init(int32_t *data_, int order_, int row_, int cols_, int zero_point_) {
    data = data_;
    order = order_;
    rows = row_;
    cols = cols_;
    depth = 0;
    zero_point = zero_point_;
  }

  void Init(const int32_t *data_, int order_, int row_, int cols_, int depth_,
            int zero_point_) {
    immutable_data = data_;
    order = order_;
    rows = row_;
    cols = cols_;
    depth = depth_;
    zero_point = zero_point_;
  }
};

void splitfinder(int *sizelist, int split, int num) {
  int rem = num % split;
  int divs = num / split;
  for (int i = 0; i < split; i++) {
    sizelist[i] = divs;
    if (i < rem) sizelist[i]++;
  }
}

int roundUp(int numToRound, int multiple) {
  if (multiple == 0) return numToRound;
  int remainder = numToRound % multiple;
  if (remainder == 0) return numToRound;
  return numToRound + multiple - remainder;
}

int roundDown(int numToRound, int multiple) {
  assert(multiple != 0);
  return numToRound - (numToRound % multiple);
}

void unpad_matrix(int N_dim, int M_dim, int tN, int tM, int8_t **padded_A,
                  int8_t *A) {
  int pM = roundUp(M_dim, tM);
  for (int n = 0; n < N_dim; n++) {
    for (int m = 0; m < M_dim; m++) {
      A[n * M_dim + m] = padded_A[n][m];
    }
  }
}

void pad_matrix(int N_dim, int M_dim, int tN, int tM, const int8_t *A,
                int8_t *padded_A) {
  int pM = roundUp(M_dim, tM);
  for (int n = 0; n < N_dim; n++) {
    for (int m = 0; m < M_dim; m++) {
      padded_A[n * pM + m] = A[n * M_dim + m];
    }
  }
}

void pad_matrix(int N_dim, int M_dim, int tN, int tM, const int8_t *A,
                int8_t **padded_A) {
  int pM = roundUp(M_dim, tM);
  for (int n = 0; n < N_dim; n++) {
    for (int m = 0; m < M_dim; m++) {
      padded_A[n][m] = A[n * M_dim + m];
    }
  }
  int pN = roundUp(N_dim, tN);
}

void trans_matrix(int N_dim, int pN, int M_dim, const int8_t *A, int8_t **B) {
  for (int n = 0; n < N_dim; n++)
    for (int m = 0; m < M_dim; m++) B[m][n] = A[n * M_dim + m];
}

void padT_matrix(int N_dim, int M_dim, int tN, int tM, const int8_t *A,
                 int8_t **padded_AT) {
  int pN = roundUp(N_dim, tN);
  trans_matrix(N_dim, pN, M_dim, A, padded_AT);
}

void unpadT_matrix(int N_dim, int M_dim, int tN, int tM, int8_t **padded_A,
                   int8_t *A) {
  for (int n = 0; n < N_dim; n++) {
    for (int m = 0; m < M_dim; m++) {
      A[n * M_dim + m] = padded_A[m][n];
    }
  }
}

#endif // SECDA_TOOLS_UTILS