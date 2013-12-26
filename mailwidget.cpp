#include "mailwidget.h"

MailWidget::MailWidget(QWidget *parent)
    : QWidget(parent)
{
    client = new SmtpClient;
    this->setWindowTitle(QString::fromUtf8("SMTP sender"));
    this->setFixedSize(600,500);

    toLabel = new QLabel(QString::fromUtf8("To:"),this);
    toLine = new QLineEdit(this);
    toLabel->setGeometry(50,50,toLabel->width(),20);
    toLine->setGeometry(50+toLabel->width(),50,this->width()-100-toLabel->width(),20);
    toLine->setValidator(&valid.mail);
    connect(toLine,SIGNAL(textChanged(QString)),this,SLOT(validation(QString)));

    subjectLabel = new QLabel(QString::fromUtf8("Subject:"),this);
    subjectLine = new QLineEdit(this);
    subjectLabel->setGeometry(50,80,subjectLabel->width(),20);
    subjectLine->setGeometry(50+subjectLabel->width(),80,
                             this->width()-100-subjectLabel->width(),20);

    messageText = new QTextEdit(this);
    messageText->setGeometry(50,110,this->width()-100,this->height()-180);

    sendButton = new QPushButton(QString::fromUtf8("Send"),this);
    sendButton->move(50,this->height()-50);
    sendButton->setEnabled(false);
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
    delete client;
}

void MailWidget::start()
{
    cw = new ConfigWidget(&conf,this);
    connect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
    cw->show();
}

void MailWidget::configChanged()
{
    disconnect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
    client->setHost(conf.server);
    client->setPort(conf.port);
}

void MailWidget::clicked_on_exitButton()
{
    this->close();
}

void MailWidget::clicked_on_configButton()
{
    cw = new ConfigWidget(&conf,this);
    connect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
    cw->show();
}

void MailWidget::clicked_on_sendButton()
{
    this->setEnabled(false);
    connect(client,SIGNAL(smtpError(SmtpError)),
            this,SLOT(errorHandler(SmtpError)));
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
                disconnect(client,SIGNAL(smtpError(SmtpError)),
                           this,SLOT(errorHandler(SmtpError)));
                QMessageBox::information(this,tr("Success"),tr("Message has been sent"),
                                         QMessageBox::Ok,QMessageBox::Ok);
                this->setEnabled(true);
            }
        }
    }
}

void MailWidget::validation(QString text)
{
    int pos = toLine->text().length();
    if(valid.mail.validate(text,pos)==QValidator::Acceptable)
    {
        if(!sendButton->isEnabled())
            sendButton->setEnabled(true);
    }
    else if(sendButton->isEnabled())
            sendButton->setEnabled(false);
}

void MailWidget::errorHandler(SmtpError e)
{
    client->blockSignals(true);
    client->quit();
    QString text;
    switch(e)
    {
    case(SocketError):
        text.append("Socket");
        break;
    case(ClientError):
        text.append("Client");
        break;
    case(ServerError):
        text.append("Server");
        break;
    case(ConnectionTimeoutError):
        text.append("Connection timeout");
        break;
    case(ResponseTimeoutError):
        text.append("Response timeout");
        break;
    case(AuthorizationRequiredError):
        text.append("Authorization required");
        break;
    case(AuthenticationFailedError):
        text.append("Authentication failed");
        break;
    default:
        text.append("Unknown");
    }
    text.append(" error.");
    QMessageBox::critical(this,tr("Error"),text,QMessageBox::Ok,QMessageBox::Ok);
    client->blockSignals(false);
    disconnect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler(SmtpError)));
    this->setEnabled(true);
}
