FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://qcom-swupdate.cfg"

# Install the runtime config (/etc/swupdate.cfg) and the CMS verification cert so
# swupdate accepts signed .swu packages on-device. CONFIG_SIGNED_IMAGES=y
# (qcom-swupdate.cfg) makes the binary reject any update unless it can verify
# sw-description.sig against a public key; qcom-swupdate-runtime.conf points
# public-key-file at the cert installed just below.
#
# This cert is the public half of the signing key used by the ABOTA image build
# (meta-qcom-abota/recipes-products/images/files/qcom-swupdate-dev.{cert,key}.pem),
# so a .swu signed by that build verifies against this on-device cert.
# DEVELOPMENT ONLY -- for release builds, install only the product public cert.
SRC_URI += " \
    file://qcom-swupdate-runtime.conf \
    file://qcom-swupdate-dev.cert.pem \
"

do_install:append() {
    install -d ${D}${sysconfdir}/swupdate
    install -m 0644 ${UNPACKDIR}/qcom-swupdate-dev.cert.pem ${D}${sysconfdir}/swupdate/qcom-swupdate-dev.cert.pem
    install -m 0644 ${UNPACKDIR}/qcom-swupdate-runtime.conf ${D}${sysconfdir}/swupdate.cfg
}

FILES:${PN} += " \
    ${sysconfdir}/swupdate.cfg \
    ${sysconfdir}/swupdate/qcom-swupdate-dev.cert.pem \
"
