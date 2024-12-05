licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mysql_statement",
    srcs = ["mysql_statement.cc"],
    hdrs = ["mysql_statement.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@mysqlclient//:mysqlclient",
        "@trpc_cpp//trpc/util/log:logging"
    ]
)

cc_library(
    name = "mysql_type",
    srcs = ["mysql_type.cc"],
    hdrs = ["mysql_type.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@mysqlclient//:mysqlclient",
        "@trpc_cpp//trpc/util:time"
    ],
)

cc_library(
    name = "mysql_binder",
    hdrs = [ "mysql_binder.h"],
    srcs = [ "mysql_binder.cc"],
    deps = [
        ":mysql_type",
        "@trpc_cpp//trpc/util:string_util",
    ],
)

cc_library(
    name = "mysql_results",
    hdrs = ["mysql_results.h"],
    deps = [
        "@mysqlclient//:mysqlclient",
        "//trpc/client/mysql:mysql_error_number",
        ":mysql_binder"
        ],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "mysql_executor",
    srcs = ["mysql_executor.cc"],
    hdrs = ["mysql_executor.h",],
    deps = [
        ":mysql_statement",
        ":mysql_results",
        ":mysql_binder",
        "@trpc_cpp//trpc/util:time",
        "@trpc_cpp//trpc/util:ref_ptr",
        "@trpc_cpp//trpc/util/log:logging",
        "@trpc_cpp//trpc/util:string_util",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "mysql_executor_test",
    srcs = ["mysql_executor_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":mysql_executor",
        "@trpc_cpp//trpc/util:random",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ]
)