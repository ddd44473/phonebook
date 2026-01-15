#pragma once

#include <QDialog>

namespace Ui {
class AddEditContactDialog;
}

class AddEditContactDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditContactDialog(QWidget *parent = nullptr);
    ~AddEditContactDialog();

    QString firstName() const;
    QString lastName() const;
    QString middleName() const;
    QString email() const;
    QString phoneWork() const;
    QString phoneHome() const;
    QString phoneService() const;
    QString birthDate() const;
    QString address() const;

private:
    Ui::AddEditContactDialog *ui;
};
