#include "SkOSFile.h"

#ifdef SK_BUILD_FOR_WIN

static U16* concat_to_16(const char src[], const char suffix[])
{
    size_t  i, len = strlen(src);
    size_t  len2 = 3 + (suffix ? strlen(suffix) : 0);
    U16*    dst = (U16*)sk_malloc_throw((len + len2) * sizeof(U16));

    for (i = 0; i < len; i++)
        dst[i] = src[i];

    if (i > 0 && dst[i-1] != '/')
        dst[i++] = '/';
    dst[i++] = '*';

    if (suffix)
    {
        while (*suffix)
            dst[i++] = *suffix++;
    }
    dst[i] = 0;
    SkASSERT(i + 1 <= len + len2);

    return dst;
}

SkUTF16_Str::SkUTF16_Str(const char src[])
{
    size_t  len = strlen(src);

    fStr = (U16*)sk_malloc_throw((len + 1) * sizeof(U16));
    for (size_t i = 0; i < len; i++)
        fStr[i] = src[i];
    fStr[i] = 0;
}

////////////////////////////////////////////////////////////////////////////

SkOSFile::Iter::Iter() : fHandle(0), fPath16(nil)
{
}

SkOSFile::Iter::Iter(const char path[], const char suffix[]) : fHandle(0), fPath16(nil)
{
    this->reset(path, suffix);
}

SkOSFile::Iter::~Iter()
{
    sk_free(fPath16);
    if (fHandle)
        ::FindClose(fHandle);
}

void SkOSFile::Iter::reset(const char path[], const char suffix[])
{
    if (fHandle)
    {
        ::FindClose(fHandle);
        fHandle = 0;
    }
    if (path == nil)
        path = "";

    sk_free(fPath16);
    fPath16 = concat_to_16(path, suffix);
}

static bool is_magic_dir(const U16 dir[])
{
    // return true for "." and ".."
    return dir[0] == '.' && (dir[1] == 0 || dir[1] == '.' && dir[2] == 0);
}

static bool get_the_file(HANDLE handle, SkString* name, WIN32_FIND_DATAW* dataPtr, bool getDir)
{
    WIN32_FIND_DATAW    data;

    if (dataPtr == nil)
    {
        if (::FindNextFileW(handle, &data))
            dataPtr = &data;
        else
            return false;
    }

    for (;;)
    {
        if (getDir)
        {
            if ((dataPtr->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !is_magic_dir(dataPtr->cFileName))
                break;
        }
        else
        {
            if (!(dataPtr->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                break;
        }
        if (!::FindNextFileW(handle, dataPtr))
            return false;
    }
    // if we get here, we've found a file/dir
    if (name)
        name->setUTF16(dataPtr->cFileName);
    return true;
}

bool SkOSFile::Iter::next(SkString* name, bool getDir)
{
    WIN32_FIND_DATAW    data;
    WIN32_FIND_DATAW*   dataPtr = nil;

    if (fHandle == 0)   // our first time
    {
        if (fPath16 == nil || *fPath16 == 0)    // check for no path
            return false;

        fHandle = ::FindFirstFileW(fPath16, &data);
        if (fHandle != 0 && fHandle != (HANDLE)~0)
            dataPtr = &data;
    }
    return fHandle != (HANDLE)~0 && get_the_file(fHandle, name, dataPtr, getDir);
}

#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)

#if 0
OSStatus FSPathMakeRef (
   const UInt8 * path,
   FSRef * ref,
   Boolean * isDirectory
);
#endif

SkOSFile::Iter::Iter() : fDIR(0)
{
}

SkOSFile::Iter::Iter(const char path[], const char suffix[]) : fDIR(0)
{
    this->reset(path, suffix);
}

SkOSFile::Iter::~Iter()
{
    if (fDIR)
        ::closedir(fDIR);
}

void SkOSFile::Iter::reset(const char path[], const char suffix[])
{
    if (fDIR)
    {
        ::closedir(fDIR);
        fDIR = 0;
    }

    fPath.set(path);
    if (path)
    {
        fDIR = ::opendir(path);
        fSuffix.set(suffix);
    }
    else
        fSuffix.reset();
}

// returns true if suffix is empty, or if str ends with suffix
static bool issuffixfor(const SkString& suffix, const char str[])
{
    size_t  suffixLen = suffix.size();
    size_t  strLen = strlen(str);
    
    return  strLen >= suffixLen &&
            suffixLen == 0 ||
            memcmp(suffix.c_str(), str + strLen - suffixLen, suffixLen) == 0;
}

#include <sys/stat.h>

bool SkOSFile::Iter::next(SkString* name, bool getDir)
{
    if (fDIR)
    {
        dirent* entry;

        while ((entry = ::readdir(fDIR)) != NULL)
        {
            struct stat s;
            SkString    str(fPath);

            if (!str.endsWith("/") && !str.endsWith("\\"))
                str.append("/");
            str.append(entry->d_name);

            if (0 == stat(str.c_str(), &s))
            {
                if (getDir)
                {
                    if (s.st_mode & S_IFDIR)
                        break;
                }
                else
                {
                    if (!(s.st_mode & S_IFDIR) && issuffixfor(fSuffix, entry->d_name))
                        break;
                }
            }
        }
        if (entry)  // we broke out with a file
        {
            if (name)
                name->set(entry->d_name);
            return true;
        }
    }
    return false;
}

#endif

