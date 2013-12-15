#include "smtpclient.h"

SmtpClient::SmtpClient(const QString & host, quint16 port) :
    name("localhost"),
    connectionTimeout(5000),
    responseTimeout(5000),
    user("0"),
    password("0")
{
    this->host = host;
    this->port = port;
    file = new QFile("log.txt");
    file->open(QIODevice::Append);
}

SmtpClient::~SmtpClient()
{
    file->write("END\n");
    file->close();
    delete file;
    delete socket;
}

bool SmtpClient::connectToHost()
{
    socket = new QSslSocket(this);

    connect(socket,SIGNAL(encrypted()),this,SLOT(ready()));
    connect(socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorRecieved(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    socket->connectToHostEncrypted(host,port);

    if(!socket->waitForConnected(connectionTimeout))
    {
        emit smtpError(ConnectionTimeoutError);
        return false;
    }

    if(!waitForResponse())
        return false;

    if(responseCode != 220)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage("EHLO " + name);

    if(!waitForResponse())
        return false;

    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }


    return true;
}

bool SmtpClient::login()
{
    return login(user,password);
}

bool SmtpClient::login(const QString &user, const QString &password)
{
    sendMessage("AUTH PLAIN " + QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());

    if(!waitForResponse())
        return false;

    if(responseCode != 235)
    {
        sendMessage("AUTH LOGIN");

        if(!waitForResponse())
            return false;

        if(responseCode != 334)
        {
            emit smtpError(AuthenticationFaileError);
            return false;
        }

        sendMessage(QByteArray().append(user).toBase64());

        if(!waitForResponse())
            return false;

        if(responseCode != 334)
        {
            emit smtpError(AuthenticationFaileError);
            return false;
        }

        sendMessage(QByteArray().append(password).toBase64());

        if(!waitForResponse())
            return false;

        if(responseCode != 235)
        {
            emit smtpError(AuthenticationFaileError);
            return false;
        }
    }

    return true;
}

bool SmtpClient::sendMail(const SmtpMessage &mail)
{
    sendMessage("MAIL FROM: <"+ mail.getFromAddress() + ">");

    if(!waitForResponse())
        return false;

    if(responseCode == 503)
    {
        emit smtpError(AuthorizationRequiredError);
        return false;
    }
    else if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage("RCPT TO: <" + mail.getToAddress() + ">");

    if(!waitForResponse())
        return false;

    if(responseCode != 250)
    {
        emit smtpError(ServerError);
        return false;
    }

    sendMessage("DATA");

    if(!waitForResponse())
        return false;

    if(responseCode != 354)
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

            if(responseCode / 100 == 4)
                emit smtpError(ServerError);
            if(responseCode / 100 == 5)
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
    sendMessage("QUIT");
    waitForResponse();
    if(responseCode != 221)
        return false;
    socket->abort();
    return true;
}

void SmtpClient::connected()
{
    file->write("Connected.\n");
}

void SmtpClient::errorRecieved(QAbstractSocket::SocketError socketError)
{
    file->write("ERROR: " + socket->errorString().toUtf8() + "\n");
    emit stringError("ERROR: " + socketError);
}

void SmtpClient::ready()
{
    file->write("Encrypted.\n");
}

void SmtpClient::disconnected()
{
    file->write("Disconnected.\n");
}

QString SmtpClient::getLastWords()
{
    return responseText;
}
