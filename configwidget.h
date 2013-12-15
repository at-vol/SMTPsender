#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QCheckBox>
#include <QFile>

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
    QWidget *Parent;

    QFile *config;

    QPushButton *okButton;

    QPushButton *exitButton;

    QLabel *smtpLabel;
    QLineEdit *smtpLine;

    QLabel *portLabel;
    QLineEdit *portLine;

    QLabel *userLabel;
    QLineEdit *userLine;

    QLabel *passwdLabel;
    QLineEdit *passwdLine;

    QLabel *nameLabel;
    QLineEdit *nameLine;

    QCheckBox *saveCheck;

    CONFIG *conf;
};


#endif // CONFIGWIDGET_H
