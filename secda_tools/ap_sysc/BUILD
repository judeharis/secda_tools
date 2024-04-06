load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "ap_sysc",
    hdrs = [
        "ap_mem_if.h",
        "ap_sc_ext.h",
        "AXI4_if.h",
        "hls_bus_if.h",
    ],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
    ],
)
