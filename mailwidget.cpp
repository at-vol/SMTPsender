#include <QGridLayout>
#include <QHBoxLayout>
#include "mailwidget.h"

MailWidget::MailWidget(QWidget *parent)
    : QWidget(parent)
{
    client = new SmtpClient;
    setWindowTitle(QString::fromUtf8("SMTP sender"));
    setMinimumSize(400,400);

    QGridLayout *mainlayout = new QGridLayout;

    toLabel = new QLabel(QString::fromUtf8("To:"),this);
    toLine = new QLineEdit(this);
    mainlayout->addWidget(toLabel,MAIL_TO,0);
    mainlayout->addWidget(toLine,MAIL_TO,1,1,2);
    toLine->setValidator(new QRegExpValidator(QRegExp("^[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,62}[a-zA-Z\\d])"
                                                      "@[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,180}[a-zA-Z\\d])"
                                                      "\\.[a-zA-Z]{2,6}$")));

    subjectLabel = new QLabel(QString::fromUtf8("Subject:"),this);
    subjectLine = new QLineEdit(this);
    mainlayout->addWidget(subjectLabel,SUBJECT,0);
    mainlayout->addWidget(subjectLine,SUBJECT,1,1,2);

    messageText = new QTextEdit(this);
    mainlayout->addWidget(messageText,MESSAGE,0,1,3);

    QHBoxLayout *boxlayout = new QHBoxLayout;

    sendButton = new QPushButton(QString::fromUtf8("Send"),this);
    boxlayout->addWidget(sendButton);
    connect(sendButton,SIGNAL(clicked()),this,SLOT(clicked_on_sendButton()));

    configButton = new QPushButton(QString::fromUtf8("Config"),this);
    boxlayout->addWidget(configButton);
    connect(configButton,SIGNAL(clicked()),this,SLOT(clicked_on_configButton()));

    mainlayout->addLayout(boxlayout,BUTTONS,0,1,2,Qt::AlignLeft);

    boxlayout = new QHBoxLayout;

    warningLabel = new QLabel(tr(""),this);
    boxlayout->addWidget(warningLabel,10,Qt::AlignLeft);

    exitButton = new QPushButton(QString::fromUtf8("Exit"),this);
    boxlayout->addWidget(exitButton,0,Qt::AlignRight);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));

    mainlayout->addLayout(boxlayout,BUTTONS,2);
    mainlayout->setColumnStretch(0,1);
    mainlayout->setColumnStretch(1,10);
    mainlayout->setColumnStretch(2,10);

    setLayout(mainlayout);
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
    int pos = toLine->text().length();
    QString text = toLine->text();
    if(toLine->validator()->validate(text,pos)!=QValidator::Acceptable)
    {
        warningLabel->setStyleSheet("QLabel{border: 2px solid red;"
                                    "border-radius: 5px;}");
        warningLabel->setText(tr("Wrong e-mail"));
        warningLabel->setVisible(true);
        return;
    }
    else warningLabel->setVisible(false);

    if(subjectLine->text().isEmpty())
    {
        int ret = QMessageBox::question(this,tr("Empty subject"),
                                        tr("The subject is empty. Continue?"),
                                        QMessageBox::Yes|QMessageBox::No,
                                        QMessageBox::No);
        if(ret==QMessageBox::No)
            return;
    }

    if(messageText->toPlainText().isEmpty())
    {
        int ret = QMessageBox::question(this,tr("Empty body"),
                                        tr("Your message have no text. Continue?"),
                                        QMessageBox::Yes|QMessageBox::No,
                                        QMessageBox::No);
        if(ret==QMessageBox::No)
            return;
    }

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
