load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
package(default_visibility = ["//visibility:public"])
common_copts = ["-Wall"] + secda_copts()

cc_library(
    name = "secda_utils",
    hdrs = [
        "multi_threading.h",
        "acc_helpers.h",
        "utils.h",
    ],
    copts = common_copts,
)
