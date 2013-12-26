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

class validExp
{
public:
    validExp():
        mail(QRegExp("^[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,62}[a-zA-Z\\d])"
             "@[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,180}[a-zA-Z\\d])"
             "\\.[a-zA-Z]{2,6}$")),
        host(QRegExp("^[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,254}[a-zA-Z\\d])"
             "\\.[a-zA-Z]{2,6}$")),
        password(QRegExp("^\\S{1,30}$")),
        name(QRegExp("^\\w{0,15} ?\\w{0,15}$"))
    {
        m=h=p=n=false;
    }

    void check(QLineEdit *line,const QRegExpValidator &valid,bool &B)
    {
        int pos = line->text().length();
        QString text = line->text();
        if(valid.validate(text,pos)==QValidator::Acceptable)
        {
            if(!B)
            {
                B = true;
                line->setStyleSheet("QLineEdit{border-color:white;}");
            }
        }
        else if(B)
        {
            B = false;
            line->setStyleSheet("QLineEdit{border: 2px solid red;"
                                "border-radius: 5px;}");
        }
    }

    const QRegExpValidator mail;
    const QRegExpValidator host;
    const QRegExpValidator password;
    const QRegExpValidator name;


    bool m;
    bool h;
    bool p;
    bool n;
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

    void validation(int index);
private:
    enum LINES{HOST, LOGIN, PASSWORD, NAME};

    QWidget *Parent;

    QFile *config;

    QPushButton *okButton;

    QPushButton *exitButton;

    QLabel *smtpLabel;
    QLineEdit *smtpLine;

    QLabel *portLabel;
    QComboBox *portBox;

    QLabel *userLabel;
    QLineEdit *userLine;

    QLabel *passwdLabel;
    QLineEdit *passwdLine;

    QLabel *nameLabel;
    QLineEdit *nameLine;

    QCheckBox *saveCheck;

    CONFIG *conf;

    QSignalMapper *signalMapper;

    validExp valid;
};

#endif // CONFIGWIDGET_H
