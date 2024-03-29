/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkTDArray.h"

static bool string_is_in(const char* target, const char* set[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (0 == strcmp(target, set[i])) {
            return true;
        }
    }
    return false;
}

/**
 *  Check to see whether string represents a boolean value.
 *  @param string C style string to parse.
 *  @param result Pointer to a boolean which will be set to the value in the string, if the
 *      string represents a boolean.
 *  @param boolean True if the string represents a boolean, false otherwise.
 */
static bool parse_bool_arg(const char* string, bool* result) {
    static const char* trueValues[] = { "1", "TRUE", "true" };
    if (string_is_in(string, trueValues, SK_ARRAY_COUNT(trueValues))) {
        *result = true;
        return true;
    }
    static const char* falseValues[] = { "0", "FALSE", "false" };
    if (string_is_in(string, falseValues, SK_ARRAY_COUNT(falseValues))) {
        *result = false;
        return true;
    }
    SkDebugf("Parameter \"%s\" not supported.\n", string);
    return false;
}

bool SkFlagInfo::match(const char* string) {
    if (SkStrStartsWith(string, '-') && strlen(string) > 1) {
        string++;
        const SkString* compareName;
        if (SkStrStartsWith(string, '-') && strlen(string) > 1) {
            string++;
            // There were two dashes. Compare against full name.
            compareName = &fName;
        } else {
            // One dash. Compare against the short name.
            compareName = &fShortName;
        }
        if (kBool_FlagType == fFlagType) {
            // In this case, go ahead and set the value.
            if (compareName->equals(string)) {
                *fBoolValue = true;
                return true;
            }
            if (SkStrStartsWith(string, "no") && strlen(string) > 2) {
                string += 2;
                // Only allow "no" to be prepended to the full name.
                if (fName.equals(string)) {
                    *fBoolValue = false;
                    return true;
                }
                return false;
            }
            int equalIndex = SkStrFind(string, "=");
            if (equalIndex > 0) {
                // The string has an equal sign. Check to see if the string matches.
                SkString flag(string, equalIndex);
                if (flag.equals(*compareName)) {
                    // Check to see if the remainder beyond the equal sign is true or false:
                    string += equalIndex + 1;
                    parse_bool_arg(string, fBoolValue);
                    return true;
                } else {
                    return false;
                }
            }
        }
        return compareName->equals(string);
    } else {
        // Has no dash
        return false;
    }
    return false;
}

SkFlagInfo* SkCommandLineFlags::gHead;
SkString SkCommandLineFlags::gUsage;

void SkCommandLineFlags::SetUsage(const char* usage) {
    gUsage.set(usage);
}

// Maximum line length for the help message.
#define LINE_LENGTH 80

static void print_help_for_flag(const SkFlagInfo* flag) {
    SkDebugf("\t--%s", flag->name().c_str());
    const SkString& shortName = flag->shortName();
    if (shortName.size() > 0) {
        SkDebugf(" or -%s", shortName.c_str());
    }
    SkDebugf(":\ttype: %s", flag->typeAsString().c_str());
    if (flag->defaultValue().size() > 0) {
        SkDebugf("\tdefault: %s", flag->defaultValue().c_str());
    }
    SkDebugf("\n");
    const SkString& help = flag->help();
    size_t length = help.size();
    const char* currLine = help.c_str();
    const char* stop = currLine + length;
    while (currLine < stop) {
        if (strlen(currLine) < LINE_LENGTH) {
            // Only one line length's worth of text left.
            SkDebugf("\t\t%s\n", currLine);
            break;
        }
        int lineBreak = SkStrFind(currLine, "\n");
        if (lineBreak < 0 || lineBreak > LINE_LENGTH) {
            // No line break within line length. Will need to insert one.
            // Find a space before the line break.
            int spaceIndex = LINE_LENGTH - 1;
            while (spaceIndex > 0 && currLine[spaceIndex] != ' ') {
                spaceIndex--;
            }
            int gap;
            if (0 == spaceIndex) {
                // No spaces on the entire line. Go ahead and break mid word.
                spaceIndex = LINE_LENGTH;
                gap = 0;
            } else {
                // Skip the space on the next line
                gap = 1;
            }
            SkDebugf("\t\t%.*s\n", spaceIndex, currLine);
            currLine += spaceIndex + gap;
        } else {
            // the line break is within the limit. Break there.
            lineBreak++;
            SkDebugf("\t\t%.*s", lineBreak, currLine);
            currLine += lineBreak;
        }
    }
    SkDebugf("\n");
}

void SkCommandLineFlags::Parse(int argc, char** argv) {
    // Only allow calling this function once.
    static bool gOnce;
    if (gOnce) {
        SkDebugf("Parse should only be called once at the beginning of main!\n");
        SkASSERT(false);
        return;
    }
    gOnce = true;

    bool helpPrinted = false;
    // Loop over argv, starting with 1, since the first is just the name of the program.
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            // Print help message.
            SkTDArray<const char*> helpFlags;
            for (int j = i + 1; j < argc; j++) {
                if (SkStrStartsWith(argv[j], '-')) {
                    break;
                }
                helpFlags.append(1, &argv[j]);
            }
            if (0 == helpFlags.count()) {
                // Only print general help message if help for specific flags is not requested.
                SkDebugf("%s\n%s\n", argv[0], gUsage.c_str());
            }
            SkDebugf("Flags:\n");
            SkFlagInfo* flag = SkCommandLineFlags::gHead;
            while (flag != NULL) {
                // If no flags followed --help, print them all
                bool printFlag = 0 == helpFlags.count();
                if (!printFlag) {
                    for (int k = 0; k < helpFlags.count(); k++) {
                        if (flag->name().equals(helpFlags[k]) ||
                            flag->shortName().equals(helpFlags[k])) {
                            printFlag = true;
                            helpFlags.remove(k);
                            break;
                        }
                    }
                }
                if (printFlag) {
                    print_help_for_flag(flag);
                }
                flag = flag->next();
            }
            if (helpFlags.count() > 0) {
                SkDebugf("Requested help for unrecognized flags:\n");
                for (int k = 0; k < helpFlags.count(); k++) {
                    SkDebugf("\t--%s\n", helpFlags[k]);
                }
            }
            helpPrinted = true;
        }
        if (!helpPrinted) {
            bool flagMatched = false;
            SkFlagInfo* flag = gHead;
            while (flag != NULL) {
                if (flag->match(argv[i])) {
                    flagMatched = true;
                    switch (flag->getFlagType()) {
                        case SkFlagInfo::kBool_FlagType:
                            // Can be handled by match, above, but can also be set by the next
                            // string.
                            if (i+1 < argc && !SkStrStartsWith(argv[i+1], '-')) {
                                i++;
                                bool value;
                                if (parse_bool_arg(argv[i], &value)) {
                                    flag->setBool(value);
                                }
                            }
                            break;
                        case SkFlagInfo::kString_FlagType:
                            flag->resetStrings();
                            // Add all arguments until another flag is reached.
                            while (i+1 < argc && !SkStrStartsWith(argv[i+1], '-')) {
                                i++;
                                flag->append(argv[i]);
                            }
                            break;
                        case SkFlagInfo::kInt_FlagType:
                            i++;
                            flag->setInt(atoi(argv[i]));
                            break;
                        case SkFlagInfo::kDouble_FlagType:
                            i++;
                            flag->setDouble(atof(argv[i]));
                            break;
                        default:
                            SkASSERT(!"Invalid flag type");
                    }
                    break;
                }
                flag = flag->next();
            }
            if (!flagMatched) {
                SkDebugf("Got unknown flag \"%s\". Exiting.\n", argv[i]);
                exit(-1);
            }
        }
    }
    // Since all of the flags have been set, release the memory used by each
    // flag. FLAGS_x can still be used after this.
    SkFlagInfo* flag = gHead;
    gHead = NULL;
    while (flag != NULL) {
        SkFlagInfo* next = flag->next();
        SkDELETE(flag);
        flag = next;
    }
    if (helpPrinted) {
        exit(0);
    }
}
