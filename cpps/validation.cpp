#include "../hpps/validation.h"

#include <cctype>
#include <ctime>
#include <regex>
#include <string>

// ---------- helpers ----------
static std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;

    return s.substr(start, end - start);
}

static std::string removeSpaces(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        if (!std::isspace(ch)) out.push_back(static_cast<char>(ch));
    }
    return out;
}

// yyyy-MM-dd -> y,m,d ; return false if format/values invalid
static bool parseIsoDate(const std::string& s, int& y, int& m, int& d)
{
    if (s.size() != 10) return false;
    if (s[4] != '-' || s[7] != '-') return false;

    auto isDig = [](char c){ return c >= '0' && c <= '9'; };
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!isDig(s[i])) return false;
    }

    y = std::stoi(s.substr(0, 4));
    m = std::stoi(s.substr(5, 2));
    d = std::stoi(s.substr(8, 2));

    if (m < 1 || m > 12) return false;

    auto isLeap = [&](int year){
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    };

    int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (m == 2 && isLeap(y)) mdays[1] = 29;

    if (d < 1 || d > mdays[m - 1]) return false;
    return true;
}

// compare (y,m,d) lexicographically
static bool isBefore(int y1,int m1,int d1, int y2,int m2,int d2)
{
    if (y1 != y2) return y1 < y2;
    if (m1 != m2) return m1 < m2;
    return d1 < d2;
}

static void todayYMD(int& y, int& m, int& d)
{
    std::time_t t = std::time(nullptr);
    std::tm lt{};
#if defined(_WIN32)
    localtime_s(&lt, &t);
#else
    lt = *std::localtime(&t);
#endif
    y = lt.tm_year + 1900;
    m = lt.tm_mon + 1;
    d = lt.tm_mday;
}

// ---------- validation ----------
bool validateName(const std::string& s)
{
    const std::string t = trim(s);
    if (t.empty()) return false;


    static const std::regex re(R"(^[A-Za-z][A-Za-z0-9]*(?:[ -][A-Za-z0-9]+)*$)");
    return std::regex_match(t, re);
}

bool validateEmail(const std::string& s)
{
    const std::string t = normalizeEmail(s);

    // 1) ровно один '@'
    const auto at = t.find('@');
    if (at == std::string::npos) return false;
    if (t.find('@', at + 1) != std::string::npos) return false;

    const std::string user = t.substr(0, at);
    const std::string dom  = t.substr(at + 1);

    if (user.empty() || dom.empty()) return false;

    auto isLatLetter = [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    };
    auto isLatAlnum = [&](unsigned char c) {
        return isLatLetter(c) || (c >= '0' && c <= '9');
    };
    auto isUserChar = [&](unsigned char c) {
        return isLatAlnum(c) || c == '.' || c == '_' || c == '-';
    };

    // 2) username: только латиница/цифры и . _ - ,
    //    не начинать/заканчивать на . _ -
    if (user.front() == '.' || user.front() == '_' || user.front() == '-') return false;
    if (user.back()  == '.' || user.back()  == '_' || user.back()  == '-') return false;

    bool hasLetter = false;
    for (unsigned char c : user) {
        if (!isUserChar(c)) return false;
        if (isLatLetter(c)) hasLetter = true;
    }
    if (!hasLetter) return false; // ✅ запрещаем 123@...

    // 3) домен должен быть вида label.label (с точкой), где label = [A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?
    //    и tld >= 2 букв
    static const std::regex domRe(
        R"(^[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?)+$)"
    );
    if (!std::regex_match(dom, domRe)) return false;

    // tld: минимум 2 буквы
    const auto lastDot = dom.rfind('.');
    if (lastDot == std::string::npos) return false;
    const std::string tld = dom.substr(lastDot + 1);
    if (tld.size() < 2) return false;
    for (unsigned char c : tld) {
        if (!isLatLetter(c)) return false;
    }

    return true;
}

std::string normalizeEmail(const std::string& s)
{
    std::string t;
    t.reserve(s.size());
    for (unsigned char ch : s) {
        if (!std::isspace(ch)) t.push_back(static_cast<char>(ch));
    }
    return t;
}

bool validatePhone(const std::string& s)
{
    const std::string t = trim(s);

    // как было: ^(\+7|8)(812|\(812\))(\d{7}|\d{3}-\d{2}-\d{2})$
    static const std::regex re(R"(^(?:\+7|8)(?:812|\(812\))(?:\d{7}|\d{3}-\d{2}-\d{2})$)");
    return std::regex_match(t, re);
}

bool validateBirthDate(const std::string& s)
{
    const std::string t = trim(s);
    if (t.empty()) return false;

    int y, m, d;
    if (!parseIsoDate(t, y, m, d)) return false;

    int ty, tm, td;
    todayYMD(ty, tm, td);

    // строго меньше сегодняшней
    return isBefore(y, m, d, ty, tm, td);
}
