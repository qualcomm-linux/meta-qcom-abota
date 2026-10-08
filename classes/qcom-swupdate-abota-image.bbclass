inherit swupdate-image

SWUPDATE_IMAGE_SYSTEM = "system.img"
SWUPDATE_IMAGE_EFI = "efi.bin"

SWUPDATE_IMAGES = "${SWUPDATE_IMAGE_SYSTEM} ${SWUPDATE_IMAGE_EFI}"

SRC_URI += "file://qcom-postinstall-abota-a.sh file://qcom-postinstall-abota-b.sh"

# Sign sw-description with CMS (PKCS#7). The swupdate binary is built with
# CONFIG_SIGNED_IMAGES=y + CONFIG_SIGALG_CMS=y (see
# recipes-support/swupdate/swupdate/qcom-swupdate.cfg), so an unsigned .swu is
# rejected on-device ("SWUpdate is built for signed images"). do_swuimage runs
# "openssl cms -sign" over sw-description and bundles sw-description.sig into the
# .swu; the device verifies it against the dev cert installed at
# /etc/swupdate/qcom-swupdate-dev.cert.pem.
#
# The cert/key are read directly from this layer's files/ dir (located via
# FILESPATH), NOT fetched through SRC_URI. In an image recipe SRC_URI file://
# artifacts unpack into ${S} (= ${UNPACKDIR}), but image.bbclass declares
# do_rootfs[cleandirs] += "${S}", so do_rootfs wipes anything unpacked there
# long before do_swuimage runs -- the cert/key would vanish. Reading them from
# the stable layer path sidesteps that, and keeping them out of SRC_URI also
# means they can never be bundled into the .swu cpio (the private key must
# never ship inside an update package).
#
# The DEVELOPMENT dev public cert ships in this layer's files/, but the matching
# private key does NOT -- it is git-ignored (see .gitignore) so it never enters
# history, and must be supplied locally at files/qcom-swupdate-dev.key.pem to run
# a signing build. DEVELOPMENT ONLY -- for release builds, override
# SWUPDATE_CMS_KEY and SWUPDATE_CMS_CERT from a private/product layer or CI secret
# path and install only the matching public certificate on the target.
SWUPDATE_SIGNING = "CMS"
SWUPDATE_CMS_CERT ?= "${@bb.utils.which(d.getVar('FILESPATH'), 'qcom-swupdate-dev.cert.pem')}"
SWUPDATE_CMS_KEY ?= "${@bb.utils.which(d.getVar('FILESPATH'), 'qcom-swupdate-dev.key.pem')}"

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
