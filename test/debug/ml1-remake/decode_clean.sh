
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"


$msm_exec -verbose \
    `callcmd decode clean $citra_dump_dir/00040000001B8F00/Msg/US_en $SCRIPT_DIR/clean "widths=\"/Users/kin_tamashii/.local/share/citra-emu/dump/romfs/00040000001B8F00/Msg/US_en/font.bffnt,100,-1\"" "archive_bg4 file"`