#include "mailwidget.h"

MailWidget::MailWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle(QString::fromUtf8("SMTP sender"));
    this->setFixedSize(600,500);

    toLabel = new QLabel(QString::fromUtf8("To:"),this);
    toLine = new QLineEdit(this);
    toLabel->setGeometry(50,50,toLabel->width(),20);
    toLine->setGeometry(50+toLabel->width(),50,this->width()-100-toLabel->width(),20);

    subjectLabel = new QLabel(QString::fromUtf8("Subject:"),this);
    subjectLine = new QLineEdit(this);
    subjectLabel->setGeometry(50,80,subjectLabel->width(),20);
    subjectLine->setGeometry(50+subjectLabel->width(),80,this->width()-100-subjectLabel->width(),20);

    messageText = new QTextEdit(this);
    messageText->setGeometry(50,110,this->width()-100,this->height()-180);

    sendButton = new QPushButton(QString::fromUtf8("Send"),this);
    sendButton->move(50,this->height()-50);
    connect(sendButton,SIGNAL(clicked()),this,SLOT(clicked_on_sendButton()));

    configButton = new QPushButton(QString::fromUtf8("Config"),this);
    configButton->move(70 + sendButton->width(),sendButton->pos().y());
    connect(configButton,SIGNAL(clicked()),this,SLOT(clicked_on_configButton()));

    exitButton = new QPushButton(QString::fromUtf8("Exit"),this);
    exitButton->move(this->width() - 50 - exitButton->width(),sendButton->pos().y());
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));
}

MailWidget::~MailWidget()
{

}

void MailWidget::start()
{
    cw = new ConfigWidget(&conf,this);
    cw->show();
}

void MailWidget::clicked_on_exitButton()
{
    this->close();
}

void MailWidget::clicked_on_configButton()
{
    cw = new ConfigWidget(&conf,this);
    cw->show();
}

void MailWidget::clicked_on_sendButton()
{
    this->setEnabled(false);

    client = new SmtpClient(conf.server,conf.port);
    connect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler()));

    SmtpMessage mail;

    mail.setName(conf.name);
    mail.setFromAddress(conf.login);
    mail.setToAddress(toLine->text());
    mail.setSubject(subjectLine->text());
    mail.setText(messageText->toPlainText());

    if(client->connectToHost())
    {
        if(client->login(conf.login,conf.password))
        {
            if(client->sendMail(mail))
            {
                client->quit();
                disconnect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler()));
                delete client;
                success = new QMessageBox;
                success->setWindowTitle("Success");
                success->addButton("OK",success->AcceptRole);
                success->setText("Message was sent");
                connect(success,SIGNAL(accepted()),success,SLOT(close()));
                success->show();
                this->setEnabled(true);
            }
        }
    }
}

void MailWidget::errorHandler()
{
    client->blockSignals(true);
    Error = new QWidget;
    Error->setWindowTitle(QString::fromUtf8("ERROR"));
    Error->setFixedSize(400,120);
    errorLabel = new QLabel(Error);
    errorLabel->setWordWrap(true);
    errorLabel->setText(client->getLastWords());
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setGeometry(20,10,Error->width() - 40, Error->height() - 40);
    okButton = new QPushButton(QString::fromUtf8("OK"),Error);
    okButton->move(Error->width()/2-40,Error->height() - 40);
    connect(okButton,SIGNAL(clicked()),this,SLOT(clicked_on_okButton()));
    Error->show();
}

void MailWidget::clicked_on_okButton()
{
    client->quit();
    disconnect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler()));
    delete client;
    Error->close();
    this->setEnabled(true);
    Error->deleteLater();
}