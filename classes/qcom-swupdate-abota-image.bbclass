inherit swupdate-image

SWUPDATE_IMAGES = "system.img"

SRC_URI += "file://qcom-postinstall-abota-a.sh file://qcom-postinstall-abota-b.sh"

do_generate_ext4_image() {
    install -d ${IMAGE_ROOTFS}/efi
}
addtask generate_ext4_image after do_rootfs before do_image_complete

do_deploy_ext4_image() {
    install -m 0644 ${IMGDEPLOYDIR}/${IMAGE_LINK_NAME}.ext4 ${DEPLOY_DIR_IMAGE}/system.img
    install -m 0644 ${IMGDEPLOYDIR}/${IMAGE_LINK_NAME}.ext4 ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.qcomflash/system.img
}
addtask deploy_ext4_image after do_image_complete before do_build

addtask swuimage after do_deploy_ext4_image
