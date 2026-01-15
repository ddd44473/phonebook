#include "../hpps/file_repository.h"

#include <QFile>
#include <QTextStream>
#include <QString>

// filePath приходит строкой (мы передаём туда путь из AppData)
FileRepository::FileRepository(const std::string& filePath)
    : path(filePath)
{}

// Чтение всех контактов из файла
std::vector<Contact> FileRepository::loadAll() const
{
    std::vector<Contact> result;

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QString line;
    QString block;

    auto flushBlock = [&]() {
        if (!block.isEmpty()) {
            result.push_back(Contact::deserialize(block.toStdString()));
            block.clear();
        }
    };

    while (!in.atEnd()) {
        line = in.readLine();

        if (line == "----") {
            flushBlock();
        } else {
            if (!block.isEmpty()) block += "\n";
            block += line;
        }
    }

    flushBlock();
    return result;
}

// Полная перезапись файла списком контактов
void FileRepository::saveAll(const std::vector<Contact>& contacts) const
{
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    for (const auto& c : contacts) {
        // serialize() уже заканчивает блок "----"
        out << QString::fromStdString(c.serialize()) << "\n";
    }
}
