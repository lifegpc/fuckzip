#include "fuckzip_config.h"
#include "extract_zip.h"
#include "err.h"
#include "fileop.h"
#include "malloc.h"
#include "wchar_util.h"

#include <fcntl.h>
#if _WIN32
#include <Windows.h>
#endif

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#ifndef _O_BINARY
#define _O_BINARY 0x8000
#endif

#ifndef _SH_DENYWR
#define _SH_DENYWR 0x20
#endif

#ifndef _S_IWRITE
#define _S_IWRITE 0x80
#endif

bool extract_zip(zip_t* zip, std::string directory, int cp) {
    if (!zip) return false;
    int file_mode = 0;
#if !_WIN32
    file_mode |= (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
    auto num = zip_get_num_entries(zip, 0);
    if (num < 0) {
        printf("Can not get number of files in archive.\n");
        return false;
    }
    if (num == 0) return true;
    zip_int64_t i = 0;
    zip_flags_t flags = 0;
    if (cp != -1) flags |= ZIP_FL_ENC_RAW; else flags |= ZIP_FL_ENC_GUESS;
    for (; i < num; i++) {
        zip_stat_t stats;
        if (zip_stat_index(zip, i, flags, &stats)) {
            printf("Can not get file stats.\n");
            return false;
        }
        if (!(stats.valid & ZIP_STAT_NAME)) continue;
        std::string name = stats.name;
#if _WIN32
        if (cp != -1) {
            std::wstring tmp;
            if (!wchar_util::str_to_wstr(tmp, name, cp)) {
                printf("Can not decode name with codepage %i.\n", cp);
                return false;
            }
            if (!wchar_util::wstr_to_str(name, tmp, CP_UTF8)) {
                printf("Can not convert name to UTF-8.\n");
                return false;
            }
        }
#endif
        std::string filepath = fileop::join(directory, name);
        std::string dirpath = fileop::dirname(filepath);
#if _WIN32
        if (dirpath.length() && !fileop::isdrive(dirpath)) {
            std::string tmp = dirpath + "\\";
#else
        if (!dirpath.length()) {
            std::string tmp = dirpath + "/";
#endif
            bool re;
            if (!fileop::isdir(tmp, re)) {
                std::string errmsg;
                if (!err::get_errno_message(errmsg, errno)) {
                    errmsg = "Unknown error";
                }
                printf("Can not detect \"%s\" is a directory: %s.\n", tmp.c_str(), errmsg.c_str());
                return false;
            }
            if (!re) {
                if (!fileop::mkdir(tmp, file_mode)) {
                    printf("Can not create directory \"%s\".\n", tmp.c_str());
                    return false;
                }
            }
        }
        std::string filename = fileop::basename(filepath);
        if (!filename.length()) {
            continue;
        }
        auto zip_file = zip_fopen_index(zip, i, 0);
        if (!zip_file) {
            printf("Can not open file \"%s\" in archive.\n", name.c_str());
            return false;
        }
        int fd;
        int err = fileop::open(filepath, fd, O_CREAT | O_WRONLY | _O_BINARY, _SH_DENYWR, _S_IWRITE);
        if (err < 0) {
            std::string errmsg;
            if (!err::get_errno_message(errmsg, err)) {
                errmsg = "Unknown error";
            }
            printf("A error occured when try create file \"%s\": %s.\n", filepath.c_str(), errmsg.c_str());
            zip_fclose(zip_file);
            return false;
        }
        FILE* f = fileop::fdopen(fd, "wb");
        if (!f) {
            printf("Can not create stream from opened file.\n");
            if (!fileop::close(fd)) {
                std::string errmsg;
                if (!err::get_errno_message(errmsg, errno)) {
                    errmsg = "Unknown error";
                }
                printf("A error occured when close file \"%s\": %s.\n", filepath.c_str(), errmsg.c_str());
            }
            zip_fclose(zip_file);
            return false;
        }
        size_t num_read = 0;
        char* buff = (char*)malloc(1024);
        if (!buff) {
            printf("Out of memory.\n");
            fileop::fclose(f);
            zip_fclose(zip_file);
            return false;
        }
        do {
            num_read = zip_fread(zip_file, buff, 1024);
            fwrite(buff, 1, 1024, f);
        } while (num_read > 0);
        free(buff);
        fileop::fclose(f);
        zip_fclose(zip_file);
        if ((stats.valid & ZIP_STAT_MTIME) && !fileop::set_file_time(filepath, stats.mtime, stats.mtime, stats.mtime)) {
            printf("Can not set modified time to file \"%s\"\n", filepath.c_str());
        }
    }
    for (i = num - 1; i >= 0; i--) {
        zip_stat_t stats;
        if (zip_stat_index(zip, i, flags, &stats)) {
            printf("Can not get file stats.\n");
            return false;
        }
        if (!(stats.valid & ZIP_STAT_NAME)) continue;
        std::string name = stats.name;
#if _WIN32
        if (cp != -1) {
            std::wstring tmp;
            if (!wchar_util::str_to_wstr(tmp, name, cp)) {
                printf("Can not decode name with codepage %i.\n", cp);
                return false;
            }
            if (!wchar_util::wstr_to_str(name, tmp, CP_UTF8)) {
                printf("Can not convert name to UTF-8.\n");
                return false;
            }
        }
#endif
        std::string filepath = fileop::join(directory, name);
        bool re = false;
        fileop::isdir(filepath, re);
        if (re && (stats.valid & ZIP_STAT_MTIME) && !fileop::set_file_time(filepath, stats.mtime, stats.mtime, stats.mtime)) {
            printf("Can not set modified time to directory \"%s\"\n", filepath.c_str());
        }
    }
    return true;
}
