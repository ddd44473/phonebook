#pragma once
#include <string>
#include <vector>
#include <atomic>


enum class PhoneType { Work, Home, Service }; // types of telephones

struct PhoneNumber { // contains phone type + phone number
    PhoneType type; // (work/home/service)
    std::string number;
};

class Contact { 
public:
    //constructors count
    static std::atomic<uint64_t> created_count; //created objects
    static std::atomic<uint64_t> copy_count; //copy constructor
    static std::atomic<uint64_t> move_count; //move constructor

    
    int id = 0; // identifier of the contact(useful for database)

    //fields
    std::string firstname; //required
    std::string lastname; //required
    std::string middlename; //optional
    std::string address; //optional
    std::string birthday; //optional
    std::string email; //required

    std::vector<PhoneNumber> phones; // phonenumber

    Contact(); // default constructor
    ~Contact(); // destructor

    Contact(const Contact& other); //copy constructor
    Contact(Contact&& other) noexcept; //move constructor
    Contact& operator=(const Contact& other); //copy assignment
    Contact& operator=(Contact&& other) noexcept; //move assignment

    
    std::string serialize() const; //convert contact to text (JSON)
    static Contact deserialize(const std::string&); //create contact from text (JSON)
};
