"""This module contains some dependency"""

# buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def clean_dep(dep):
    return str(Label(dep))

# buildifier: disable=function-docstring-args
def trpc_cpp_database_workspace(path_prefix = "", repo_name = "", **kwargs):
    """Define workspace rules for the MySQL client dependency used in the tRPC project.

    Args:
        path_prefix: Path prefix for workspace configuration (currently unused in this function).
        repo_name: Repository name for the dependency (currently unused in this function).
        kwargs: Additional keyword arguments for specifying version and sha256 of dependencies.
                For example:
                - `mysqlclient_ver`: Version of the MySQL client library (default: "8.0.39").
                - `mysqlclient_sha256`: SHA256 checksum of the specified MySQL client library version

    Example:
        trpc_cpp_database_workspace(
            mysqlclient_ver="8.0.33",
            mysqlclient_sha256="specific_sha256_value",
        )

    """

    # libmysqlclient
    mysqlclient_ver = kwargs.get("mysqlclient_ver", "8.0.39")
    mysqlclient_major_ver = mysqlclient_ver.split(".")[0] + "." + mysqlclient_ver.split(".")[1]
    mysqlclient_sha256 = kwargs.get("mysqlclient_sha256", "ed3722dcd91607e118ed6cf51246f7d0882d982b33dab2935a2ec6c6e1271f2e")
    mysqlclient_urls = [
        "https://dev.mysql.com/get/Downloads/MySQL-{major}/mysql-{full_ver}-linux-glibc2.17-x86_64-minimal.tar.xz".format(
            major = mysqlclient_major_ver,
            full_ver = mysqlclient_ver
        )
    ]
    http_archive(
        name = "mysqlclient",
        urls = mysqlclient_urls,
        strip_prefix = "mysql-{full_ver}-linux-glibc2.17-x86_64-minimal".format(full_ver = mysqlclient_ver),
        sha256 = mysqlclient_sha256,
        build_file = clean_dep("//third_party/mysqlclient:mysqlclient.BUILD"),
    )