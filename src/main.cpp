#include <filesystem>
#include <msm/width.hpp>
#include <msm/languages.hpp>

#include <msm/encode/coded.hpp>
#include <msm/encode/clean.hpp>



#include <msm/decode/coded.hpp>
#include <msm/decode/clean.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////




#include <msm/languages.hpp>
#include <ktu/algorithm.hpp>
#include <ktu/memory.hpp>
#include <msm/width.hpp>
#include <ktu/string.hpp>
#include <ktu/macros/inline.hpp>
#include <fmt/format.h>
#include <msm/argparse.hpp>
#include <ktu/bit.hpp>
#include <map>








#define EXIT(...) \
    {fmt::print(__VA_ARGS__); exit(-1);}




bool matchesExtension(const char *src, const std::string &match) {

    if (!src) return true;
    for (const char *ptr = src, *last; *ptr;) {
        last = ptr + (*ptr == '.');
        while (*ptr && *ptr++ != ':');
        if (!strncmp(last, match.c_str() + 1, match.size()-1)) return true;
    }
    return false;
}


class base {
    protected:
        struct args_t {
            private:
                char *buffer[8] {(char*)"",(char*)"",(char*)"",(char*)"",(char*)"",(char*)"",(char*)"",(char*)""};
            public:
                static constexpr size_t size = 8;
                char *&operator[](size_t index) {
                    return buffer[index];
                }
                KTU_INLINE void clear() {
                    for (auto &arg : buffer) {
                        arg = (char*)"";
                    }
                }
                KTU_INLINE_REFERENCE(input,     buffer[0])
                KTU_INLINE_REFERENCE(output,    buffer[1])
                KTU_INLINE_REFERENCE(format,    buffer[2])
                KTU_INLINE_REFERENCE(external,  buffer[3])
                KTU_INLINE_REFERENCE(id,        buffer[4])
                KTU_INLINE_REFERENCE(widths,    buffer[5])
                KTU_INLINE_REFERENCE(language,  buffer[6])
                KTU_INLINE_REFERENCE(encodeCleanFlags,  buffer[7])
        } args;

        struct reusePath_t {
            enum {
                output      = ktu::field(0,                          1),
                format      = ktu::field(ktu::highest_bit(output)+1, 1),
                external    = ktu::field(ktu::highest_bit(format)+1, 1)
            };
            static const char * string [8];
        };
        int reusePath = 0;

        struct function {
            
            struct field {
                enum {
                    clean           =  ktu::field(0,                              1),
                    archive         =  ktu::field(ktu::highest_bit(clean)+1,      2),
                    group           =  ktu::field(ktu::highest_bit(archive)+1,    2),
                };
            };
            struct value {
                enum {
                    null            =  0,
                    coded           =  ktu::field_value(field::clean,   0),
                    clean           =  ktu::field_value(field::clean,   1),
                    file            =  ktu::field_value(field::archive, 0),
                    archiveML4      =  ktu::field_value(field::archive, 1),
                    archiveBG4      =  ktu::field_value(field::archive, 2),
                    encode          =  ktu::field_value(field::group,   0),
                    decode          =  ktu::field_value(field::group,   1),
                    debug           =  ktu::field_value(field::group,   2),
                    usage           =  ktu::field_value(field::group,   3),
                };
            };
        };
        int function;

        struct parameters_t {
            private:
                static constexpr size_t size = 8;
                char *buffer [size*2];
            public:
                KTU_INLINE void assign(void *src) {
                    memcpy(&buffer[0], src, sizeof(const char*)*size);
                }

                KTU_INLINE char **begin() {
                    return &buffer[0];
                }
                KTU_INLINE char **end() {
                    return &buffer[size];
                }

                KTU_INLINE_ALIAS(input,     buffer[0])
                KTU_INLINE_ALIAS(output,    buffer[1])
                KTU_INLINE_ALIAS(format,    buffer[2])
                KTU_INLINE_ALIAS(external,  buffer[3])
                KTU_INLINE_ALIAS(id,        buffer[4])
                KTU_INLINE_ALIAS(width,     buffer[5])
                KTU_INLINE_ALIAS(language,  buffer[6])
                KTU_INLINE_ALIAS(encodeCleanFlags,  buffer[7])

                KTU_INLINE void next() {
                    for (
                        char 
                            **c_it = &buffer[0],     **c_end = c_it+size,
                            **n_it = &buffer[size], **n_end = n_it+size; 
                        c_it != c_end && n_it != n_end;   
                        ++c_it, ++n_it
                    ) {
                        *n_it = splitArgNextParameter(*c_it);
                    }
                }
                KTU_INLINE void cur() {
                    memcpy(&buffer[0], &buffer[size], sizeof(const char*)*size);
                }

                KTU_INLINE const char* operator[](size_t index) {
                    return buffer[index];
                }

                KTU_INLINE operator bool() {
                    return *buffer[0];
                }
        } parameters;


        struct altMap_t {
            struct field {
                enum {
                    output     = ktu::field(0,1),
                    format     = ktu::field(ktu::highest_bit(output)+1,1),
                    external   = ktu::field(ktu::highest_bit(format)+1,1),
                };
            };
            struct value {
                enum : bool {input, output};
            };
            
        };

        struct paths_t {
            private:
                std::filesystem::path buffer[4];
            public:
                KTU_INLINE auto &operator[](size_t index) {return buffer[index];}
                KTU_INLINE_REFERENCE(input,              buffer[0])
                KTU_INLINE_REFERENCE(output,             buffer[1])
                KTU_INLINE_REFERENCE(format,             buffer[2])
                KTU_INLINE_REFERENCE(external,           buffer[3])
        } paths;

        
        struct directories_t {
            private:
                std::filesystem::path buffer[3];
            public:
                KTU_INLINE auto &operator[](size_t index) {return buffer[index];}
                KTU_INLINE_REFERENCE(output,             buffer[0])
                KTU_INLINE_REFERENCE(format,             buffer[1])
                KTU_INLINE_REFERENCE(external,           buffer[2])
        } directories;


        struct {
            private:
                const char* array[4] {"","","",""};
            public:
                KTU_INLINE auto operator[](size_t index) {
                    return array[index];
                }
                KTU_INLINE_REFERENCE(in,          array[0])
                KTU_INLINE_REFERENCE(out,         array[1])
                KTU_INLINE_REFERENCE(format,      array[2])
                KTU_INLINE_REFERENCE(external,    array[3])
        } extensions;
};

const char *base::reusePath_t::string[] {"", "output", "format", "output,format", "external", "output,external", "format,external", "output,format,external"};


void usage() {
    std::cout << R"(Usage
    Commands
        About
            Begins with the command name as the current argument.
            Template arguments which tell the correct function to
                use are assigned with an equal sign `=` directly
                after the command name, with each argument
                separated with a comma `,`.
            Each function parameter is passed in subsequent arguments.
                The parameter that each argument is assigning can be
                determined by prefacing the value with the name of the
                desired parameter, with an equal sign `=` separating the
                parameter name and value.
                If unspecified, the corresponding parameter of each 
                argument is determined by the relative index of the
                parameter to the command name.
                Any argument containing commas (if not surrounded by
                quotations), will be considered a list of the associated
                parameter to be used in multiple calls of the command.


        List
            debug                   For testing the command line interface.
            decode                  Decode a file.
            encode                  Encode a file.

        Template Arguments
            archive_bg4             Target file is a BG4 archive.
            archive_ml4             Target file is a FMes.dat or BMes.dat from
                                        Mario and Luigi: Dream Team.
            clean                   Decode/encode to/from clean text.
            coded                   Decode/encode to/from markup format.
            file                    Target file is an msbt file.
        
        Parameters [Sorted by Index]
            input                   Input file or directory.
            output                  Output file or directory.
            format                  Format file, used for clean decoding/encoding.
            external                External file containing archive information.
                                        (Used with archive_ml4)
            id                      Game ID, a 64-bit number.
            widths                  BCFNT/BFFNT font file(s), used for clean decoding/encoding.
                The format is as follows: [Quotes are necessary if commas are present.]
                    "dir1,defaultSize1,maxSize1:dir2,defaultSize2,maxSize2:...:dirN,defaultSizeN,maxSizeN"
            
            encodecleanflags=            
                autopage            Specifiy whether or not to ignore page codes present in the
                autopage~               format file, so that the placement of page codes is
                                        redetermined.
                limitlines           Specify whether or not to limit the number of lines per page.
                limitlines~
                maxlines             Specify whether or not to use the maximum number of
                maxlines~                lines per page of the strings in the file.
                maxwidth            Specify whether or not to use the maximum width
                maxwidth~                per line of the strings in the file.

            language                Language used for detecting abbreviations during clean encoding.



    Options
        About
            Any argument that begins with a hyphen `-` is considered
                an option, and can be assigned a value with an equal
                sign `=` followed by the value to assign.
            Options will not affect the relative index of function
                parameter arguments.
            If the hyphen character `-` is not followed by a option name,
                i.e. the argument string only contains a hyphen,
                the last command will be executed with any associated
                parameters, and the next non option argument will be 
                a command name.
        List
            next                An alias for the sole hyphen character.
                                    The next non option argument will be
                                    a command name.    
            verbose             Set verbose to true. Relevant information
                                    will be printed to the standard output,
                                    during the execution.
            verbose~            Set verbose to false.
            reusepath           Reuse the last element in each parameter lists
                                    if the list is shorter than the longest parameter list.
                                    The value to the right of the equal sign character `=`,
                                    is the name of the parameter to apply this functionality
                                    to. A tilde `~` character is added to the end if disabling
                                    this for a particular parameter.
                List of values to assign
                    output
                    output~
                    format
                    format~
                    external
                    external~

        Code List               Codes are separated by semicolon `:`,
                                    with the group and type of each code
                                    separated by `,` comma.
                                        Ex: `-wait=1,2:3,4:5,6`

                                    The first code is used for conversion
                                    from codename to codevalue.
                                    The code values are cleared if the 
                                    code parameter is not a valid integer
                                    value.
                                        Ex: `-option=None`

            hset                Override the associated values for the `hset` code.
            hspace              Override the associated values for the `hset` code.
            option              Override the associated values for the `option` code.
            wait                Override the associated values for the `wait` code.

    

    Macros
        Any quoted text inside parameter values can be used to assign macros.
        @{macro=value}        Assigns the value to the macro as well as inserting it.
        #{macro=value}        Assigns the value to the macro without inserting it.
        @{macro}              Inserts the value of the macro.
)";
}

class main : base {
    public:
        main(int argc, char **argv);
        void getFunction(char *arg);
        bool getFunctionInfo();
        void parseArguments();
        void execute(const std::filesystem::path &input, bool display);
        void setCode(char *postAssign);
        operator int() {
            return status;
        }
    private:
        int status = 0;
        std::map<std::string, std::string> vars;

        void *fn;
        uint8_t altMap;
        const char *description = "";

        title::id id;
        width width;
        language language = language::English;
        encodeCleanFlags_t encodeCleanFlags;
        bool verbose = false; 
};



void main::getFunction(char *arg) {
    char * postAssign = splitArgAssignment(arg);
    
    switch (ktu::hash(arg)) {
        case ktu::hash("encode"):
            function = function::value::encode;
            break;
        case ktu::hash("decode"):
            function = function::value::decode;
            break;
        case ktu::hash("debug"):
            function = function::value::debug;
            break;
        case ktu::hash("usage"):
        case ktu::hash("help"):
            function = function::value::usage;
            break;
        default:
            EXIT("Invalid function!")
    }
    if (*postAssign) {
        char *curParameter = postAssign, *nextParameter;
        while (*curParameter) {
            nextParameter = splitArgNextParameter(curParameter);
            switch (ktu::hash(curParameter)) {
                case ktu::hash("clean"):
                    function &= ~function::field::clean;
                    function |= function::value::clean;
                    break;
                case ktu::hash("coded"):
                    function &= ~function::field::clean;
                    function |= function::value::coded;
                    break;
                case ktu::hash("file"):
                    function &= ~function::field::archive;
                    function |= function::value::file;
                    break;
                case ktu::hash("archive_ml4"):
                    function &= ~function::field::archive;
                    function |= function::value::archiveML4;
                    break;
                case ktu::hash("archive_bg4"):
                    function &= ~function::field::archive;
                    function |= function::value::archiveBG4;
                    break;
            }
            curParameter = nextParameter;
        }
    }
    
}





void main::execute(const std::filesystem::path &input, bool display) {
    switch (function) {
        using value = function::value;
        using field = function::field;

        case value::decode | value::file       | value::coded:
        case value::decode | value::archiveBG4 | value::coded:
            if (display)
                fmt::print(fmt::runtime("\n  (input=\"{}\", output=\"{}\", id=0x{:016X})"),
                    input.string(),
                    paths.output().string(),
                    (uint64_t)id);
            (decltype(&decode)(fn))(input, paths.output(), id);
            break;

        case value::encode | value::file       | value::coded:
        case value::encode | value::archiveML4 | value::coded:
        case value::encode | value::archiveBG4 | value::coded:
        case value::decode | value::archiveML4 | value::coded:
            if (display) fmt::print(
                fmt::runtime("\n  (input=\"{}\", output=\"{}\", external=\"{}\", id=0x{:016X})"),
                    input.string(),
                    paths.output().string(),
                    paths.external().string(),
                    (uint64_t)id
            );
            fmt::print("[{}];\n", std::filesystem::exists(input));
            (decltype(&encode)(fn))(input, paths.output(), paths.external(), id);
            break;

        case value::decode | value::file       | value::clean:
        case value::decode | value::archiveBG4 | value::clean:   
            if (display) fmt::print(
                fmt::runtime("\n  (input=\"{}\", output=\"{}\", format=\"{}\", id=0x{:016X}, width={})"),
                    input.string(),
                    paths.output().string(),
                    paths.format().string(),
                    (uint64_t)id,
                    width
            ); 
            (decltype(&decodeClean)(fn))(input, paths.output(), paths.format(), id, width);
            break;

        case value::encode | value::file       | value::clean:
        case value::encode | value::archiveML4 | value::clean:
        case value::encode | value::archiveBG4 | value::clean:
            if (display) fmt::print(fmt::runtime("\n  (input=\"{}\", output=\"{}\", format=\"{}\", external=\"{}\", id=0x{:016X}, width={}, language=\"{}\", encodecleanflags={})"),
                input.string(), paths.output().string(), paths.format().string(), paths.external().string(), (uint64_t)id, width, language, encodeCleanFlags
            );
            (decltype(&encodeClean)(fn))(input, paths.output(), paths.format(), paths.external(), id, width, language, encodeCleanFlags);
            break;

        case value::decode | value::archiveML4 | value::clean:
            if (display) fmt::print(
                fmt::runtime("\n  (input=\"{}\", output=\"{}\", format=\"{}\", external=\"{}\", id=0x{:016X}, width={})"),
                input.string(), paths.output().string(), paths.format().string(), paths.external().string(), (uint64_t)id, width
            );
            (decltype(&decodeCleanArchiveML4)(fn))(input, paths.output(), paths.format(), paths.external(), id, width);
            break;
        
        case value::debug ... value::debug + (field::archive | field::clean):
            fmt::print(
                fmt::runtime("\n  (input=\"{}\", output=\"{}\", format=\"{}\", external=\"{}\", id=0x{:016X}, width={}, language=\"{}\", encodecleanflags={})"),
                input.string(), paths.output().string(), paths.format().string(), paths.external().string(), (uint64_t)id, width, language, encodeCleanFlags
            );
            break;
    }
}





bool main::getFunctionInfo() {
    
    switch (function) {
            
        case function::value::encode | function::value::file       | function::value::coded:
        case function::value::encode | function::value::archiveML4 | function::value::coded:
        case function::value::encode | function::value::archiveBG4 | function::value::coded:
            description = "/* Encode an msm file. */\nencode<reusePath=({})>[";
            fn = (void*)&encode;
            extensions.in() = ".msm";
            extensions.out() = "";
            extensions.external() = ".bin";

            altMap = 
                ktu::field_value(altMap_t::field::output,   altMap_t::value::input) | 
                ktu::field_value(altMap_t::field::external, altMap_t::value::output);
            break;
        case function::value::encode | function::value::file       | function::value::clean:
        case function::value::encode | function::value::archiveML4 | function::value::clean:
        case function::value::encode | function::value::archiveBG4 | function::value::clean:
            description = "/* Encode a txt file with a msf format file (and optional cwdh file). */\nencodeClean<reusePath=({})>[";
            fn = (void*)&encodeClean;
            extensions.in() = ".txt";
            extensions.out() = "";
            extensions.format() = ".msf";
            extensions.external() = ".bin";
                
            altMap = ktu::field_value(altMap_t::field::output, altMap_t::value::input) |
                ktu::field_value(altMap_t::field::format, altMap_t::value::input) |
                ktu::field_value(altMap_t::field::external, altMap_t::value::output);
            break;
        
        case function::value::decode | function::value::file       | function::value::coded:
            description = "/* Decode a msbt file into a msm file. */\ndecode<reusePath=({})>[";
            fn = (void*)&decode;
            extensions.in() = ".msbt";
            extensions.out() = ".msm";
                
            altMap = ktu::field_value(altMap_t::field::output, altMap_t::value::input);
            break;
        case function::value::decode | function::value::archiveML4 | function::value::coded:
            description = "/* Mario and Luigi Dream Team: Decode an .dat archive of msbt files into an msm file. */\ndecodeArchiveML4<reusePath=({})>[";
            fn = (void*)&decodeArchiveML4;
            extensions.in() = ".dat";
            extensions.out() = ".msm";
            extensions.external() = ".bin";
                
            altMap = ktu::field_value(altMap_t::field::output, altMap_t::value::input) |
                ktu::field_value(altMap_t::field::external, altMap_t::value::input);
            break;
        case function::value::decode | function::value::archiveBG4 | function::value::coded:
            description = "/* BG4 Compression: Decode a BG4 archive of any files into an msm file. */\ndecodeArchiveBG4<reusePath=({})>[";
            fn = (void*)&decodeArchiveBG4;
            extensions.in() = ".dat";
            extensions.out() = ".msm";

            altMap = ktu::field_value(altMap_t::field::output, altMap_t::value::input);
            break;
        
        case function::value::decode | function::value::file       | function::value::clean:
            description = "/* Decode a msbt file into a text file. */\ndecodeClean<reusePath=({})>[";
            fn = (void*)&decodeClean;
            extensions.in() = ".msbt";
            extensions.out() = ".txt";
            extensions.format() = ".msf";
             
            altMap =
                ktu::field_value(altMap_t::field::output, altMap_t::value::input) |
                ktu::field_value(altMap_t::field::format, altMap_t::value::output);
            break;
        case function::value::decode | function::value::archiveML4 | function::value::clean:
            description = "/* Mario and Luigi Dream Team: Decode an .dat archive of msbt files into a clean text file. */\ndecodeCleanArchiveML4<reusePath=({})>[";
            fn = (void*)&decodeCleanArchiveML4;
            extensions.in() = ".dat";
            extensions.out() = ".txt";
            extensions.format() = ".msf";
            extensions.external() = ".bin";
        
            altMap =
                ktu::field_value(altMap_t::field::output,   altMap_t::value::input) |
                ktu::field_value(altMap_t::field::format,   altMap_t::value::output) |
                ktu::field_value(altMap_t::field::external, altMap_t::value::input);
            break;
        case function::value::decode | function::value::archiveBG4 | function::value::clean:
            description = "/* BG4 Compression: Decode a BG4 archive of any files into a text file. */\ndecodeCleanArchiveBG4<reusePath=({})>[";
            fn = (void*)&decodeCleanArchiveBG4;
            extensions.in() = ".dat";
            extensions.out() = ".txt";
            extensions.format() = ".msf";

            altMap =
                ktu::field_value(altMap_t::field::output,   altMap_t::value::input) |
                ktu::field_value(altMap_t::field::format,   altMap_t::value::output);
            break;
        case function::value::debug ... function::value::debug + (function::field::archive | function::field::clean):
            description = (verbose) ? "/* Debug : Display each parameter. */\ndebug<verbose=true, reusePath=({})>[" :
                                      "/* Debug : Display each parameter. */\ndebug<verbose=false, reusePath=({})>[";
            extensions.in() = ".in";
            extensions.out() = ".out";
            extensions.format() = ".msf";
            extensions.external() = ".bin";

            altMap = 
                ktu::field_value(altMap_t::field::output,   altMap_t::value::input) |
                ktu::field_value(altMap_t::field::format,   altMap_t::value::input) |
                ktu::field_value(altMap_t::field::external, altMap_t::value::output);
            return true;
        
        case function::value::usage ... function::value::usage + (function::field::archive | function::field::clean):
            usage();
            return false;
    }
    return verbose;
}

void main::parseArguments() {
    parameters.assign(&args[0]);

    



    bool display = getFunctionInfo();
    
    
    if (display) fmt::print(fmt::runtime(description), reusePath_t::string[reusePath]);
    
    while (parameters) {
        parameters.next();

        if (*parameters.id()) id = title::get(parameters.id());
        else {
            for (char **pathIt = parameters.begin(), **pathEnd = pathIt + 4; pathIt != pathEnd; ++pathIt) {
                title::id tempID = title::find(*pathIt);
                id = (tempID) ? tempID : id;
            }
        }

        if (*parameters.width()) width.assign(parameters.width(), vars);
        
        if (*parameters.language()) language = parseLanguage(parameters.language());

        if (*parameters.encodeCleanFlags()) encodeCleanFlags.assign(parameters.encodeCleanFlags(), vars);//;; = stobool(parameters.encodeCleanFlags());

        paths.input().assign(parse(parameters[0], vars));

        if (std::filesystem::is_directory(paths.input())) {
            for (int i = 1; i < 4; ++i) {
                std::filesystem::path &curDir = directories[i-1];
                
                if (*parameters[i]) {
                    curDir.assign(parse(parameters[i], vars));
                } else if (!(reusePath & (1 << (i-1)))) {
                    int altShift = (i-1);
                    curDir.assign(
                        ((altMap & (1 << altShift)) >> altShift) ?
                            directories.output() : paths.input()
                    );
                }
                curDir = std::filesystem::is_regular_file(curDir) ?
                    curDir.has_parent_path() ?
                        curDir.parent_path() : ""
                    : curDir.replace_extension();
            }
            
            for (const auto &entry : std::filesystem::recursive_directory_iterator(paths.input())) {
                if (!(entry.is_regular_file() && matchesExtension(extensions.in(), entry.path().extension().string()))) continue;
                
                for (int i = 1; i < 4; ++i) {
                    std::filesystem::path &curPath = paths[i], &curDir = directories[i-1];
                    (curPath = curDir / entry.path().filename()).replace_extension(extensions[i]);
                }
                
                execute(entry, display);
                next:;
            }
        } else {
            if (!paths.input().has_extension()) paths.input().replace_extension(extensions.in());
            for (int i = 1; i < 4; ++i) {
                std::filesystem::path &curPath = paths[i];
                int altShift = (i-1);
                const std::filesystem::path &altPath = paths[(altMap & (1 << altShift)) >> altShift];
                
                if (*parameters[i]) {
                    curPath.assign(parse(parameters[i], vars));
                    if (!curPath.has_filename())  {
                        curPath.replace_filename(altPath.filename());
                        curPath.replace_extension(extensions[i]);
                        continue;
                    }
                    if (std::filesystem::is_directory(curPath)) {
                        curPath /= altPath.filename();
                        curPath.replace_extension(extensions[i]);
                        continue;
                    }
                    if (!curPath.has_extension()) curPath.replace_extension(extensions[i]);
                } else if (!(reusePath & (1 << (i-1)))) {
                    curPath.assign(altPath);
                    curPath.replace_extension(extensions[i]);
                }
            }
            execute(paths.input(), display);
        }

        parameters.cur();
    }
    if (display) fmt::print("\n]\n");
    args.clear();
}






main::main(int argc, char **argv) {
    for (auto it = argv+1, end = argv+argc; it != end; ++it) {
        
        if (**it == '-') {
            char *postAssign = splitArgAssignment(*it);
            switch (ktu::hash((*it)+1)) {
                case ktu::hash("verbose"):
                    verbose = true;
                    break;
                case ktu::hash("verbose~"):
                    verbose = false;
                    break;
                case ktu::hash("hset"):
                case ktu::hash("hspace"):
                case ktu::hash("option"):
                case ktu::hash("wait"):
                    title::codeOverride.set((*it)+1, postAssign);
                    break;
            }
            continue;
        }
        getFunction(*it++);
        
        char **internal_args_it = &args[0], **internal_args_end = internal_args_it + args.size;
        for (char *arg = *it; true; arg = *(++it)) {
            if (it == end) {
                parseArguments();
                goto eol;
            }
            char *postAssign = splitArgAssignment(arg);


            if (*arg == '-') {
                switch (ktu::hash(arg+1)) {
                    case ktu::hash(""):
                    case ktu::hash("next"):
                        goto cont;
                    case ktu::hash("verbose"):
                        verbose = true;
                        break;
                    case ktu::hash("verbose~"):
                        verbose = false;
                        break;
                    case ktu::hash("reusepath"): {
                        char *curParameter = postAssign, *nextParameter;
                        while (*curParameter) {
                            nextParameter = splitArgNextParameter(curParameter);
                            switch (ktu::hash(curParameter)) {
                                case ktu::hash("output"):
                                    reusePath |= reusePath_t::output;
                                    break;
                                case ktu::hash("output~"):
                                    reusePath &= ~reusePath_t::output;
                                    break;
                                case ktu::hash("format"):
                                    reusePath |= reusePath_t::format;
                                    break;
                                case ktu::hash("format~"):
                                    reusePath &= ~reusePath_t::format;
                                    break;
                                case ktu::hash("external"):
                                    reusePath |= reusePath_t::external;
                                    break;
                                case ktu::hash("external~"):
                                    reusePath &= ~reusePath_t::external;
                                    break;
                            }
                            curParameter = nextParameter;
                        }
                        break;
                    }
                    case ktu::hash("hset"):
                    case ktu::hash("hspace"):
                    case ktu::hash("option"):
                    case ktu::hash("wait"):
                        title::codeOverride.set((*it)+1, postAssign);
                        break;
                }
                continue;
            }
            
            
            
            if (*postAssign) {
                fmt::print("<{}>\n", arg);
                switch (ktu::hash(arg)) {
                    case ktu::hash("input"):
                        args.input() = postAssign;
                        break;
                    case ktu::hash("output"):
                        args.output() = postAssign;
                        break;
                    case ktu::hash("format"):
                        args.format() = postAssign;
                        break;
                    case ktu::hash("external"):
                        args.external() = postAssign;
                        break;
                    case ktu::hash("id"):
                        args.id() = postAssign;
                        break;
                    case ktu::hash("widths"):
                        args.widths() = postAssign;
                        break;
                    case ktu::hash("encodecleanflags"):
                        args.encodeCleanFlags() = postAssign;
                        break;
                    case ktu::hash("language"):
                        args.language() = postAssign;
                        break;
                }
                continue;
            }

            if (internal_args_it == internal_args_end) continue;
            *internal_args_it++ = arg;
        }
        
        cont:;
        parseArguments();
    }
    eol:;
}





int main(int argc, char **argv) {
    class main main(argc, argv);

    return main;
}
