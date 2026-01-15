#pragma once
#include <string>

//std::string trim(const std::string& s);
//std::string removeSpaces(const std::string& s);

bool validateName(const std::string& s);
bool validateEmail(const std::string& s);
bool validatePhone(const std::string& s);
bool validateBirthDate(const std::string& s);
std::string normalizeEmail(const std::string& s);
