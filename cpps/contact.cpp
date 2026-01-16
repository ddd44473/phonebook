#include "../hpps/contact.h"
#include <utility> 
#include <cstdint>   
#include <sstream>   

//static counters
std::atomic<uint64_t> Contact::created_count{0}; // created contacts count
std::atomic<uint64_t> Contact::copy_count{0}; // copy contacts count
std::atomic<uint64_t> Contact::move_count{0}; // move contacts count


Contact::Contact() {
    ++created_count;
}

Contact::~Contact() = default; // default destructor

Contact::Contact(const Contact& other) // copy constructor
    : id(other.id),
      firstname(other.firstname),
      lastname(other.lastname),
      middlename(other.middlename),
      address(other.address),
      birthday(other.birthday),
      email(other.email),
      phones(other.phones)
{
    ++copy_count;
}

Contact::Contact(Contact&& other) noexcept // move constructor
    : id(other.id),
      firstname(std::move(other.firstname)),
      lastname(std::move(other.lastname)),
      middlename(std::move(other.middlename)),
      address(std::move(other.address)),
      birthday(std::move(other.birthday)),
      email(std::move(other.email)),
      phones(std::move(other.phones))
{
    ++move_count;
    other.id = 0;
}

Contact& Contact::operator=(const Contact& other) { // copy assignment
    if (this != &other) {
        ++copy_count;
        id         = other.id;
        firstname  = other.firstname;
        lastname   = other.lastname;
        middlename = other.middlename;
        address    = other.address;
        birthday   = other.birthday;
        email      = other.email;
        phones     = other.phones;
    }
    return *this;
}

Contact& Contact::operator=(Contact&& other) noexcept { // move assignment
    if (this != &other) {
        ++move_count;
        id         = other.id;
        firstname  = std::move(other.firstname);
        lastname   = std::move(other.lastname);
        middlename = std::move(other.middlename);
        address    = std::move(other.address);
        birthday   = std::move(other.birthday);
        email      = std::move(other.email);
        phones     = std::move(other.phones);
        other.id = 0;
    }
    return *this;
}


std::string Contact::serialize() const
{
    std::ostringstream ss;

    ss << "id:"     << id        << '\n';
    ss << "first:"  << firstname << '\n';
    ss << "last:"   << lastname  << '\n';
    ss << "middle:" << middlename << '\n';
    ss << "addr:"   << address   << '\n';
    ss << "birth:"  << birthday  << '\n';
    ss << "email:"  << email     << '\n';

    // phones
    for (const auto& p : phones) {
        ss << "phone:";
        switch (p.type) {
            case PhoneType::Work:    ss << "Work";    break;
            case PhoneType::Home:    ss << "Home";    break;
            case PhoneType::Service: ss << "Service"; break;
        }
        ss << '|' << p.number << '\n';
    }

    ss << "----"; // конец блока
    return ss.str();
}

// string into PhoneType
static PhoneType phoneTypeFromString(const std::string& s)
{
    if (s == "Work")    return PhoneType::Work;
    if (s == "Home")    return PhoneType::Home;
    return PhoneType::Service; // по умолчанию
}


Contact Contact::deserialize(const std::string& block)
{
    Contact c;

    std::istringstream in(block);
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line == "----") break; // конец контакта

        auto pos = line.find(':');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "id")        c.id        = std::stoi(val);
        else if (key == "first")  c.firstname  = val;
        else if (key == "last")   c.lastname   = val;
        else if (key == "middle") c.middlename = val;
        else if (key == "addr")   c.address    = val;
        else if (key == "birth")  c.birthday   = val;
        else if (key == "email")  c.email      = val;
        else if (key == "phone") {
            // формат: Type|Number
            auto sep = val.find('|');
            PhoneNumber p;
            if (sep != std::string::npos) {
                std::string typeStr = val.substr(0, sep);
                std::string numStr  = val.substr(sep + 1);
                p.type   = phoneTypeFromString(typeStr);
                p.number = numStr;
            } else {
                p.type   = PhoneType::Work;  // default
                p.number = val;
            }
            c.phones.push_back(std::move(p));
        }
    }

    return c;
}
