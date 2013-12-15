#ifndef SMTPMESSAGE_H
#define SMTPMESSAGE_H

#include <QObject>

class SmtpMessage : public QObject
{
    Q_OBJECT

public:
    SmtpMessage(QObject *parent = 0);

    void setName(const QString &name);
    void setFromAddress(const QString &fromAddress);
    void setToAddress(const QString &toAddress);
    void setSubject(const QString &subject);
    void setText(const QString &text);

    QString getName() const;
    QString getFromAddress() const;
    QString getToAddress() const;
    QString getSubject() const;
    QString getText() const;

    QString getMail() const;

    ~SmtpMessage();
public slots:

private:
    QString name;
    QString fromAddress;
    QString toAddress;
    QString subject;
    QString text;
};

#endif // SMTPMESSAGE_H
