package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "app",
    srcs = ["sensor_receiver.c"],
    deps = [
        "//encode:encode",
        "//logging:logging",
        "//proto_compiled:proto_compiled",
        "//sensor:sensor",
        "@nanopb//:nanopb",
        "//iota:mam",
        "//jsmn:jsmn",
    ],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-pthread"],
)