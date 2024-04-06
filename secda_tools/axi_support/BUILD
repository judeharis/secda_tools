load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()


cc_library(
    name = "axi_support",
    srcs = [
        "axi_api_v2.cc",
    ],
    hdrs = [
        "axi_api_v2.h"
    ],
    copts = common_copts,
    deps = [
    ],
)


cc_library(
    name = "axi_support_sysc",
    srcs = [
        "axi_api_sysc_v2.cc",
    ],
    hdrs = [
        "axi_api_v2.h"
    ],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_integrator:secda_integrator",
    ],
)

cc_library(
    name = "axi_supportv3",
    srcs = [
        "axi_api_v3.cc",
    ],
    hdrs = [
        "axi_api_v3.h"
    ],
    copts = common_copts,
)


cc_library(
    name = "axi_support_syscv3",
    srcs = [
        "axi_api_sysc_v3.cc",
    ],
    hdrs = [
        "axi_api_v3.h"
    ],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_integrator:secda_integrator",
    ],
)
