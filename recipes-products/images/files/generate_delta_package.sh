#!/bin/bash
# Build an rdiff- or zchunk-based delta .swu from a set of images, each
# present in both a "source" (currently-installed) and "dest" (target)
# build directory. Picks its sw-description verbatim from --scripts
# (sw-description-diff for --mode differential, sw-description-stream for
# --mode stream) and bundles the qcom-postinstall-abota.sh script
# from the same directory, so the package flips the active slot itself,
# like a production OTA package.
#
# Usage:
#   ./generate_delta_package.sh --output OUT.swu \
#       --source-dir DIR --dest-dir DIR \
#       --scripts DIR \
#       [--mode differential|stream] [--images "img1 img2 ..."] \
#       [--machine MACHINE] [--delta-url URL] \
#       [--sign --cert CERT --key KEY]
#
# --images is a space-separated list of image basenames (as they appear in
# both --source-dir and --dest-dir). Defaults to "system.img efi.bin".
# The partlabel used on-device is the image basename without its extension
# (e.g. "system.img" -> partlabel "system"), matching sw-description-diff /
# sw-description-stream.
#
# --mode differential (default) computes an rdiff patch per image and picks
# sw-description-diff from --scripts.
# --mode stream generates a zchunk file per image (embedding only the
# .zck.header in the .swu, writing the full .zck next to the output .swu
# for hosting on a delta-update server) and picks sw-description-stream
# from --scripts. Requires --delta-url.
#
# --sign CMS-signs sw-description and bundles the resulting sw-description.sig
# as the second cpio member, so the package verifies on a target built with
# CONFIG_SIGNED_IMAGES=y. The cert/key are picked from --cert/--key -- any
# filename works, there is no fixed/required name. Use the same keypair the
# ABOTA image build signs with (SWUPDATE_CMS_CERT/SWUPDATE_CMS_KEY in
# qcom-swupdate-abota-image.bbclass) and that matches the public cert
# installed on-device (qcom-swupdate-runtime.conf's public-key-file), or a
# --sign package won't verify. The private key is a signing input only and is
# never bundled into the .swu.
#
# This script lives in the same dir as the sw-description templates and the
# qcom-postinstall-abota.sh script, so --scripts is simply the
# directory it sits in (use --scripts . when run from here).
#
# Example (differential, default images, signed):
#   ./generate_delta_package.sh --output ota-delta.swu --sign \
#       --cert /path/to/cert.pem --key /path/to/key.pem \
#       --source-dir /path/to/old/build --dest-dir /path/to/new/build \
#       --scripts .
#
# Example (stream mode):
#   ./generate_delta_package.sh --output ota-stream.swu --mode stream --sign \
#       --cert /path/to/cert.pem --key /path/to/key.pem \
#       --source-dir /path/to/old/build --dest-dir /path/to/new/build \
#       --scripts . \
#       --delta-url https://update.example.com/qcs8300-ride-sx/

set -euo pipefail

# Yocto's native zck build supports -u/--chunk-hash-type, matching the flags
# the production do_generate_delta task uses; the distro-packaged zck on this
# host is often older and lacks them. Set ZCHUNK_NATIVE_BINDIR (e.g. export
# ZCHUNK_NATIVE_BINDIR=/path/to/build/tmp/sysroots-components/x86_64/zchunk-native/usr/bin)
# to point at the native build; falls back to whatever zck is on PATH otherwise.
ZCHUNK_NATIVE_BINDIR="${ZCHUNK_NATIVE_BINDIR:-}"
if [ -n "$ZCHUNK_NATIVE_BINDIR" ] && [ -x "$ZCHUNK_NATIVE_BINDIR/zck" ] && [ -x "$ZCHUNK_NATIVE_BINDIR/zck_read_header" ]; then
    ZCK="$ZCHUNK_NATIVE_BINDIR/zck"
    ZCK_READ_HEADER="$ZCHUNK_NATIVE_BINDIR/zck_read_header"
else
    ZCK="zck"
    ZCK_READ_HEADER="zck_read_header"
fi

usage() {
    echo "Usage: $0 --output OUT.swu --source-dir DIR --dest-dir DIR --scripts DIR [--mode differential|stream] [--images \"img1 img2 ...\"] [--machine MACHINE] [--delta-url URL] [--sign --cert CERT --key KEY]" >&2
    exit 1
}

MACHINE="qcs8300-ride-sx"
OUTPUT_SWU=""
SCRIPTS_DIR=""
SOURCE_DIR=""
DEST_DIR=""
MODE="differential"
IMAGES="system.img efi.bin"
DELTA_URL=""
SIGN=0
CMS_CERT=""
CMS_KEY=""

while [ $# -gt 0 ]; do
    case "$1" in
        --output)
            OUTPUT_SWU="${2:?--output requires a value}"
            shift 2
            ;;
        --source-dir)
            SOURCE_DIR="${2:?--source-dir requires a value}"
            shift 2
            ;;
        --dest-dir)
            DEST_DIR="${2:?--dest-dir requires a value}"
            shift 2
            ;;
        --scripts)
            SCRIPTS_DIR="${2:?--scripts requires a value}"
            shift 2
            ;;
        --mode)
            MODE="${2:?--mode requires a value}"
            shift 2
            ;;
        --images)
            IMAGES="${2:?--images requires a value}"
            shift 2
            ;;
        --machine)
            MACHINE="${2:?--machine requires a value}"
            shift 2
            ;;
        --delta-url)
            DELTA_URL="${2:?--delta-url requires a value}"
            shift 2
            ;;
        --sign)
            SIGN=1
            shift
            ;;
        --cert)
            CMS_CERT="${2:?--cert requires a value}"
            shift 2
            ;;
        --key)
            CMS_KEY="${2:?--key requires a value}"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: unknown argument: $1" >&2
            usage
            ;;
    esac
done

[ -n "$OUTPUT_SWU" ] || usage
[ -n "$SOURCE_DIR" ] || usage
[ -n "$DEST_DIR" ] || usage
[ -n "$SCRIPTS_DIR" ] || usage

case "$MODE" in
    differential|stream) ;;
    *)
        echo "Error: --mode must be 'differential' or 'stream', got: $MODE" >&2
        exit 1
        ;;
esac

if [ "$MODE" = "stream" ] && [ -z "$DELTA_URL" ]; then
    echo "Error: --mode stream requires --delta-url" >&2
    exit 1
fi

if [ "$MODE" = "differential" ]; then
    SW_DESCRIPTION_TEMPLATE="$SCRIPTS_DIR/sw-description-diff"
else
    SW_DESCRIPTION_TEMPLATE="$SCRIPTS_DIR/sw-description-stream"
fi
POSTINSTALL="$SCRIPTS_DIR/qcom-postinstall-abota.sh"
for f in "$SW_DESCRIPTION_TEMPLATE" "$POSTINSTALL"; do
    if [ ! -f "$f" ]; then
        echo "Error: required file not found: $f" >&2
        exit 1
    fi
done

# --sign requires an explicit --cert/--key pair -- a public cert is only
# meaningful paired with the matching private key, so there is no default
# path to fall back to. Use the same keypair the ABOTA image build signs
# with (SWUPDATE_CMS_CERT/SWUPDATE_CMS_KEY in qcom-swupdate-abota-image.bbclass)
# and that matches the public cert installed on-device, or a --sign package
# won't verify. The key is only an input to openssl cms and is never bundled
# into the .swu.
if [ "$SIGN" -eq 1 ]; then
    [ -n "$CMS_CERT" ] || { echo "Error: --sign requires --cert" >&2; exit 1; }
    [ -n "$CMS_KEY" ] || { echo "Error: --sign requires --key" >&2; exit 1; }
    for f in "$CMS_CERT" "$CMS_KEY"; do
        if [ ! -f "$f" ]; then
            echo "Error: --sign requires signing material, not found: $f" >&2
            exit 1
        fi
    done
fi

OUTPUT_SWU="$(realpath -m "$OUTPUT_SWU")"
OUTPUT_DIR="$(dirname "$OUTPUT_SWU")"
mkdir -p "$OUTPUT_DIR"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

declare -a IMAGE_NAMES=()
read -r -a IMAGE_NAMES <<< "$IMAGES"
[ "${#IMAGE_NAMES[@]}" -gt 0 ] || { echo "Error: --images list is empty" >&2; exit 1; }

declare -a BUNDLE_FILES=()
declare -a SHA256_SED_ARGS=()

for image in "${IMAGE_NAMES[@]}"; do
    SOURCE_IMG="$SOURCE_DIR/$image"
    TARGET_IMG="$DEST_DIR/$image"
    if [ ! -f "$SOURCE_IMG" ]; then
        echo "Error: source image not found: $SOURCE_IMG" >&2
        exit 1
    fi
    if [ ! -f "$TARGET_IMG" ]; then
        echo "Error: target image not found: $TARGET_IMG" >&2
        exit 1
    fi

    NAME_NOEXT="${image%.*}"
    PLACEHOLDER="SHA256_$(echo "$NAME_NOEXT" | tr '[:lower:]' '[:upper:]')"

    if [ "$MODE" = "differential" ]; then
        RDIFF_BASENAME="${NAME_NOEXT}.rdiff"
        SIGFILE="$WORKDIR/${NAME_NOEXT}.sig"

        echo "Generating rdiff for '$image' ($SOURCE_IMG -> $TARGET_IMG)..."
        rdiff signature "$SOURCE_IMG" "$SIGFILE"
        rdiff delta "$SIGFILE" "$TARGET_IMG" "$WORKDIR/$RDIFF_BASENAME"
        rm -f "$SIGFILE"

        BUNDLE_FILES+=("$RDIFF_BASENAME")
        HASHFILE="$WORKDIR/$RDIFF_BASENAME"
    else
        ZCKFILE="$WORKDIR/${image}.zck"
        HEADERFILE="${image}.zck.header"

        echo "Generating zchunk for '$image' ($SOURCE_IMG -> $TARGET_IMG)..."
        "$ZCK" --output "$ZCKFILE" -u --chunk-hash-type sha256 "$TARGET_IMG"

        HSIZE="$("$ZCK_READ_HEADER" -v "$ZCKFILE" | grep 'Header size' | cut -d ':' -f 2 | tr -d ' ')"
        dd if="$ZCKFILE" of="$WORKDIR/$HEADERFILE" bs=1 count="$HSIZE" status=none

        cp "$ZCKFILE" "$OUTPUT_DIR/${image}.zck"
        echo "Wrote $OUTPUT_DIR/${image}.zck (host at @@SWUPDATE_DELTA_URL@@)"

        BUNDLE_FILES+=("$HEADERFILE")
        HASHFILE="$WORKDIR/$HEADERFILE"
    fi

    # sw-description's sha256 must match the bytes actually bundled into the .swu
    # (the .rdiff patch, or the .zck.header -- not the full target image), since
    # CONFIG_SIGNED_IMAGES=y makes SWUpdate's check_missing_hash() reject any
    # rdiff_image/delta entry without a valid hash before its handler ever runs.
    SHA256_VALUE="$(sha256sum "$HASHFILE" | cut -d ' ' -f1)"
    SHA256_SED_ARGS+=(-e "s|@@${PLACEHOLDER}@@|$SHA256_VALUE|g")
done

# The postinstall script is bundled into the .swu too, and under
# CONFIG_SIGNED_IMAGES=y every sw-description entry -- including scripts --
# must carry a sha256 or check_missing_hash() rejects the package
# ("Hash not set for qcom-postinstall-abota.sh"). Hash the exact bytes
# that get bundled (the script is copied verbatim below).
SHA256_POSTINST="$(sha256sum "$POSTINSTALL" | cut -d ' ' -f1)"
SHA256_SED_ARGS+=(-e "s|@@SHA256_POSTINST@@|$SHA256_POSTINST|g")

sed -e "s/@@MACHINE@@/$MACHINE/g" -e "s|@@SWUPDATE_DELTA_URL@@|$DELTA_URL|g" \
    "${SHA256_SED_ARGS[@]}" \
    "$SW_DESCRIPTION_TEMPLATE" > "$WORKDIR/sw-description"

cp "$POSTINSTALL" "$WORKDIR/$(basename "$POSTINSTALL")"

# Under CONFIG_SIGNED_IMAGES=y the target verifies a CMS signature over
# sw-description before parsing it. Produce sw-description.sig the same way
# meta-swupdate's swupdate.bbclass does (DER CMS, detached, no S/MIME caps)
# so a --sign package verifies with the on-device public cert.
SIG_MEMBER=""
if [ "$SIGN" -eq 1 ]; then
    echo "Signing sw-description with $CMS_CERT ..."
    openssl cms -sign -in "$WORKDIR/sw-description" -out "$WORKDIR/sw-description.sig" \
        -signer "$CMS_CERT" -inkey "$CMS_KEY" \
        -binary -outform DER -nosmimecap
    SIG_MEMBER="sw-description.sig"
fi

(
    cd "$WORKDIR"
    {
        echo "sw-description"
        [ -n "$SIG_MEMBER" ] && echo "$SIG_MEMBER"
        for f in "${BUNDLE_FILES[@]}"; do
            echo "$f"
        done
        basename "$POSTINSTALL"
    } | cpio -ov -H crc --reproducible > "$OUTPUT_SWU"
)

echo "Wrote $OUTPUT_SWU"
