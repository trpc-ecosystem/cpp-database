"""This module contains some dependency"""

# buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def clean_dep(dep):
    return str(Label(dep))

# buildifier: disable=function-docstring-args
def database_mysql_workspace(path_prefix = "", repo_name = "", **kwargs):
    """Build rules for the trpc project

    Note: The main idea is to determine the required version of dependent software during the build process
          by passing in parameters.
    Args:
        path_prefix: Path prefix.
        repo_name: Repository name of the dependency.
        kwargs: Keyword arguments, dictionary type, mainly used to specify the version and sha256 value of
                dependent software, where the key of the keyword is constructed by the `name + version`.
                eg: protobuf_ver,zlib_ver...
    Example:
        trpc_workspace(path_prefix="", repo_name="", protobuf_ver="xxx", protobuf_sha256="xxx", ...)
        Here, `xxx` is the specific specified version. If the version is not specified through the key,
        the default value will be used. eg: protobuf_ver = kwargs.get("protobuf_ver", "3.8.0")
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