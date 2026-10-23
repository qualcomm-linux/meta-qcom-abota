inherit swupdate-image

SWUPDATE_IMAGE_SYSTEM = "system.img"
SWUPDATE_IMAGE_EFI = "efi.bin"

SWUPDATE_IMAGES = "${SWUPDATE_IMAGE_SYSTEM} ${SWUPDATE_IMAGE_EFI}"

SRC_URI += "file://qcom-postinstall-abota.sh"

# CMS (PKCS#7) signing of sw-description, matched by on-device verification in
# recipes-support/swupdate/swupdate/qcom-swupdate-signing.cfg.
#
# A single global flag, ABOTA_SIGNING, drives BOTH sides (this build-side
# signing and the device-side verification wired in swupdate_%.bbappend) so
# they can never drift out of sync. Set it in local.conf or a product layer:
#
#     ABOTA_SIGNING = "1"
#
# and drop a signing keypair into this layer's files/ (default filenames
# below; override SWUPDATE_CMS_CERT/SWUPDATE_CMS_KEY to use any other name or
# an out-of-tree CI secret path). That is the whole opt-in -- no file edits
# needed. Default is "0": unsigned .swu, no cert shipped, no key required, and
# the device accepts the unsigned package.
#
# When enabled, do_swuimage runs "openssl cms -sign" over sw-description and
# bundles sw-description.sig into the .swu; the device verifies it against the
# public cert installed by swupdate_%.bbappend (its public-key-file).
#
# The cert/key are read directly from the shared signing dir ABOTA_SIGNING_DIR
# (defined in conf/layer.conf, overridable in local.conf or via export), NOT
# fetched through SRC_URI. In an image recipe SRC_URI file:// artifacts unpack
# into ${S} (= ${UNPACKDIR}), but image.bbclass declares
# do_rootfs[cleandirs] += "${S}", so do_rootfs wipes anything unpacked there
# long before do_swuimage runs -- the cert/key would vanish. Reading them from
# the stable dir sidesteps that, and keeping them out of SRC_URI also means
# they can never be bundled into the .swu cpio (a private key must never ship
# inside an update package).
#
# DEVELOPMENT ONLY -- this layer ships no cert or key; both are supplied
# locally and the private key is git-ignored (see .gitignore) so it never
# enters history. For release builds, point ABOTA_SIGNING_DIR (or the
# SWUPDATE_CMS_CERT/SWUPDATE_CMS_KEY paths directly) at a product/CI keypair
# and install only the matching public cert on target.
ABOTA_SIGNING ??= "0"

SWUPDATE_CMS_CERT ?= "${ABOTA_SIGNING_DIR}/qcom-swupdate-dev.cert.pem"
SWUPDATE_CMS_KEY ?= "${ABOTA_SIGNING_DIR}/qcom-swupdate-dev.key.pem"
SWUPDATE_SIGNING = "${@'CMS' if d.getVar('ABOTA_SIGNING') == '1' else ''}"

do_generate_ext4_image() {
    install -d ${IMAGE_ROOTFS}/efi
}
addtask generate_ext4_image after do_rootfs before do_image_complete

do_deploy_ext4_image() {
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.ext4 ${DEPLOY_DIR_IMAGE}/system.img
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.ext4 ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.qcomflash/system.img
}
addtask deploy_ext4_image after do_image_complete before do_build

do_deploy_efi_image() {
    if [ -n "${QCOM_ESP_FILE}" ]; then
        install -m 0644 ${QCOM_ESP_FILE} ${DEPLOY_DIR_IMAGE}/${SWUPDATE_IMAGE_EFI}
    fi
}
do_deploy_efi_image[depends] += "${@ ['', '${QCOM_ESP_IMAGE}:do_image_complete'][d.getVar('QCOM_ESP_IMAGE') != '']}"
addtask deploy_efi_image after do_image_complete before do_build

addtask swuimage after do_deploy_ext4_image do_deploy_efi_image
