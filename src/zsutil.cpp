/*
 * zsync2
 * ======
 *
 * zsutil.h: Header-only utility library.
 */

// system headers
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>

// library headers
#include <cpr/cpr.h>

// local headers
#include "zsutil.h"

namespace zsync2 {
    bool ltrim(std::string& s, char to_trim) {
        // TODO: find more efficient way to check whether elements have been removed
        size_t initialLength = s.length();
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        return s.length() < initialLength;
    }

    bool rtrim(std::string& s, char to_trim) {
        // TODO: find more efficient way to check whether elements have been removed
        auto initialLength = s.length();
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
        return s.length() < initialLength;
    }

    bool trim(std::string& s, char to_trim) {
        // returns true if either modifies s
        auto ltrim_result = ltrim(s, to_trim);
        return rtrim(s, to_trim) && ltrim_result;
    }

    bool isfile(const std::string& path) {
        std::ifstream ifs(path);
        return (bool) ifs && ifs.good();
    }

    time_t readMtime(const std::string& path) {
        struct stat fstat;

        if (stat(path.c_str(), &fstat) == 0) {
            return fstat.st_mtim.tv_sec;
        }

        return -1;
    }

    bool isUrlAbsolute(const std::string& url) {
        static const char special[] = { ":/?" };

        auto firstSpecial = url.find_first_of(special);

        // if none of those characters has been found, it can't be an absolute URL
        if (firstSpecial == std::string::npos)
            return false;

        // must not be first character
        if (firstSpecial <= 0)
            return false;

        const auto& firstSpecialValue = url.c_str()[firstSpecial];

        if (firstSpecialValue == ':')
            return true;

        return false;
    }

    std::string pathPrefix(std::string path) {
        // remove everything before last /
        auto lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) {
            lastSlash++;
            path = path.substr(lastSlash, path.length());
        }

        // find first non-alphanumeric character
        auto firstNonAlphaNum = std::distance(path.begin(), std::find_if_not(path.begin(), path.end(), isalnum));

        // return alphanumeric part
        return path.substr(0, firstNonAlphaNum);
    }

    std::string toLower(std::string string) {
        std::transform(string.begin(), string.end(), string.begin(), ::tolower);
        return string;
    }

    bool endsWith(const std::string& value, const std::string& ending) {
        if (ending.size() > value.size())
            return false;

        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    bool resolveRedirections(const std::string& absoluteUrl, std::string& redirectedUrl) {
        auto response = cpr::Head(absoluteUrl);

        // check for proper response code
        // 4xx and 5xx responses can be considered okay for a redirection resolver, as they're valid responses
        // however, 3xx responses shouldn't be seen here any more, as CPR should have followed any possible
        // redirection already
        if (response.status_code >= 300 && response.status_code < 400)
            return false;

        redirectedUrl = response.url;
        return true;
    }

    int32_t getPerms(const std::string& path, mode_t& permissions) {
        // check existing permissions
        struct stat appImageStat{};

        if (stat(path.c_str(), &appImageStat) != 0) {
            return errno;
        }

        permissions = appImageStat.st_mode;
        return 0;
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> result;

        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, delim)) {
            result.push_back(item);
        }

        return result;
    }

    std::string base64Decode(const std::string& in) {
        std::string out;

        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

        int val = 0, valb = -8;
        for (const auto& c : in) {
            if (T[c] == -1)
                break;

            val = (val << 6) + T[c];
            valb += 6;

            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }

        return out;
    }

    std::string bytesToHex(unsigned char* data, int len) {
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < len; ++i)
            ss << std::setw(2) << std::setfill('0') << (int)data[i];
        return ss.str();
    }
}
