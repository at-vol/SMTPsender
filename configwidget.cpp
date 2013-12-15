#include "configwidget.h"

ConfigWidget::ConfigWidget(CONFIG * c, QWidget *parent)
    : conf(c),
      Parent(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->setWindowTitle(QString::fromUtf8("SMTP configuration"));
    this->setFixedSize(400,250);

    smtpLabel = new QLabel(QString::fromUtf8("SMTP server"),this);
    smtpLine = new QLineEdit(this);
    smtpLabel->setGeometry(50,20,smtpLabel->width(),20);
    smtpLine->setGeometry(50+smtpLabel->width(),20,200,20);

    portLabel = new QLabel(QString::fromUtf8("SMTP port"),this);
    portLine = new QLineEdit(this);
    portLabel->setGeometry(50,50,portLabel->width(),20);
    portLine->setGeometry(50+portLabel->width(),50,200,20);

    userLabel = new QLabel(QString::fromUtf8("Login: "),this);
    userLine  = new QLineEdit(this);
    userLabel->setGeometry(50,80,userLabel->width(),20);
    userLine->setGeometry(50+userLabel->width(),80,200,20);

    passwdLabel = new QLabel(QString::fromUtf8("Password: "),this);
    passwdLine = new QLineEdit(this);
    passwdLabel->setGeometry(50,110,passwdLabel->width(),20);
    passwdLine->setGeometry(50+passwdLabel->width(),110,200,20);

    nameLabel = new QLabel(QString::fromUtf8("Name:"),this);
    nameLine = new QLineEdit(this);
    nameLabel->setGeometry(50,140,nameLabel->width(),20);
    nameLine->setGeometry(50+nameLabel->width(),140,200,20);

    config = new QFile("config");

    if(Parent->isVisible())
    {
        smtpLine->setText(conf->server);
        portLine->setText(QString::number(conf->port));
        userLine->setText(conf->login);
        passwdLine->setText(conf->password);
        nameLine->setText(conf->name);

    }
    else if(config->exists())
    {
        config->open(QIODevice::ReadOnly);
        QByteArray temp;
        temp = config->readLine();
        temp.chop(1);
        smtpLine->setText(QString(temp));
        temp = config->readLine();
        temp.chop(1);
        portLine->setText(QString(temp));
        temp = config->readLine();
        temp.chop(1);
        userLine->setText(QString(temp));
        temp = config->readLine();
        temp.chop(1);
        passwdLine->setText(QString(temp));
        temp = config->readLine();
        temp.chop(1);
        nameLine->setText(QString(temp));
        config->close();
    }

    saveCheck = new QCheckBox(QString::fromUtf8("save as default"),this);
    saveCheck->move(this->width()/2 - saveCheck->width()/2,170);
    saveCheck->setChecked(true);

    okButton = new QPushButton(QString::fromUtf8("OK"),this);
    okButton->move(this->width()/2 - okButton->width() - 10,this->height()-40);
    connect(okButton,SIGNAL(clicked()),this,SLOT(clicked_on_okButton()));

    exitButton = new QPushButton(QString::fromUtf8((Parent->isVisible())?"Cancel":"Exit"),this);
    exitButton->move(this->width()/2 + 10,okButton->pos().y());
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));
}

ConfigWidget::~ConfigWidget()
{
    delete config;
}

void ConfigWidget::clicked_on_exitButton()
{
    if(!Parent->isVisible())
        Parent->close();
    this->close();
}

void ConfigWidget::clicked_on_okButton()
{
    conf->server = smtpLine->text();
    conf->port = portLine->text().toInt();
    conf->login = userLine->text();
    conf->password = passwdLine->text();
    conf->name = nameLine->text();

    if(saveCheck->isChecked())
    {
        config->open(QIODevice::WriteOnly);
        config->write(smtpLine->text().toUtf8()+"\n");
        config->write(portLine->text().toUtf8()+"\n");
        config->write(userLine->text().toUtf8()+"\n");
        config->write(passwdLine->text().toUtf8()+"\n");
        config->write(nameLine->text().toUtf8()+"\n");
        config->close();
    }

    if(!Parent->isVisible())
        Parent->show();
    this->close();
}

