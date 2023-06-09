import sys
PROJECT_NAME = f"{sys.argv[1]}"
INCLUDE_DIR = f"{sys.argv[2]}"
SOURCE_DIR = f"{sys.argv[3]}"


import abbreviations, languages, titleID, gameSpecific, codeData

def main():
    abbreviations.main()
    languages.main()
    titleID.main()
    gameSpecific.main()
    codeData.main()
    


if __name__ == '__main__':
    main()