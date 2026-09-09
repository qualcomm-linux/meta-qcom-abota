inherit autotools pkgconfig

DESCRIPTION = "Standalone EFI-variable based A/B slot control library and utility."

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/${LICENSE};md5=7a434440b651f4a472ca93716d01033a"

DEPENDS += "efivar"

SRC_URI = "file://abctl/Makefile.am \
           file://abctl/configure.ac \
           file://abctl/inc/efiabctl.h \
           file://abctl/inc/libabctl.h \
           file://abctl/src/efiabctl.cpp \
           file://abctl/src/libabctl.cpp \
           file://abctl/src/abctl.cpp \
           file://abctl/scripts/abctl-helper.sh \
           "

S = "${UNPACKDIR}/abctl"
