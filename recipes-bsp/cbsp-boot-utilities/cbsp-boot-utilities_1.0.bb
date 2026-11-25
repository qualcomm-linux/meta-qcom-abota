SUMMARY = "Qualcomm UEFI secure on-device daemon (uefi_sec)"
DESCRIPTION = "uefi_sec: on-device systemd daemon from cbsp-boot-utilities that \
periodically syncs the UEFI secure-variable tables to the UEFISecApp trustlet. \
This build uses the MinkCom (MINK-IPC) transport via minkadaptor, with the \
IClientEnv client bindings generated from the shipped IDLs by idlc."
HOMEPAGE = "https://github.com/quic/cbsp-boot-utilities"

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/${LICENSE};md5=7a434440b651f4a472ca93716d01033a"

SRC_URI = "git://github.com/quic/cbsp-boot-utilities.git;protocol=https;branch=main \
           file://0001-uefi-sec-use-qcomtee-mink-ipc.patch \
           file://0002-uefi-sec-use-minkcom.patch \
"
SRCREV = "d5833dcaa437b4424de6199312f9f1f3d5a85b09"


S = "${UNPACKDIR}/${BPN}-${PV}/uefi_sec"

inherit autotools pkgconfig systemd

# glib-2.0                 - required by configure.ac (PKG_CHECK_MODULES GLIB)
# minkipc                  - provides minkadaptor.pc, <MinkCom.h> and -lminkadaptor
# mink-idl-compiler-native - provides the host 'idlc' used to generate the
#                            IClientEnv/IIO/ICredentials headers from the IDLs
DEPENDS += " \
    glib-2.0 \
    minkipc \
    mink-idl-compiler-native \
"

# Resolve the systemd unit dir explicitly instead of letting configure probe
# pkg-config (which can return a host path under cross-compilation).
EXTRA_OECONF += "--with-systemdunitdir=${systemd_system_unitdir}"

SYSTEMD_SERVICE:${PN} = "uefi_sec.service"

# uefi_sec.service: Requires=qteesupplicant.service, provided by minkipc.
RDEPENDS:${PN} += "minkipc-qteesupplicant"
