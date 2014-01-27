#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QCheckBox>
#include <QFile>
#include <QRegExpValidator>

struct CONFIG
{
    QString smtpServer;
    quint16 smtpPort;
    quint8 encryption;
    QString login;
    QString password;
    QString name;
};

class ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    ConfigWidget(CONFIG *c = 0, QWidget *parent = 0);
    ~ConfigWidget();
protected slots:
    void clicked_on_exitButton();

    void clicked_on_okButton();
private:
    enum layoutRows{SMTPSERVER, SMTPPORT, ENCRYPTION, LOGIN, PASSWORD, NAME, SAVE, WARNING, BUTTONS};

    bool checkValidation(const QLineEdit *line) const;

    QWidget *Parent;

    QPushButton *okButton;

    QPushButton *exitButton;

    QLabel *smtpServerLabel;
    QLineEdit *smtpServerLine;

    QLabel *smtpPortLabel;
    QComboBox *smtpPortBox;

    QLabel *encryptionLabel;
    QComboBox *encryptionBox;

    QLabel *userLabel;
    QLineEdit *userLine;

    QLabel *passwdLabel;
    QLineEdit *passwdLine;

    QLabel *nameLabel;
    QLineEdit *nameLine;

    QCheckBox *saveCheck;

    QLabel *warningLabel;

    CONFIG *conf;
};

#endif // CONFIGWIDGET_H
