load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:def.bzl", "secda_copts")
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")
package(default_visibility = ["//visibility:public"])

common_copts = ["-Wall"] + secda_copts()



cc_binary(
    name = "exp",
    srcs = ["exp.cc"],
    copts = common_copts,
    deps = [
        "//secda_tools:secda_toolsv5",
    ],
)
