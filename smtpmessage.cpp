#include "smtpmessage.h"

SmtpMessage::SmtpMessage(QObject *parent) :
    QObject(parent),
    name(""),
    fromAddress(""),
    toAddress(""),
    subject(""),
    text("")
{
}

SmtpMessage::~SmtpMessage()
{

}

void SmtpMessage::setName(const QString & name)
{
    this->name = name;
}

void SmtpMessage::setFromAddress(const QString & fromAddress)
{
    this->fromAddress = fromAddress;
}

void SmtpMessage::setToAddress(const QString & toAddress)
{
    this->toAddress = toAddress;
}

void SmtpMessage::setSubject(const QString & subject)
{
    this->subject = subject;
}

void SmtpMessage::setText(const QString & text)
{
    this->text = text;
}

QString SmtpMessage::getName() const
{
    return this->name;
}
QString SmtpMessage::getFromAddress() const
{
    return this->fromAddress;
}
QString SmtpMessage::getToAddress() const
{
    return this->toAddress;
}
QString SmtpMessage::getSubject() const
{
    return this->subject;
}
QString SmtpMessage::getText() const
{
    return this->text;
}

QString SmtpMessage::getMail() const
{
    QString message;

    message = "To: <" + this->toAddress + ">\n";
    message.append("From: \""+ this->name + "\" <" + this->fromAddress + ">\n");
    message.append("Subject: " + this->subject + "\n");
    message.append(this->text);
    message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );

    return message;
}
