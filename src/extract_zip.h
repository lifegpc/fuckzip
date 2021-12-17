#ifndef _FUCKZIP_FILEOP_H
#define _FUCKZIP_FILEOP_H
#include "zip.h"
#include <string>

bool extract_zip(zip_t* zip, std::string directory, int cp, std::string enc);
#endif
