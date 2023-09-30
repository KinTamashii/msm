case ktu::hash("option"):
    if (!(inString)) break;
    inStringH();
    switch (gameID) {
        case title::id::ML4_US:
        case title::id::ML4_EU:
        case title::id::ML4_JP:
            codeVariadicParams(5, 1);
            return;
        case title::id::ML5_US:
        case title::id::ML5_EU:
        case title::id::ML5_JP:
        case title::id::ML1_REMAKE_US:
        case title::id::ML1_REMAKE_EU:
        case title::id::ML1_REMAKE_JP:
        case title::id::ML3_REMAKE_US:
        case title::id::ML3_REMAKE_EU:
        case title::id::ML3_REMAKE_JP:
        case title::id::ML3_REMAKE_JP_VER_1_2:
            codeVariadicParams(4, 0);
            return;
        default:
            break;
    };
    break;
case ktu::hash("wait"):
    if (!(inString)) break;
    inStringH();
    switch (gameID) {
        case title::id::ML4_US:
        case title::id::ML4_EU:
        case title::id::ML4_JP:
            codeVariadicParams(3, 1);
            return;
        case title::id::ML5_US:
        case title::id::ML5_EU:
        case title::id::ML5_JP:
        case title::id::ML1_REMAKE_US:
        case title::id::ML1_REMAKE_EU:
        case title::id::ML1_REMAKE_JP:
        case title::id::ML3_REMAKE_US:
        case title::id::ML3_REMAKE_EU:
        case title::id::ML3_REMAKE_JP:
        case title::id::ML3_REMAKE_JP_VER_1_2:
            codeVariadicParams(2, 1);
            return;
        default:
            break;
    };
    break;
case ktu::hash("hspace"):
    if (!(inString)) break;
    inStringH();
    switch (gameID) {
        case title::id::ML5_US:
        case title::id::ML5_EU:
        case title::id::ML5_JP:
        case title::id::ML1_REMAKE_US:
        case title::id::ML1_REMAKE_EU:
        case title::id::ML1_REMAKE_JP:
        case title::id::ML3_REMAKE_US:
        case title::id::ML3_REMAKE_EU:
        case title::id::ML3_REMAKE_JP:
        case title::id::ML3_REMAKE_JP_VER_1_2:
            codeVariadicParams(6, 0);
            return;
        default:
            break;
    };
    break;
case ktu::hash("hset"):
    if (!(inString)) break;
    inStringH();
    switch (gameID) {
        case title::id::ML5_US:
        case title::id::ML5_EU:
        case title::id::ML5_JP:
        case title::id::ML1_REMAKE_US:
        case title::id::ML1_REMAKE_EU:
        case title::id::ML1_REMAKE_JP:
        case title::id::ML3_REMAKE_US:
        case title::id::ML3_REMAKE_EU:
        case title::id::ML3_REMAKE_JP:
        case title::id::ML3_REMAKE_JP_VER_1_2:
            codeVariadicParams(6, 1);
            return;
        default:
            break;
    };
    break;
