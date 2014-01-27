#include <QGridLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include "mailwidget.h"

MailWidget::MailWidget(QWidget *parent)
    : QWidget(parent),
      conf(new CONFIG)
{
    setWindowTitle(QString::fromUtf8("Send mail"));
    setMinimumSize(400,400);
    resize(this->minimumSize());
    QRect frect = frameGeometry();
    frect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(frect.topLeft());

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

    warningLabel = new QLabel(tr(""),this);
    warningLabel->setAlignment(Qt::AlignCenter);
    mainlayout->addWidget(warningLabel,BUTTONS,0,1,2);

    QHBoxLayout *boxlayout = new QHBoxLayout;

    configButton = new QPushButton(tr("Config"),this);
    connect(configButton,SIGNAL(clicked()),this,SLOT(clicked_on_configButton()));

    sendButton = new QPushButton(QString::fromUtf8("Send"),this);
    connect(sendButton,SIGNAL(clicked()),this,SLOT(clicked_on_sendButton()));

    exitButton = new QPushButton(QString::fromUtf8("Exit"),this);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));


    boxlayout->addWidget(configButton,0,Qt::AlignLeft);
    boxlayout->addWidget(exitButton,0,Qt::AlignRight);
    boxlayout->addWidget(sendButton,0,Qt::AlignRight);
    mainlayout->addLayout(boxlayout,BUTTONS,2,Qt::AlignRight);

    mainlayout->setColumnStretch(0,1);
    mainlayout->setColumnStretch(1,10);
    mainlayout->setColumnStretch(2,5);

    setLayout(mainlayout);
}

MailWidget::~MailWidget()
{
    delete conf;
    delete client;
}

void MailWidget::start()
{
    config = new QFile("config");

    if(config->exists())
    {
        config->open(QIODevice::ReadOnly);
        QByteArray temp;
        temp = config->readLine();
        temp.chop(1);
        conf->smtpServer = QString(temp);
        temp = config->readLine();
        temp.chop(1);
        conf->smtpPort = temp.toInt();
        temp = config->readLine();
        temp.chop(1);
        conf->encryption = temp.toInt();
        temp = config->readLine();
        temp.chop(1);
        conf->login = QString(temp);
        temp = config->readLine();
        temp.chop(1);
        conf->password = QString(temp);
        temp = config->readLine();
        temp.chop(1);
        conf->name = QString(temp);
        config->close();

        delete config;

        this->show();
    }
    else
    {
        cw = new ConfigWidget(conf,this);
        connect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
        cw->show();
    }
    client = new SmtpClient(conf->smtpServer,conf->smtpPort,SmtpClient::EncryptionType(conf->encryption));
}

void MailWidget::clicked_on_configButton()
{
    cw = new ConfigWidget(conf,this);
    connect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
    cw->show();
}

void MailWidget::configChanged()
{
    SmtpClient::EncryptionType en = client->getEncryptionType();
    disconnect(cw,SIGNAL(destroyed()),this,SLOT(configChanged()));
    if((en == SmtpClient::NONE || SmtpClient::EncryptionType(conf->encryption) == SmtpClient::NONE) && en != conf->encryption)
    {
        delete client;
        client = new SmtpClient(conf->smtpServer, conf->smtpPort, SmtpClient::EncryptionType(conf->encryption));
    }
    else
    {
        if(en != conf->encryption)
            client->changeEncryptionType(SmtpClient::EncryptionType(conf->encryption));
        client->setHost(conf->smtpServer);
        client->setPort(conf->smtpPort);
    }
}

void MailWidget::clicked_on_exitButton()
{
    this->close();
}

void MailWidget::clicked_on_sendButton()
{
    int pos = toLine->text().length();
    QString text = toLine->text();
    if(toLine->validator()->validate(text,pos)!=QValidator::Acceptable)
    {
        warningLabel->setStyleSheet("QLabel{border: 2px solid red;"
                                    "border-radius: 5px;}");
        warningLabel->setText(tr("Incorrect e-mail format"));
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

    mail.setName(conf->name);
    mail.setFromAddress(conf->login);
    mail.setToAddress(toLine->text());
    mail.setSubject(subjectLine->text());
    mail.setText(messageText->toPlainText());

    if(client->connectToHost())
    {
        if(client->login(conf->login,conf->password))
        {
            if(client->sendMail(mail))
            {
                disconnect(client,SIGNAL(smtpError(SmtpError)),
                           this,SLOT(errorHandler(SmtpError)));
                QMessageBox::information(this,tr("Success"),tr("Message has been sent"),
                                         QMessageBox::Ok,QMessageBox::Ok);
                this->setEnabled(true);
            }
        }
    }
    client->quit();
}

void MailWidget::errorHandler(SmtpError e)
{
    client->blockSignals(true);

    QString text;

    if(e == UntrustedCertificateError)
    {
        text = "The server's' SSL certificate is not trusted by your computer.\n"
                "If you continue, it may cause security problems.\n"
                "Accept the certificate anyway?";
        int ret = QMessageBox::warning(this,tr("Security warning"),text, QMessageBox::No | QMessageBox::Yes,QMessageBox::No);

        if(ret == QMessageBox::Yes)
            client->setIgnoreSslErrors(true);
        else disconnect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler(SmtpError)));
    }
    else
    {
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
            text.append("Check your network. Connection timeout");
            break;
        case(ResponseTimeoutError):
            text.append("Try again. Response timeout");
            break;
        case(AuthorizationRequiredError):
            text.append("Authorization required");
            break;
        case(AuthenticationFailedError):
            text.append("Check your login and password. Authentication failed");
            break;
        case(InsecureConnectionError):
            text.append("Check encryption settings. Insecure connection");
            break;
        case(SslHandshakeError):
            text.append("Untrusted certificate. Unreliable connection");
            break;
        default:
            text.append("Unknown");
        }

        text.append(" error.");

        QMessageBox::critical(this,tr("Error"),text,QMessageBox::Ok,QMessageBox::Ok);

        disconnect(client,SIGNAL(smtpError(SmtpError)),this,SLOT(errorHandler(SmtpError)));
    }
    client->blockSignals(false);
    this->setEnabled(true);
}
