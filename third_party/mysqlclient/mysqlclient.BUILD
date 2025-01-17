# mysql.BUILD

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "mysqlclient_deps",
    srcs = [],
    hdrs = [],
    linkopts = [
        "-lssl",
        "-lcrypto",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "mysqlclient",
    srcs = glob(["lib/libmysqlclient.a"]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    deps = [":mysqlclient_deps"],
    strip_include_prefix = "include",
    include_prefix = "mysqlclient",
    visibility = ["//visibility:public"],
)

