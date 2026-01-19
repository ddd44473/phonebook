#include "addeditcontactdialog.h"
#include "ui_AddEditContactDialog.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegularExpression>

#include "../hpps/validation.h" // validateEmail

AddEditContactDialog::AddEditContactDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddEditContactDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // email fix
        QString e = ui->leEmail->text();
        e = e.trimmed();
        e.remove(QRegularExpression("\\s+"));

        // current email
        ui->leEmail->setText(e);

        // current email fix
        if (!validateEmail(e.toStdString())) {
            QMessageBox::warning(
                this,
                "Ошибка",
                "Некорректный email по ТЗ.\n"
                "Допустимы только латинские буквы/цифры и один символ '@'.\n"
                "Пробелы автоматически удаляются."
            );
            return; 
        }

        // email fix
        QString first = ui->leFirstName->text().trimmed().toLower();
        if (!first.isEmpty()) {
            const QString userPart = e.section('@', 0, 0).toLower(); // часть до '@'
            if (!userPart.contains(first)) {
                QMessageBox::warning(
                    this,
                    "Ошибка",
                    "Email должен содержать имя пользователя (часть до '@' должна включать имя).\n"
                    "Пример: emet@gmailcom или emetspb@gmailcom."
                );
                return; // НЕ закрываем диалог
            }
        }

        accept(); 
    });

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AddEditContactDialog::~AddEditContactDialog()
{
    delete ui;
}

QString AddEditContactDialog::firstName()  const { return ui->leFirstName->text(); }
QString AddEditContactDialog::lastName()   const { return ui->leLastName->text(); }
QString AddEditContactDialog::middleName() const { return ui->leOtch->text(); }
QString AddEditContactDialog::email()      const { return ui->leEmail->text(); }
QString AddEditContactDialog::phoneWork()  const { return ui->lePhoneWork->text(); }
QString AddEditContactDialog::phoneHome()  const { return ui->lePhoneHome->text(); }
QString AddEditContactDialog::phoneService() const { return ui->lePhoneService->text(); }
QString AddEditContactDialog::birthDate()  const { return ui->leBirthDate->text(); }
QString AddEditContactDialog::address()    const { return ui->leAddress->text(); }
