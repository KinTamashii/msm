
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"

$msm_exec -verbose \
    `callcmd encode clean $SCRIPT_DIR/decode "$citra_load_dir/00040000001D1400/romfs/Msg/US_en" \
        "widths=\"$citra_dump_dir/00040000001D1400/Msg/US_en/font.bffnt,100,-1\""   "file archive_bg4"`