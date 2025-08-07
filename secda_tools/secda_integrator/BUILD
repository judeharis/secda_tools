load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "secda_hw_utils",
    srcs = [
        "secda_hw_utils.sc.h",
        "sysc_types.h",
        ],
    copts = common_copts,
    deps = [
        "@systemc_bazel//:systemc_lib",
        "//secda_tools/ap_sysc:ap_sysc",
    ],
)


cc_library(
    name = "secda_integrator",
    srcs = [
        "systemc_integrate.h",
        "axi4s_engine_generic.sc.h",
        "hwc.sc.h"
        ],
    copts = common_copts,
    deps = [
        "@systemc_bazel//:systemc_lib",
        ":secda_hw_utils",
        "//secda_tools/ap_sysc:ap_sysc",
    ],
)
