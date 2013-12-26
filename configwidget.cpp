#include "configwidget.h"

#define SMTP_MTA "25"  //mail transfer agent
#define SMTP_SSL "465"   //SMTP with SSL encryption
#define SMTP_MSA "587"   //mail submission agent
#define SMTP_ALT "2525"   //alternative port

ConfigWidget::ConfigWidget(CONFIG * c, QWidget *parent)
    : Parent(parent),
      conf(c)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->setWindowTitle(QString::fromUtf8("SMTP configuration"));
    this->setFixedSize(400,250);

    signalMapper = new QSignalMapper(this);

    smtpLabel = new QLabel(QString::fromUtf8("SMTP server"),this);
    smtpLine = new QLineEdit(this);
    smtpLabel->setGeometry(50,20,smtpLabel->width(),20);
    smtpLine->setGeometry(50+smtpLabel->width(),20,200,20);
    smtpLine->setValidator(&valid.host);
    connect(smtpLine,SIGNAL(textChanged(QString)),signalMapper,SLOT(map()));
    signalMapper->setMapping(smtpLine,HOST);

    portLabel = new QLabel(QString::fromUtf8("SMTP port"),this);
    portBox = new QComboBox(this);
    QStringList ports;
    ports << SMTP_MTA << SMTP_SSL << SMTP_MSA << SMTP_ALT;
    portBox->addItems(ports);
    portLabel->setGeometry(50,50,portLabel->width(),20);
    portBox->setGeometry(50+portLabel->width(),50,200,20);

    userLabel = new QLabel(QString::fromUtf8("Login: "),this);
    userLine  = new QLineEdit(this);
    userLabel->setGeometry(50,80,userLabel->width(),20);
    userLine->setGeometry(50+userLabel->width(),80,200,20);
    userLine->setValidator(&valid.mail);
    connect(userLine,SIGNAL(textChanged(QString)),signalMapper,SLOT(map()));
    signalMapper->setMapping(userLine,LOGIN);

    passwdLabel = new QLabel(QString::fromUtf8("Password: "),this);
    passwdLine = new QLineEdit(this);
    passwdLabel->setGeometry(50,110,passwdLabel->width(),20);
    passwdLine->setGeometry(50+passwdLabel->width(),110,200,20);
    passwdLine->setValidator(&valid.password);
    passwdLine->setEchoMode(QLineEdit::Password);
    connect(passwdLine,SIGNAL(textChanged(QString)),signalMapper,SLOT(map()));
    signalMapper->setMapping(passwdLine,PASSWORD);

    nameLabel = new QLabel(QString::fromUtf8("Name:"),this);
    nameLine = new QLineEdit(this);
    nameLabel->setGeometry(50,140,nameLabel->width(),20);
    nameLine->setGeometry(50+nameLabel->width(),140,200,20);
    nameLine->setValidator(&valid.name);
    connect(nameLine,SIGNAL(textChanged(QString)),signalMapper,SLOT(map()));
    signalMapper->setMapping(nameLine,NAME);

    connect(signalMapper,SIGNAL(mapped(int)),this,SLOT(validation(int)));

    saveCheck = new QCheckBox(QString::fromUtf8("save as default"),this);
    saveCheck->move(this->width()/2 - saveCheck->width()/2,170);

    okButton = new QPushButton(QString::fromUtf8("OK"),this);
    okButton->move(this->width()/2 - okButton->width() - 10,this->height()-40);
    okButton->setAutoDefault(true);
    connect(okButton,SIGNAL(clicked()),this,SLOT(clicked_on_okButton()));

    exitButton = new QPushButton(QString::fromUtf8((Parent->isVisible())?"Cancel":"Exit"),this);
    exitButton->move(this->width()/2 + 10,okButton->pos().y());
    exitButton->setAutoDefault(true);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));

    config = new QFile("config");

    if(Parent->isVisible())
    {
        smtpLine->setText(conf->server);
        for(int i=0;i<5;i++)
            if(ports.at(i).toInt()==conf->port)
            {
                portBox->setCurrentIndex(i);
                break;
            }
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
        portBox->setCurrentIndex(QString(temp).toInt());
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

    okButton->setFocus();
}

ConfigWidget::~ConfigWidget()
{
    delete config;
}

void ConfigWidget::validation(int index)
{
    switch(index)
    {
    case HOST:
        valid.check(smtpLine,valid.host,valid.h);
        break;
    case LOGIN:
        valid.check(userLine,valid.mail,valid.m);
        break;
    case PASSWORD:
        valid.check(passwdLine,valid.password,valid.p);
        break;
    case NAME:
        valid.check(nameLine,valid.name,valid.n);
    }
    if(valid.h && valid.m && valid.p && valid.p)
    {
        if(!okButton->isEnabled())
            okButton->setEnabled(true);
    }
    else if(okButton->isEnabled())
            okButton->setEnabled(false);
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
    conf->port = portBox->currentText().toInt();
    conf->login = userLine->text();
    conf->password = passwdLine->text();
    conf->name = nameLine->text();

    if(saveCheck->isChecked())
    {
        config->open(QIODevice::WriteOnly);
        config->write(smtpLine->text().toUtf8()+"\n");
        config->write(QString().number(portBox->currentIndex()).toUtf8()+"\n");
        config->write(userLine->text().toUtf8()+"\n");
        config->write(passwdLine->text().toUtf8()+"\n");
        config->write(nameLine->text().toUtf8()+"\n");
        config->close();
    }

    if(!Parent->isVisible())
        Parent->show();
    this->close();
}

