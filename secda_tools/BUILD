load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "secda_tflite",
    copts = common_copts,
    deps = [
        "//secda_tools/threading_utils:threading_utils",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/axi_support:axi_support",
    ],
)

cc_library(
    name = "secda_tflite_sim",
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/threading_utils:threading_utils",
        "//secda_tools/axi_support:axi_support_sysc",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "secda_tflitev3",
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/threading_utils:threading_utils",
        "//secda_tools/axi_support:axi_supportv3",

    ],
)

cc_library(
    name = "secda_tflite_simv3",
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/threading_utils:threading_utils",
        "//secda_tools/axi_support:axi_support_syscv3",

    ],
)

cc_binary(
    name = "test",
    srcs = ["test.cc"],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
    ],
)
