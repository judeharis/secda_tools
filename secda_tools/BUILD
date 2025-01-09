load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")
package(default_visibility = ["//visibility:public"])

common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "secda_tools",
    copts = common_copts,
    deps = [
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/axi_support:axi_support",
    ],
)

cc_library(
    name = "secda_tools_sim",
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_support_sysc",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "secda_toolsv3",
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_supportv3",

    ],
)

cc_library(
    name = "secda_tools_simv3",
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_support_syscv3",

    ],
)

cc_library(
    name = "secda_tools_fpga_v4",
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_supportv4",

    ],
)

cc_library(
    name = "secda_toolsv4",
    copts = common_copts,
    deps =  [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_supportv4",
    ] + select({
        "//:sysc": [ "@systemc//:systemc_lib",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "secda_toolsv5",
    copts = common_copts,
    deps =  [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils:secda_utils",
        "//secda_tools/axi_support:axi_supportv5",
    ] + select({
        "//:sysc": [ "@systemc//:systemc_lib",
        "//secda_tools/ap_sysc:ap_sysc",
        "//secda_tools/secda_integrator:secda_integrator"],
        "//conditions:default": [],
    }),
)