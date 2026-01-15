#pragma once
#include <vector>
#include "../hpps/contact.h"

class IRepository
{
public:
    virtual ~IRepository() = default;

    virtual std::vector<Contact> loadAll() const = 0;
    virtual void saveAll(const std::vector<Contact>& contacts) const = 0;
};
