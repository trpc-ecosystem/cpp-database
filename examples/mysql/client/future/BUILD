cc_binary(
    name = "future_client",
    srcs = ["future_client.cc"],
    deps = [
        "@com_github_gflags_gflags//:gflags",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/client:make_client_context",
        "@trpc_cpp//trpc/client:trpc_client",
        "@com_github_trpc_ecosystem_cpp_database_mysql//trpc/client/mysql:mysql_plugin",
        "@trpc_cpp//trpc/common:runtime_manager",
        "@trpc_cpp//trpc/common:trpc_plugin",
    ],
)
