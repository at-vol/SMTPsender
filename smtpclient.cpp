#include "smtpclient.h"

SmtpClient::SmtpClient(const QString & host, quint16 port, EncryptionType encryption) :
    name("localhost"),
    connectionTimeout(1000),
    responseTimeout(1000),
    user("0"),
    password("0")
{
    setEncryptionType(encryption);

    connect(socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorRecieved(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    this->host = host;
    this->port = port;
    file = new QFile("log.txt");
}

SmtpClient::~SmtpClient()
{
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

SmtpClient::EncryptionType SmtpClient::getEncryptionType() const
{
    return encryptionType;
}

bool SmtpClient::connectToHost()
{
    writeLog("---BEGIN---\n");

    switch(encryptionType)
    {
    case NONE:
    case STARTTLS:
        socket->connectToHost(host,port);
        break;
    case SSL:
        ((QSslSocket *)socket)->connectToHostEncrypted(host,port);
    }

    if(!socket->waitForConnected(connectionTimeout))
    {
        emit smtpError(ConnectionTimeoutError);
        return false;
    }

    if(!waitForResponse())
        return false;

    //220 - <domain> Service ready
    if(responseCode != 220)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(EHLO_C + name);

    if(!waitForResponse())
        return false;

    //250 - Requested mail action okay, completed
    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    if(encryptionType == STARTTLS)
    {
        sendMessage(STARTTLS_C);

        if(!waitForResponse())
            return false;

        //220 - <domain> Service ready
        if(responseCode != 220)
        {
            emit smtpError(ServerError);
            return false;
        }

        ((QSslSocket *)socket)->startClientEncryption();

        if(!((QSslSocket *)socket)->waitForEncrypted(connectionTimeout))
        {
            emit smtpError(ConnectionTimeoutError);
            return false;
        }

        sendMessage(EHLO_C + name);

        if(!waitForResponse())
            return false;

        //250 - Requested mail action okay, completed
        if(responseCode != 250)
        {
            emit smtpError(ServerError);
            return false;
        }
    }

    return true;
}

bool SmtpClient::login(const QString &user, const QString &password)
{
    sendMessage(AUTHP_C + QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());

    if(!waitForResponse())
        return false;

    //235 - Authentication succeeded
    if(responseCode != 235)
    {
        sendMessage(AUTHL_C);

        if(!waitForResponse())
            return false;

        //334 - Text part containing the [BASE64] encoded string
        if(responseCode != 334)
        {
            emit smtpError(AuthenticationFailedError);
            return false;
        }

        sendMessage(QByteArray().append(user).toBase64());

        if(!waitForResponse())
            return false;

        //334 - Text part containing the [BASE64] encoded string
        if(responseCode != 334)
        {
            emit smtpError(AuthenticationFailedError);
            return false;
        }

        sendMessage(QByteArray().append(password).toBase64());

        if(!waitForResponse())
            return false;

        //235 - Authentication succeeded
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

    //250 - Requested mail action okay, completed
    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(RCPT_C + "<" + mail.getToAddress() + ">");

    if(!waitForResponse())
        return false;

    //250 - Requested mail action okay, completed
    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(DATA_C);

    if(!waitForResponse())
        return false;

    //354 - Start mail input; end with <CRLF>.<CRLF>
    if(responseCode != 354)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage(mail.getMail());
    sendMessage(".");

    if(!waitForResponse())
        return false;

    //250 - Requested mail action okay, completed
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
            writeLog(responseText.toAscii());

            responseCode = responseText.left(3).toInt();

            //4xx - Server errors
            if(responseCode / 100 == 4)
                emit smtpError(ServerError);
            //5xx - Client errors
            if(responseCode / 100 == 5)
                emit smtpError(ClientError);

            if(responseText[3] == ' ') { return true; }

        }
    } while(true);
}

void SmtpClient::sendMessage(const QString &text)
{
    socket->write(text.toUtf8()+"\r\n");
    writeLog(text.toUtf8()+"\n");
}

void SmtpClient::writeLog(const char* data)
{
    file->open(QIODevice::Append);
    file->write(data);
    file->close();
}

bool SmtpClient::quit()
{
    if(socket->state()==QAbstractSocket::ConnectedState)
    {
        sendMessage(QUIT_C);
        waitForResponse();
        //221 - <domain> Service closing transmission channel
        if(responseCode != 221)
            return false;
    }
    socket->abort();
    writeLog("---END---\n\n");
    return true;
}

void SmtpClient::connected()
{
    writeLog("Connected.\n");
}

void SmtpClient::errorRecieved(QAbstractSocket::SocketError socketError)
{
    writeLog("SocketError[" + QString().number(socketError).toUtf8() + "]: "
                + socket->errorString().toUtf8() + "\n");
    emit smtpError(SocketError);
}

void SmtpClient::ready()
{
    writeLog("Encrypted.\n");
}

void SmtpClient::disconnected()
{
    writeLog("Disconnected.\n");
}

void SmtpClient::setEncryptionType(EncryptionType et)
{
    this->encryptionType = et;
    switch(encryptionType)
    {
    case NONE:
        socket = new QTcpSocket(this);
        break;
    case STARTTLS:
    case SSL:
        socket = new QSslSocket(this);
        connect(socket,SIGNAL(encrypted()),this,SLOT(ready()));
    }
}
