FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://qcom-swupdate.cfg"

# A single global flag, ABOTA_SIGNING, drives both the device-side signature
# verification here and the build-side .swu signing in
# classes/qcom-swupdate-abota-image.bbclass, so the two can never drift apart.
#
#   ABOTA_SIGNING = "1"   (in local.conf or a product layer)
#
# turns on verification: it merges qcom-swupdate-signing.cfg into the swupdate
# build (CONFIG_SIGNED_IMAGES=y etc.), installs the public verification cert,
# and points swupdate's public-key-file at it. Default "0" builds a binary
# that accepts an unsigned .swu, installs no cert, and needs no keypair.
#
# The signing material lives in ONE place: ABOTA_SIGNING_DIR (defined in
# conf/layer.conf, overridable in local.conf or via export). The build-side
# .swu signing in classes/qcom-swupdate-abota-image.bbclass reads the keypair
# from there, and the public verification cert is sourced from that same dir
# here (see the second FILESEXTRAPATHS below) -- the customer drops cert + key
# into that single directory, nothing is duplicated.
#
# SWUPDATE_PUBLIC_CERT is the public cert filename resolved out of that shared
# dir. The default name matches the build-side keypair default; override it
# (here or in a product layer) to use any other name. There is no fixed/
# required name, and the layer ships no cert.
ABOTA_SIGNING ??= "0"
SWUPDATE_PUBLIC_CERT ?= "qcom-swupdate-dev.cert.pem"

# Shared signing dir -- the same ABOTA_SIGNING_DIR the image recipe reads the
# keypair from. Only the public cert is fetched from here, and only when
# ABOTA_SIGNING = "1"; adding the path is harmless otherwise.
FILESEXTRAPATHS:prepend := "${ABOTA_SIGNING_DIR}:"

SRC_URI += "file://qcom-swupdate-runtime.conf"
SRC_URI += "${@' file://qcom-swupdate-signing.cfg file://${SWUPDATE_PUBLIC_CERT}' if d.getVar('ABOTA_SIGNING') == '1' else ''}"

do_install:append() {
    install -d ${D}${sysconfdir}/swupdate
    install -m 0644 ${UNPACKDIR}/qcom-swupdate-runtime.conf ${D}${sysconfdir}/swupdate.cfg

    if [ "${ABOTA_SIGNING}" = "1" ]; then
        # Install the public cert and point swupdate's public-key-file at it.
        # The runtime config ships without a public-key-file line; insert it
        # here so the path tracks SWUPDATE_PUBLIC_CERT automatically.
        install -m 0644 ${UNPACKDIR}/${SWUPDATE_PUBLIC_CERT} ${D}${sysconfdir}/swupdate/${SWUPDATE_PUBLIC_CERT}
        sed -i 's|^{|{\n    public-key-file = "${sysconfdir}/swupdate/${SWUPDATE_PUBLIC_CERT}";|' ${D}${sysconfdir}/swupdate.cfg
    fi
}

FILES:${PN} += "${sysconfdir}/swupdate.cfg"
FILES:${PN} += "${@' ${sysconfdir}/swupdate/${SWUPDATE_PUBLIC_CERT}' if d.getVar('ABOTA_SIGNING') == '1' else ''}"
