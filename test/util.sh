msm_exec="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/../build/msm"
citra_dump_dir=/Users/kin_tamashii/.local/share/citra-emu/dump/romfs
citra_load_dir=/Users/kin_tamashii/.local/share/citra-emu/load/mods

function callcmd() {
    cmdname=$1;shift;
    type=$1;shift;
    input=$1;shift;
    output=$1;shift;
    trailing=$1;shift;
    for arg in $@; do
        echo - $cmdname=$type,$arg "$input" "$output" $trailing
    done
}

