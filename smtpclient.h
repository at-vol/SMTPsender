#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QSslSocket>
#include <QFile>
#include "smtpmessage.h"

const QString EHLO_C = "EHLO ";
const QString STARTTLS_C = "STARTTLS";
const QString AUTHP_C = "AUTH PLAIN ";
const QString AUTHL_C = "AUTH LOGIN";
const QString MAIL_C = "MAIL FROM: ";
const QString RCPT_C = "RCPT TO: ";
const QString DATA_C = "DATA";
const QString QUIT_C = "QUIT";

enum SmtpError
{
    ConnectionTimeoutError,
    ResponseTimeoutError,
    AuthorizationRequiredError,
    AuthenticationFailedError,
    ServerError,
    ClientError,
    SocketError
};

class SmtpClient : public QObject
{
    Q_OBJECT

public:
    enum EncryptionType{NONE, STARTTLS, SSL};

    SmtpClient(const QString & host = "localhost", quint16 port = 25, EncryptionType encryption = SSL);

    void setHost(const QString host);

    void setPort(const quint16 port);

    EncryptionType getEncryptionType() const;

    bool sendMail(const SmtpMessage &mail);

    bool login(const QString & user, const QString & password);

    bool connectToHost();

    bool quit();

    ~SmtpClient();

signals:

    void smtpError(SmtpError e);

public slots:

    void connected();

    void ready();

    void errorRecieved(QAbstractSocket::SocketError);

    void disconnected();

protected:
    void setEncryptionType(EncryptionType et);

    QString name;
    QString host;
    quint16 port;
    EncryptionType encryptionType;

    quint16 connectionTimeout;
    quint16 responseTimeout;

    QString user;
    QString password;

    bool waitForResponse();

    void sendMessage(const QString &text);

    quint16 responseCode;
    QString responseText;

    QTcpSocket *socket;
    QFile *file;

    void writeLog(const char *data);
};

#endif // SMTP_H
