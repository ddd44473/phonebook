#include "../hpps/postgres_repository.h"

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDate>

PostgresRepository::PostgresRepository(
    const std::string& host,
    int port,
    const std::string& dbName,
    const std::string& user,
    const std::string& password
)
    : host_(host)
    , port_(port)
    , dbName_(dbName)
    , user_(user)
    , password_(password)
{
    
}

static QSqlDatabase makeDb(const std::string& host, int port,
                          const std::string& dbName,
                          const std::string& user,
                          const std::string& password)
{
    // unique name 
    const QString connName =
        QString("pg_%1_%2_%3")
            .arg(QString::fromStdString(user))
            .arg(QString::fromStdString(dbName))
            .arg(port);

    QSqlDatabase db;
    if (QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::database(connName);
    } else {
        db = QSqlDatabase::addDatabase("QPSQL", connName);
        db.setHostName(QString::fromStdString(host));
        db.setPort(port);
        db.setDatabaseName(QString::fromStdString(dbName));
        db.setUserName(QString::fromStdString(user));
        db.setPassword(QString::fromStdString(password));
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            qWarning() << "PostgreSQL open error:" << db.lastError().text();
        }
    }
    return db;
}

void PostgresRepository::ensureSchema() const
{
    auto db = makeDb(host_, port_, dbName_, user_, password_);
    if (!db.isOpen()) return;

    QSqlQuery q(db);

    // contacts
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS contacts ("
        "  id INTEGER PRIMARY KEY,"
        "  firstname  TEXT NOT NULL,"
        "  lastname   TEXT NOT NULL,"
        "  middlename TEXT,"
        "  address    TEXT,"
        "  birthday   DATE,"
        "  email      TEXT NOT NULL"
        ");"
    )) {
        qWarning() << "ensureSchema contacts error:" << q.lastError().text();
        return;
    }

    // phones
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS phones ("
        "  id SERIAL PRIMARY KEY,"
        "  contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,"
        "  type SMALLINT NOT NULL,"
        "  number TEXT NOT NULL"
        ");"
    )) {
        qWarning() << "ensureSchema phones error:" << q.lastError().text();
        return;
    }

    // index
    q.exec("CREATE INDEX IF NOT EXISTS idx_phones_contact_id ON phones(contact_id);");
}

std::vector<Contact> PostgresRepository::loadAll() const
{
    ensureSchema();

    std::vector<Contact> result;

    auto db = makeDb(host_, port_, dbName_, user_, password_);
    if (!db.isOpen()) return result;

    QSqlQuery qc(db);
    if (!qc.exec("SELECT id, firstname, lastname, middlename, address, birthday, email FROM contacts ORDER BY id;")) {
        qWarning() << "loadAll contacts error:" << qc.lastError().text();
        return result;
    }

    while (qc.next()) {
        Contact c;
        c.id = qc.value(0).toInt();
        c.firstname  = qc.value(1).toString().toStdString();
        c.lastname   = qc.value(2).toString().toStdString();
        c.middlename = qc.value(3).toString().toStdString();
        c.address    = qc.value(4).toString().toStdString();

        // birthday can be null
        const QVariant b = qc.value(5);
        c.birthday = b.isNull() ? "" : b.toDate().toString("yyyy-MM-dd").toStdString();

        c.email = qc.value(6).toString().toStdString();

        // phones
        QSqlQuery qp(db);
        qp.prepare("SELECT type, number FROM phones WHERE contact_id = :cid ORDER BY id;");
        qp.bindValue(":cid", c.id);

        if (!qp.exec()) {
            qWarning() << "loadAll phones error:" << qp.lastError().text();
            return result;
        }

        while (qp.next()) {
            PhoneNumber p;
            p.type = static_cast<PhoneType>(qp.value(0).toInt());
            p.number = qp.value(1).toString().toStdString();
            c.phones.push_back(p);
        }

        result.push_back(std::move(c));
    }

    return result;
}

void PostgresRepository::saveAll(const std::vector<Contact>& contacts) const
{
    ensureSchema();

    auto db = makeDb(host_, port_, dbName_, user_, password_);
    if (!db.isOpen()) return;

    if (!db.transaction()) {
        qWarning() << "saveAll: cannot start transaction:" << db.lastError().text();
        return;
    }

    QSqlQuery q(db);

    // clear tables
    if (!q.exec("DELETE FROM contacts;")) {
        qWarning() << "saveAll delete contacts error:" << q.lastError().text();
        db.rollback();
        return;
    }

    // write contacts
    QSqlQuery ic(db);
    ic.prepare(
        "INSERT INTO contacts(id, firstname, lastname, middlename, address, birthday, email) "
        "VALUES(:id, :fn, :ln, :mn, :addr, :bd, :em);"
    );

    QSqlQuery ip(db);
    ip.prepare(
        "INSERT INTO phones(contact_id, type, number) "
        "VALUES(:cid, :type, :num);"
    );

    for (const auto& c : contacts) {
        ic.bindValue(":id", c.id);
        ic.bindValue(":fn", QString::fromStdString(c.firstname));
        ic.bindValue(":ln", QString::fromStdString(c.lastname));
        ic.bindValue(":mn", QString::fromStdString(c.middlename));
        ic.bindValue(":addr", QString::fromStdString(c.address));

        // birthday can be null
        if (c.birthday.empty()) {
            ic.bindValue(":bd", QVariant(QVariant::Date));
        } else {
            ic.bindValue(":bd", QDate::fromString(QString::fromStdString(c.birthday), "yyyy-MM-dd"));
        }

        ic.bindValue(":em", QString::fromStdString(c.email));

        if (!ic.exec()) {
            qWarning() << "saveAll insert contact error:" << ic.lastError().text();
            db.rollback();
            return;
        }

        for (const auto& p : c.phones) {
            ip.bindValue(":cid", c.id);
            ip.bindValue(":type", static_cast<int>(p.type));
            ip.bindValue(":num", QString::fromStdString(p.number));

            if (!ip.exec()) {
                qWarning() << "saveAll insert phone error:" << ip.lastError().text();
                db.rollback();
                return;
            }
        }
    }

    if (!db.commit()) {
        qWarning() << "saveAll commit error:" << db.lastError().text();
        db.rollback();
    }
}
