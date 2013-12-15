#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QSslSocket>
#include <QFile>
#include "smtpmessage.h"

class SmtpClient : public QObject
{
    Q_OBJECT

public:
    SmtpClient(const QString & host = "localhost", quint16 port = 25);

    enum SmtpError
    {
        ConnectionTimeoutError,
        ResponseTimeoutError,
        AuthorizationRequiredError,
        AuthenticationFaileError,
        ServerError,
        ClientError
    };

    bool sendMail(const SmtpMessage &mail);

    bool login();

    bool login(const QString & user, const QString & password);

    bool connectToHost();

    bool quit();

    QString getLastWords();

    ~SmtpClient();

signals:

    void smtpError(SmtpError e);

    void stringError(QString error);

public slots:

    void connected();

    void ready();

    void errorRecieved(QAbstractSocket::SocketError);

    void disconnected();

protected:

    QString name;
    QString host;
    quint16 port;

    quint16 connectionTimeout;
    quint16 responseTimeout;

    QString user;
    QString password;

    bool waitForResponse();

    void sendMessage(const QString &text);

    quint16 responseCode;
    QString responseText;

    QSslSocket *socket;
    QFile *file;
};

#endif // SMTP_H
