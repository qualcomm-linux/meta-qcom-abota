inherit autotools pkgconfig

DESCRIPTION = "A/B mirror copy utility: clones active-slot partition contents \
to the inactive slot (with SHA1 verification) after a successful OTA boot."

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/${LICENSE};md5=7a434440b651f4a472ca93716d01033a"

DEPENDS += "openssl qcom-efiabctl-abota"

SRC_URI = "file://Makefile.am \
           file://configure.ac \
           file://mirror_copy.cpp \
           "

S = "${UNPACKDIR}"
