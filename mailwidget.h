#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QMessageBox>
#include "smtpclient.h"
#include "configwidget.h"


class MailWidget : public QWidget
{
    Q_OBJECT
    
public:
    MailWidget(QWidget *parent = 0);
    ~MailWidget();
    void start();
protected slots:
    void errorHandler();

    void clicked_on_exitButton();

    void clicked_on_sendButton();

    void clicked_on_configButton();

    void clicked_on_okButton();
private:
    ConfigWidget *cw;

    QPushButton *sendButton;

    QPushButton *exitButton;

    QPushButton *configButton;

    QLabel *toLabel;
    QLineEdit *toLine;

    QLabel *subjectLabel;
    QLineEdit *subjectLine;

    QTextEdit *messageText;

    CONFIG conf;

    SmtpClient *client;

    SmtpClient::SmtpError error;

    QMessageBox *success;

    QWidget *Error;
    QLabel *errorLabel;
    QPushButton *okButton;
};

#endif // WIDGET_H
