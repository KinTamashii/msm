
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"


$msm_exec -verbose \
    `callcmd decode coded $citra_dump_dir/00040000001D1400/Msg/US_en $SCRIPT_DIR/coded "" "archive_bg4 file"`