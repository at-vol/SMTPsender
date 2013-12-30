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
#include <QSignalMapper>

struct CONFIG
{
    QString server;
    quint16 port;
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
    enum layoutRows{SERVER, PORT, LOGIN, PASSWORD, NAME, SAVE, WARNING, BUTTONS};

    bool checkValidation(const QLineEdit *line);

    QWidget *Parent;

    QFile *config;

    QPushButton *okButton;

    QPushButton *exitButton;

    QLabel *serverLabel;
    QLineEdit *serverLine;

    QLabel *portLabel;
    QComboBox *portBox;

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
