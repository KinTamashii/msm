
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../../util.sh"

$msm_exec -verbose \
    encode \
        $SCRIPT_DIR/0-source.msm \
        $SCRIPT_DIR/1-encoded.msbt \
        id=ML5 \
        - \
    decode \
        $SCRIPT_DIR/1-encoded.msbt \
        $SCRIPT_DIR/2-valid_source.msm \
        - \
    encode \
        $SCRIPT_DIR/2-valid_source.msm \
        $SCRIPT_DIR/3-valid_encoded.msm \
        - \
    decode=clean \
        $SCRIPT_DIR/3-valid_encoded.msm \
        $SCRIPT_DIR/4-clean.txt \
        "widths=\"/Users/kin_tamashii/.local/share/citra-emu/dump/romfs/00040000001D1400/Msg/US_en/font.bffnt,100,-1\"" \
        - \
    encode=clean \
        $SCRIPT_DIR/4-clean-alt.txt \
        $SCRIPT_DIR/5-clean_encoded.msbt \
        format=$SCRIPT_DIR/4-clean.msf \
        - \
    decode \
        $SCRIPT_DIR/5-clean_encoded.msbt \
        $SCRIPT_DIR/6-decoded_result.msm