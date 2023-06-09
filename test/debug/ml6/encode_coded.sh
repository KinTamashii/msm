
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"


$msm_exec -verbose \
    `callcmd encode coded $SCRIPT_DIR/coded $citra_load_dir/00040000001D1400/romfs/Msg/US_en "" "archive_bg4 file"`