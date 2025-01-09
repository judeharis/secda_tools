load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts() 


cc_library(
    name = "axi_support",
    srcs = [
        "v2/axi_api_v2.cc",
    ],
    hdrs = [
        "v2/axi_api_v2.h"
    ],
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
    ],
)


cc_library(
    name = "axi_support_sysc",
    srcs = [
        "v2/axi_api_sysc_v2.cc",
    ],
    hdrs = [
        "v2/axi_api_v2.h"
    ],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/secda_profiler:profiler",
    ],
)

cc_library(
    name = "axi_supportv3",
    srcs = [
        "v3/axi_api_v3.cc",
    ],
    hdrs = [
        "v3/axi_api_v3.h"
    ],
    deps = [
        "//secda_tools/secda_profiler:profiler",
    ],
    copts = common_copts,
)


cc_library(
    name = "axi_support_syscv3",
    srcs = [
        "v3/axi_api_sysc_v3.cc",
    ],
    hdrs = [
        "v3/axi_api_v3.h"
    ],
    copts = common_copts,
    deps = [
        "@systemc//:systemc_lib",
        "//secda_tools/secda_integrator:secda_integrator",
        "//secda_tools/secda_profiler:profiler",
    ],
)


cc_library(
    name = "axi_supportv4",
    srcs = [
        "v4/axi_api_v4.h"
    ],
    hdrs = select({
        "//:sysc": ["v4/axi_api_sysc_v4.tpp"],
        "//:fpga": ["v4/axi_api_v4.tpp"],
        "//conditions:default": [],
    }),
    copts = common_copts,
    deps =  [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils:secda_utils",
    ] + select({
        "//:sysc": [ "@systemc//:systemc_lib",
        "//secda_tools/secda_integrator:secda_integrator"],
        "//conditions:default": [],
    }),
)



cc_library(
    name = "libxlnk_cma",
    srcs = [
        "libcma.so",
    ],
    linkopts = ["-lpthread"],
    copts = common_copts + ["-Lhome/jude/Workspace/secda_tools/secda_tools/axi_support/"]
)

cc_library(
    name = "axi_supportv5",
    srcs = [
        "v5/axi_api_v5.h",
        "libxlnk_cma.h",
    ],
    hdrs = select({
        "//:sysc": ["v5/axi_api_sysc_v5.tpp"],
        "//:fpga": ["v5/axi_api_v5.tpp"],
        "//conditions:default": [],
    }),
    copts = common_copts + ["-Lhome/jude/Workspace/secda_tools/secda_tools/axi_support/"],
    deps = [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils",
    ] + select({
        "//:sysc": [
            "//secda_tools/secda_integrator",
            "@systemc//:systemc_lib",
        ],
        "//conditions:default": ["//secda_tools/axi_support:libxlnk_cma"],
    })
)