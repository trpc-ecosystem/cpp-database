# Description: trpc-cpp.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mysql_protocol",
    srcs = ["mysql_protocol.cc"],
    hdrs = ["mysql_protocol.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@trpc_cpp//trpc/codec:protocol",
        "@trpc_cpp//trpc/util/buffer:noncontiguous_buffer",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_library(
    name = "mysql_client_codec",
    srcs = ["mysql_client_codec.cc"],
    hdrs = ["mysql_client_codec.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":mysql_protocol",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/codec:client_codec",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

