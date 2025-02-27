load("@rules_cc//cc:defs.bzl", "cc_library")
load("//:def.bzl", "secda_copts")

package(default_visibility = ["//visibility:public"])

common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "axi_support",
    srcs = [
        "v2/axi_api_v2.cc",
    ],
    hdrs = [
        "v2/axi_api_v2.h",
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
        "v2/axi_api_v2.h",
    ],
    copts = common_copts,
    deps = [
        "//secda_tools/secda_integrator",
        "//secda_tools/secda_profiler:profiler",
        "@systemc_bazel//:systemc_lib",
    ],
)

cc_library(
    name = "axi_supportv3",
    srcs = [
        "v3/axi_api_v3.cc",
    ],
    hdrs = [
        "v3/axi_api_v3.h",
    ],
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
    ],
)

cc_library(
    name = "axi_support_syscv3",
    srcs = [
        "v3/axi_api_sysc_v3.cc",
    ],
    hdrs = [
        "v3/axi_api_v3.h",
    ],
    copts = common_copts,
    deps = [
        "//secda_tools/secda_integrator",
        "//secda_tools/secda_profiler:profiler",
        "@systemc_bazel//:systemc_lib",
    ],
)

cc_library(
    name = "axi_supportv4",
    srcs = [
        "v4/axi_api_v4.h",
    ],
    hdrs = select({
        "//:sysc": ["v4/axi_api_sysc_v4.tpp"],
        "//:fpga": ["v4/axi_api_v4.tpp"],
        "//conditions:default": [],
    }),
    copts = common_copts,
    deps = [
        "//secda_tools/secda_profiler:profiler",
        "//secda_tools/secda_utils",
    ] + select({
        "//:sysc": [
            "//secda_tools/secda_integrator",
            "@systemc_bazel//:systemc_lib",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "libxlnk_cma",
    srcs = select({
        "//:fpga": ["arm32/libcma.so"],
        "//:fpga_arm64": ["arch64/libcma.so"],
        "//conditions:default": ["arm32/libcma.so"],
    }),
    copts = common_copts + ["-Lhome/jude/Workspace/secda_tools/secda_tools/axi_support/"],
    linkopts = ["-lpthread"],
)

cc_library(
    name = "axi_supportv5",
    srcs = [
        "libxlnk_cma.h",
        "v5/axi_api_v5.h",
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
            "@systemc_bazel//:systemc_lib",
        ],
        "//:fpga": ["//secda_tools/axi_support:libxlnk_cma"],
        "//:fpga_arm64": ["//secda_tools/axi_support:libxlnk_cma"],
        "//conditions:default": ["//secda_tools/axi_support:libxlnk_cma"],
    }),
)
