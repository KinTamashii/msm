
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"


$msm_exec -verbose \
    `callcmd encode clean $SCRIPT_DIR/clean $citra_load_dir/0004000000132700/romfs/Msg/US_en "widths=\"$citra_dump_dir/0004000000132700/Msg/US_en/font.bffnt,100,-1\"" "archive_bg4 file"`