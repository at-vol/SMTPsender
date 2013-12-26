#include "smtpclient.h"

SmtpClient::SmtpClient(const QString & host, quint16 port) :
    name("localhost"),
    connectionTimeout(1000),
    responseTimeout(1000),
    user("0"),
    password("0")
{
    socket = new QSslSocket(this);
    connect(socket,SIGNAL(encrypted()),this,SLOT(ready()));
    connect(socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorRecieved(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    this->host = host;
    this->port = port;
    file = new QFile("log.txt");
    file->open(QIODevice::Append);
}

SmtpClient::~SmtpClient()
{
    file->close();
    delete file;
    delete socket;
}

void SmtpClient::setHost(const QString host)
{
    this->host = host;
}

void SmtpClient::setPort(const quint16 port)
{
    this->port = port;
}

bool SmtpClient::connectToHost()
{
    file->write("---BEGIN---\n");

    socket->connectToHostEncrypted(host,port);

    if(!socket->waitForConnected(connectionTimeout))
    {
        emit smtpError(ConnectionTimeoutError);
        return false;
    }

    if(!waitForResponse())
        return false;

    if(responseCode != 220)//220 - <domain> Service ready
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(EHLO_C + name);

    if(!waitForResponse())
        return false;

    if(responseCode != 250)//250 - Requested mail action okay, completed
    {
        emit smtpError(ServerError);
        return false;
    }


    return true;
}

bool SmtpClient::login(const QString &user, const QString &password)
{
    sendMessage(AUTHP_C + QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());

    if(!waitForResponse())
        return false;

    if(responseCode != 235)//235 - Authentication succeeded
    {
        sendMessage(AUTHL_C);

        if(!waitForResponse())
            return false;

        if(responseCode != 334)//334 - Text part containing the [BASE64] encoded string
        {
            emit smtpError(AuthenticationFailedError);
            return false;
        }

        sendMessage(QByteArray().append(user).toBase64());

        if(!waitForResponse())
            return false;

        if(responseCode != 334)
        {
            emit smtpError(AuthenticationFailedError);
            return false;
        }

        sendMessage(QByteArray().append(password).toBase64());

        if(!waitForResponse())
            return false;

        if(responseCode != 235)
        {
            emit smtpError(AuthenticationFailedError);
            return false;
        }
    }

    return true;
}

bool SmtpClient::sendMail(const SmtpMessage &mail)
{
    sendMessage(MAIL_C + "<"+ mail.getFromAddress() + ">");

    if(!waitForResponse())
        return false;

    /*When trying to sendmail without AUTH
    if(responseCode == 530)//530 - Authentication required
    {
        emit smtpError(AuthorizationRequiredError);
        return false;
    }
    */

    else if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(RCPT_C + "<" + mail.getToAddress() + ">");

    if(!waitForResponse())
        return false;

    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(DATA_C);

    if(!waitForResponse())
        return false;

    if(responseCode != 354)//354 - Start mail input; end with <CRLF>.<CRLF>
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(mail.getMail());
    sendMessage(".");

    if(!waitForResponse())
        return false;

    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    return true;
}

bool SmtpClient::waitForResponse()
{
    do {
        if(!socket->waitForReadyRead(responseTimeout))
        {
            emit smtpError(ResponseTimeoutError);
            return false;
        }

        while(socket->canReadLine())
        {
            responseText = socket->readLine();
            file->write(responseText.toAscii());

            responseCode = responseText.left(3).toInt();

            if(responseCode / 100 == 4)//4xx - Server errors
                emit smtpError(ServerError);
            if(responseCode / 100 == 5)//5xx - Client errors
                emit smtpError(ClientError);

            if(responseText[3] == ' ') { return true; }

        }
    } while(true);
}

void SmtpClient::sendMessage(const QString &text)
{
    socket->write(text.toUtf8()+"\r\n");
    file->write(text.toUtf8()+"\n");
}

bool SmtpClient::quit()
{
    if(socket->isEncrypted())
    {
        sendMessage(QUIT_C);
        waitForResponse();
        if(responseCode != 221)//221 - <domain> Service closing transmission channel
            return false;
    }
    socket->abort();
    file->write("---END---\n\n");
    return true;
}

void SmtpClient::connected()
{
    file->write("Connected.\n");
}

void SmtpClient::errorRecieved(QAbstractSocket::SocketError socketError)
{
    file->write("SocketError[" + QString().number(socketError).toUtf8() + "]: "
                + socket->errorString().toUtf8() + "\n");
    emit smtpError(SocketError);
}

void SmtpClient::ready()
{
    file->write("Encrypted.\n");
}

void SmtpClient::disconnected()
{
    file->write("Disconnected.\n");
}
