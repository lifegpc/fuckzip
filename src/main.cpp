#include "fuckzip_config.h"

#include <stdio.h>
#include <string>
#include <fcntl.h>
#include "getopt.h"
#include "wchar_util.h"
#include "fileop.h"
#include "err.h"
#include "cstr_util.h"
#include "zip.h"
#include "extract_zip.h"
#include "encoding.h"

#if _WIN32
#include "Windows.h"
#endif

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#if HAVE_SSCANF_S
#define sscanf sscanf_s
#endif

void print_help() {
    printf("%s", "Usage: fuckzip [options] FILE\n\
A tool that extract zip archive with a custom encoding.\n\
\n\
Options:\n\
    -h, --help          Print this message and exit.\n");
#if _WIN32
    printf("\
    -c <num>, --codepage <num>\n\
                        Specify the code page of the archive. Can not combined with -e.\n");
#endif
#if HAVE_ICONV || _WIN32
    printf("\
    -e <encoding>, --encoding <encoding>\n\
                        Specify the encoding of the archive.\n");
#endif
}

int main(int argc, char* argv[]) {
#if _WIN32
    SetConsoleOutputCP(CP_UTF8);
    bool have_wargv = false;
    int wargc;
    char** wargv;
    if (wchar_util::getArgv(wargv, wargc)) {
        have_wargv = true;
        argc = wargc;
        argv = wargv;
    }
#endif
    if (argc == 1) {
        print_help();
#if _WIN32
        if (have_wargv) wchar_util::freeArgv(wargv, wargc);
#endif
        return 0;
    }
    struct option opts[] = {
        {"help", 0, nullptr, 'h'},
#if _WIN32
        {"codepage", 1, nullptr, 'c'},
#endif
#if HAVE_ICONV || _WIN32
        {"encoding", 1, nullptr, 'e'},
#endif
        nullptr,
    };
    int c;
    std::string shortopts = "-h";
#if _WIN32
    shortopts += "c:";
#endif
#if HAVE_ICONV || _WIN32
    shortopts += "e:";
#endif
    std::string input;
    int cp = -1;
    std::string enc;
    while ((c = getopt_long(argc, argv, shortopts.c_str(), opts, nullptr)) != -1) {
        switch (c) {
        case 'h':
            print_help();
#if _WIN32
            if (have_wargv) wchar_util::freeArgv(wargv, wargc);
#endif
            return 0;
#if _WIN32
        case 'c':
            if (!cstr_is_integer(optarg, 0)) {
                printf("%s is not a non-negative integer.\n", optarg);
                if (have_wargv) wchar_util::freeArgv(wargv, wargc);
                return 1;
            }
            if (sscanf(optarg, "%i", &cp) != 1) {
                printf("Can not parse codepage.\n");
                if (have_wargv) wchar_util::freeArgv(wargv, wargc);
                return 1;
            }
            break;
#endif
#if HAVE_ICONV || _WIN32
        case 'e':
            enc = optarg;
            break;
#endif
        case 1:
            if (!input.length()) {
                input = optarg;
            } else {
                printf("%s\n", "Too much input files.");
#if _WIN32
                if (have_wargv) wchar_util::freeArgv(wargv, wargc);
#endif
                return 1;
            }
            break;
        case '?':
        default:
#if _WIN32
            if (have_wargv) wchar_util::freeArgv(wargv, wargc);
#endif
            return 1;
        }
    }
#if _WIN32
    if (have_wargv) wchar_util::freeArgv(wargv, wargc);
#endif
    if (cp != -1 && enc.length()) {
        printf("Error: Both codepage and encoding specified.\n");
        return 1;
    }
    int err;
    int flags = ZIP_FL_ENC_UTF_8;
#if _WIN32
    if (!have_wargv) flags = ZIP_FL_ENC_RAW;
#endif
    zip_t* zip = zip_open(input.c_str(), flags, &err);
    if (!zip) {
        std::string msg;
        if (!err::get_errno_message(msg, err)) {
            msg = "Unknown error.";
        }
        printf("An error occured when opening file: %s\n", msg.c_str());
        return 1;
    }
    int result = 0;
    if (!extract_zip(zip, "", cp, enc)) {
        goto end;
    }
end:
    auto zip_err = zip_get_error(zip);
    if (zip_err && (zip_err->zip_err != ZIP_ER_OK || zip_err->sys_err != 0)) printf("%s\n", zip_error_strerror(zip_err));
    if (zip_close(zip)) {
        auto err = zip_get_error(zip);
        if (err) printf("Can not close ZIP archive: %s\n", zip_error_strerror(err));
        return result ? result : 1;
    }
    return result;
}
