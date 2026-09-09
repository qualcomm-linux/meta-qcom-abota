FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://qcs8300-partitions-abota.conf"

do_configure:append() {
    install -d ${S}/platforms/qcs8300-ride-sx/emmc
    install -m 0644 ${UNPACKDIR}/qcs8300-partitions-abota.conf ${S}/platforms/qcs8300-ride-sx/emmc/partitions.conf
}
