load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()



cc_library(
    name = "profiler",
    srcs = [
        "profiler.cc",
        ],
    hdrs = [
        "profiler.h"
    ],
    copts = common_copts,
)
