#pragma once

#include <string>
#include <vector>

#include "../hpps/contact.h"
#include "../hpps/repository.h"

class FileRepository : public IRepository
{
public:
    explicit FileRepository(const std::string& filePath);

    std::vector<Contact> loadAll() const override;
    void saveAll(const std::vector<Contact>& contacts) const override;

private:
    std::string path;
};
