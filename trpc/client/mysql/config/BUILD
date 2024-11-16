licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "mysql_client_conf",
    srcs = ["mysql_client_conf.cc"],
    hdrs = ["mysql_client_conf.h"],
    deps = [
        "@trpc_cpp//trpc/util/log:logging",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
    ],
)

cc_library(
    name = "mysql_client_conf_parser",
    hdrs = ["mysql_client_conf_parser.h"],
    deps = [
        ":mysql_client_conf",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
    ],
)