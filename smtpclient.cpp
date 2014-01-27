#include "smtpclient.h"

SmtpClient::SmtpClient(const QString & host, quint16 port, EncryptionType encryption) :
    name("localhost"),
    ignoreSslErrors(false),
    connectionTimeout(1000),
    responseTimeout(1000),
    user("0"),
    password("0")
{
    // Add your own CA cerificate exceptions
    QSslSocket::addDefaultCaCertificates("certs/*",QSsl::Pem,QRegExp::WildcardUnix);

    file = new QFile("log.txt");

    setEncryptionType(encryption);

    connect(socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorRecieved(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    this->host = host;
    this->port = port;
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

void SmtpClient::changeEncryptionType(SmtpClient::EncryptionType e)
{
    this->encryptionType = e;
}

void SmtpClient::setConnectionTimeout(quint16 time)
{
    this->connectionTimeout = time;
}

void SmtpClient::setIgnoreSslErrors(bool ignore)
{
    this->ignoreSslErrors = ignore;
}

SmtpClient::EncryptionType SmtpClient::getEncryptionType() const
{
    return encryptionType;
}

bool SmtpClient::connectToHost()
{
    writeLog("---BEGIN---\n");

    if(encryptionType == NONE)
    {
        socket->connectToHost(host,port);
        if(!socket->waitForConnected(connectionTimeout))
        {
            emit smtpError(ConnectionTimeoutError);
            return false;
        }
    }
    else
    {
        do
        {
            if(encryptionType == STARTTLS )
            {
                socket->connectToHost(host,port);

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
            }
            else ((QSslSocket *)socket)->connectToHostEncrypted(host,port);

            if(!((QSslSocket *)socket)->waitForEncrypted(connectionTimeout))
            {
                if(socket->error() == QAbstractSocket::SslHandshakeFailedError)
                {
                    if(ignoreSslErrors == false)
                    {
                        emit smtpError(SslHandshakeError);
                        return false;
                    }
                }
                else
                {
                    emit smtpError(ConnectionTimeoutError);
                    return false;
                }
            }
            else ignoreSslErrors = false;
        }while(ignoreSslErrors);
    }

    if(encryptionType != STARTTLS)
    {
        if(!waitForResponse())
            return false;

        //220 - <domain> Service ready
        if(responseCode != 220)
        {
            emit smtpError(ServerError);
            return false;
        }
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

    return true;
}

bool SmtpClient::login(const QString &user, const QString &password)
{
    if(encryptionType != NONE && !((QSslSocket *)socket)->isEncrypted())
    {
        emit smtpError(InsecureConnectionError);
        return false;
    }

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

            if(responseText[3] == ' ') return true;

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
    return true;
}

void SmtpClient::connected()
{
    writeLog("Connected.\n");
}

void SmtpClient::errorRecieved(QAbstractSocket::SocketError socketError)
{
    writeLog(QString("SocketError[%1]: %2").arg(socketError).arg(socket->errorString()).toUtf8() + "\n");
    if(socketError != QAbstractSocket::SslHandshakeFailedError && socketError != QAbstractSocket::SocketTimeoutError)
        emit smtpError(SocketError);
}

void SmtpClient::errorRecieved(const QList<QSslError>& sslErrors)
{
    if(ignoreSslErrors)
        ((QSslSocket *) socket)->ignoreSslErrors();
    else
    {
        bool flag = false;

        foreach( const QSslError &error, sslErrors )
        {
            if(error.error() == QSslError::CertificateUntrusted)
            {
                flag = true;
                break;
            }
        }

        if(flag)
            emit smtpError(UntrustedCertificateError);
        else
            emit smtpError(SslHandshakeError);
    }
}

void SmtpClient::ready()
{
    writeLog("Encrypted.\n");
}

void SmtpClient::disconnected()
{
    writeLog("Disconnected.\n---END---\n\n");
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
        connect(socket,SIGNAL(sslErrors(const QList<QSslError>&)),this,SLOT(errorRecieved(const QList<QSslError>&)));
    }
}
