# Message Studio Markup

A markup language for Message Studio Binary Text (MSBT) files.

This application can decode MSBT files into the Message Studio Markup (MSM) language, and encode MSM files back into MSBT.

It can also decode MSBT files into plain text accompanied with Message Studio Format (MSF) files, of which the pairs can be encoded back into MSBT.

## Dependencies

    ktutils library https://github.com/KinTamashii/ktutils

    fmt library https://github.com/fmtlib/fmt

    cmake

## Setup
After installing the dependencies, run cmake and build with the following options if necessary:

    -MSM_CERT_AUTH=${AUTHENTICATOR}.        On macOS, applications must be signed
                                                in order to be run on other
                                                computers.
    -DMSM_STATIC=${true}                    If this variable is defined, the
                                                program will compile with static
                                                linking.

## Usage
### Commands
#### About
Begins with the command name as the current argument.

Template arguments which tell the correct function to use are assigned with an equal sign `=` directly after the command name, with each argument separated with a comma `,`.

Each function parameter is passed in subsequent arguments. The parameter that each argument is assigning can be determined by prefacing the value with the name of the desired parameter, with an equal sign `=` separating the parameter name and value.

If unspecified, the corresponding parameter of each  argument is determined by the relative index of the parameter to the command name. Any argument containing commas (if not surrounded by quotations), will be considered a list of the associated parameter to be used in multiple calls of the command.
#### List
    debug                   For testing the command line interface.
    decode                  Decode a file.
    encode                  Encode a file.

#### Template Arguments
    archive_bg4             Target file is a BG4 archive.
    archive_ml4             Target file is a FMes.dat or BMes.dat from
                                Mario and Luigi: Dream Team.
    clean                   Decode/encode to/from clean text.
    coded                   Decode/encode to/from markup format.
    file                    Target file is an msbt file.

#### Parameters [Sorted by Index]
    input                   Input file or directory.
    output                  Output file or directory.
    format                  Format file, used for clean decoding/encoding.
    external                External file containing archive information.
                                (Used with archive_ml4)
    id                      Game ID, a 64-bit number.
    widths                  BCFNT/BFFNT font file(s), used for clean
                                decoding/encoding. The format is as follows:
                                [Quotes are necessary if commas are present.]
                                "dir1,defaultSize1,maxSize1:dir2,defaultSize2,
                                maxSize2:...:dirN,defaultSizeN,maxSizeN"
    
    encodecleanflags=            
        autopage  autopage~            Specifiy whether or not to ignore page
                                            codes present in the format file,
                                            so that the placement of page codes
                                            is redetermined.
        limitlines  limitlines~           Specify whether or not to limit the
                                            number of lines per page.
        maxlines  maxlines~            Specify whether or not to use the
                                            maximum number of lines per page of
                                            the strings in the file.
        maxwidth  maxwidth~           Specify whether or not to use the maximum
                                            width per line of the strings in the
                                            file.
                        

    language                Language used for detecting abbreviations during
                                clean encoding.



### Options
#### About
Any argument that begins with a hyphen `-` is considered an option, and can be assigned a value with an equal sign `=` followed by the value to assign. Options will not affect the relative index of function parameter arguments. 

If the hyphen character `-` is not followed by a option name, i.e. the argument string only contains a hyphen, the last command will be executed with any associated parameters, and the next non option argument will be a command name.
#### List
    next                An alias for the sole hyphen character.
                            The next non option argument will be
                            a command name.    
    verbose             Set verbose to true. Relevant information
                            will be printed to the standard output,
                            during the execution.
    verbose~            Set verbose to false.
    reusepath           Reuse the last element in each parameter
                            lists if the list is shorter than the
                            longest parameter list. The value to
                            the right of the equal sign character
                            `=`, is the name of the parameter to
                            apply this functionality to. A tilde
                            `~` character is added to the end if
                            disabling this for a particular
                            parameter.
    List of values to assign
        output  output~
        format  format~
        external  external~

#### Code List
Codes are separated by semicolon `:`, with the group and type of each code separated by `,` comma. Ex: `-wait=1,2:3,4:5,6`

The first code is used for conversion from codename to codevalue.

The code values are cleared if the  code parameter is not a valid integer value. Ex: `-option=None`

    hset                Override the associated values for the `hset` code.
    hspace              Override the associated values for the `hset` code.
    option              Override the associated values for the `option` code.
    wait                Override the associated values for the `wait` code.

    

### Macros
    Any quoted text inside parameter values can be used to assign macros.
    @{macro=value}        Assigns the value to the macro as well as inserting it.
    #{macro=value}        Assigns the value to the macro without inserting it.
    @{macro}              Inserts the value of the macro.

## Visual Studio Code Extension
Bundled in releases is the Visual Studio Code Extension for MSM, which provides syntax highlighting for the language.
### .vscode Config
To configure the extension, the following options are present in .vscode
#### msm.executablePath
The path to the msm executable.
#### msm.decode
  - coded
    - specification
    - input
    - output
    - external
    - id
  - clean
    - specification
    - input
    - output
    - format
    - external
    - id
    - widths
#### msm.encode
  - coded
    - specification
    - input
    - output
    - external
    - id
    - widths
  - clean
     - specification
    - input
    - output
    - format
    - external
    - id
    - widths
    - encodecleanflags
    - language
#### msm.flags
  - -verbose
  - -verbose~
  - -hset
  - -hspace
  - -option
  - -wait
  - -reusepath
  
  