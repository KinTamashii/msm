
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$SCRIPT_DIR/../util.sh"






function msm_coded() {
    gdir=$SCRIPT_DIR/$1/coded;shift;
    decodeargs=$1;shift;
    encodeargs=$1;shift;
    srcdir=$1;shift;
    destdir=$1;shift;
    firstdir="$gdir/'1 - decode'";
    seconddir="$gdir/'2 - encode'";
    thirddir="$gdir/'3 - redecode'";
    $msm_exec -verbose                                                                                \
        `callcmd decode coded   "$srcdir"           $gdir/1-decode    "" $decodeargs`    \
        `callcmd encode coded   $gdir/1-decode      $gdir/2-encode    "" $encodeargs`    \
        `callcmd decode coded   $gdir/2-encode      $gdir/3-redecode  "" $decodeargs`    \
        `callcmd encode coded   $gdir/3-redecode    "$destdir"        "" $encodeargs`
    if ! diff -r $gdir/1-decode $gdir/3-redecode;
    then
        echo "Test case [type=coded, gdir=$gdir] failed!" 1>&2;
        exit -1
    fi
}

function msm_clean() {
    gdir=$SCRIPT_DIR/$1/clean;shift;
    decodeargs=$1;shift;
    encodeargs=$1;shift;
    widtharg=$1;shift;
    srcdir=$1;shift;
    destdir=$1;shift;
    firstdir="$gdir/'1 - decode'";
    seconddir="$gdir/'2 - encode'";
    thirddir="$gdir/'3 - redecode'";
    # echo $msm_exec -verbose                                                                         \
    #     `callcmd decode clean   "$srcdir"           $gdir/1-decode        "$widtharg"   $decodeargs`    \
    #     `callcmd encode clean   $gdir/1-decode      $gdir/2-encode        ""            $encodeargs`    \
    #     `callcmd decode coded   $gdir/2-encode      $gdir/3-redecode      ""            $decodeargs`    \
    #     `callcmd encode coded   $gdir/3-redecode    "$destdir"            ""            $encodeargs`
    $msm_exec -verbose                                                                         \
        `callcmd decode clean   "$srcdir"           $gdir/1-decode        "$widtharg"   $decodeargs`    \
        `callcmd encode clean   $gdir/1-decode      $gdir/2-encode        ""            $encodeargs`    \
        `callcmd decode coded   $gdir/2-encode      $gdir/3-redecode      ""            $decodeargs`    \
        `callcmd encode coded   $gdir/3-redecode    "$destdir"            ""            $encodeargs`
}


msm_coded ml4 "archive_ml4 file" "file"            \
    $citra_dump_dir/00040000000D5A00/Message/US_English/        \
    $citra_load_dir/00040000000D5A00/romfs/Message/US_English


msm_clean ml4 "archive_ml4 file" "file" \
    "widths=\"$citra_dump_dir/00040000000D5A00/Message/US_English/font_s.bcfnt,75,85:$citra_dump_dir/00040000000D5A00/Message/US_English/font.bcfnt,100,125:$citra_dump_dir/00040000000D5A00/Message/US_English/font_l.bcfnt,150\""            \
    $citra_dump_dir/00040000000D5A00/Message/US_English/        \
    $citra_load_dir/00040000000D5A00/romfs/Message/US_English

msm_coded ml3-remake "archive_bg4 file" "file" \
    $citra_dump_dir/00040000001D1400/Msg/US_en \
    $citra_load_dir/00040000001D1400/romfs/Msg/US_en


msm_clean ml3-remake "archive_bg4 file" "file" \
    "widths=\"$citra_dump_dir/00040000001D1400/Msg/US_en/font.bffnt,100,-1\""            \
    $citra_dump_dir/00040000001D1400/Msg/US_en        \
    $citra_load_dir/00040000001D1400/romfs/Msg/US_en



msm_coded ml1-remake "archive_bg4 file" "file" \
    $citra_dump_dir/00040000001B8F00/Msg/US_en \
    $citra_load_dir/00040000001B8F00/romfs/Msg/US_en

msm_clean ml1-remake "archive_bg4 file" "file" \
    "widths=\"$citra_dump_dir/00040000001B8F00/Msg/US_en/font.bffnt,100,-1\""            \
    $citra_dump_dir/00040000001B8F00/Msg/US_en \
    $citra_load_dir/00040000001B8F00/romfs/Msg/US_en


msm_coded ml5 "archive_bg4 file" "file" \
    $citra_dump_dir/0004000000132700/Msg/US_en \
    $citra_load_dir/0004000000132700/romfs/Msg/US_en


msm_clean ml5 "archive_bg4 file" "file" \
    "widths=\"$citra_dump_dir/0004000000132700/Msg/US_en/font.bffnt,100,-1\""            \
    $citra_dump_dir/0004000000132700/Msg/US_en \
    $citra_load_dir/0004000000132700/romfs/Msg/US_en
#
echo "COMPLETE"