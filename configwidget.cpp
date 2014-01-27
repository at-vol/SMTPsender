#include <QGridLayout>
#include <QBoxLayout>
#include "configwidget.h"

#define SMTP_MTA "25"  //mail transfer agent
#define SMTP_SSL "465"   //SMTP with SSL encryption
#define SMTP_MSA "587"   //mail submission agent
#define SMTP_ALT "2525"   //alternative port

#define IMAP_STD "143"
#define IMAP_SSL "993"

//encryption types
#define NONE_E "NONE"
#define STARTTLS_E "STARTTLS"
#define SSL_E "SSL/TLS"

ConfigWidget::ConfigWidget(CONFIG * c, QWidget *parent)
    : Parent(parent),
      conf(c)
{
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Configuration"));
    setFixedSize(370,280);
    QRect frect = frameGeometry();
    frect.moveCenter(Parent->frameGeometry().center());
    move(frect.topLeft());

    QGridLayout *mainlayout = new QGridLayout;

    smtpServerLabel = new QLabel(QString::fromUtf8("SMTP server"),this);
    smtpServerLine = new QLineEdit(this);
    mainlayout->addWidget(smtpServerLabel,SMTPSERVER,0);
    mainlayout->addWidget(smtpServerLine,SMTPSERVER,1);
    smtpServerLine->setValidator(new QRegExpValidator(QRegExp("^[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,254}[a-zA-Z\\d])"
                                                        "\\.[a-zA-Z]{2,6}$")));

    smtpPortLabel = new QLabel(QString::fromUtf8("SMTP port"),this);
    smtpPortBox = new QComboBox(this);
    QStringList list;
    list << SMTP_MTA << SMTP_SSL << SMTP_MSA << SMTP_ALT;
    smtpPortBox->addItems(list);
    mainlayout->addWidget(smtpPortLabel,SMTPPORT,0);
    mainlayout->addWidget(smtpPortBox,SMTPPORT,1);

    encryptionLabel = new QLabel(QString::fromUtf8("Encryption"),this);
    encryptionBox = new QComboBox(this);
    list.clear();
    list << NONE_E << STARTTLS_E << SSL_E;
    encryptionBox->addItems(list);
    mainlayout->addWidget(encryptionLabel,ENCRYPTION,0);
    mainlayout->addWidget(encryptionBox,ENCRYPTION,1);

    userLabel = new QLabel(QString::fromUtf8("Login"),this);
    userLine  = new QLineEdit(this);
    mainlayout->addWidget(userLabel,LOGIN,0);
    mainlayout->addWidget(userLine,LOGIN,1);
    userLine->setValidator(new QRegExpValidator(QRegExp("^(?:([a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,62}[a-zA-Z\\d])"
                                                        "@[a-zA-Z\\d](?:[a-zA-Z\\d]?|[\\w\\-\\.]{0,180}[a-zA-Z\\d])"
                                                        "\\.[a-zA-Z]{2,6})|(\\w{1,4}\\\\[\\w\\-\\.]{,62}))$")));

    passwdLabel = new QLabel(QString::fromUtf8("Password"),this);
    passwdLine = new QLineEdit(this);
    mainlayout->addWidget(passwdLabel,PASSWORD,0);
    mainlayout->addWidget(passwdLine,PASSWORD,1);
    passwdLine->setValidator(new QRegExpValidator(QRegExp("^\\S+$")));
    passwdLine->setEchoMode(QLineEdit::Password);

    nameLabel = new QLabel(QString::fromUtf8("Name"),this);
    nameLine = new QLineEdit(this);
    mainlayout->addWidget(nameLabel,NAME,0);
    mainlayout->addWidget(nameLine,NAME,1);
    nameLine->setValidator(new QRegExpValidator(QRegExp("^\\w{0,15} ?\\w{0,15}$")));

    saveCheck = new QCheckBox(QString::fromUtf8("save as default"),this);
    mainlayout->addWidget(saveCheck,SAVE,0,1,2,Qt::AlignCenter);

    QBoxLayout *buttonsbox = new QBoxLayout(QBoxLayout::RightToLeft);

    warningLabel = new QLabel("",this);

    okButton = new QPushButton(QString::fromUtf8("OK"),this);
    okButton->setAutoDefault(true);
    connect(okButton,SIGNAL(clicked()),this,SLOT(clicked_on_okButton()));
    okButton->setFocus();

    exitButton = new QPushButton(QString::fromUtf8((Parent->isVisible())?"Cancel":"Exit"),this);
    exitButton->setAutoDefault(true);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(clicked_on_exitButton()));

    buttonsbox->addWidget(okButton);
    buttonsbox->addWidget(exitButton);
    buttonsbox->addWidget(warningLabel,10,Qt::AlignLeft);
    mainlayout->addLayout(buttonsbox,BUTTONS,0,1,2);
    setLayout(mainlayout);

    smtpServerLine->setText(conf->smtpServer);
    for(int i=0;i<smtpPortBox->count();i++)
        if(smtpPortBox->itemText(i).toInt()==conf->smtpPort)
        {
            smtpPortBox->setCurrentIndex(i);
            break;
        }
    encryptionBox->setCurrentIndex(conf->encryption);
    userLine->setText(conf->login);
    passwdLine->setText(conf->password);
    nameLine->setText(conf->name);
}

ConfigWidget::~ConfigWidget()
{
}

bool ConfigWidget::checkValidation(const QLineEdit *line) const
{
    int pos = line->text().length();
    QString text = line->text();
    if(line->validator()->validate(text,pos)==QValidator::Acceptable)
        return true;
    else return false;
}

void ConfigWidget::clicked_on_exitButton()
{
    if(!Parent->isVisible())
        Parent->close();
    this->close();
}

void ConfigWidget::clicked_on_okButton()
{
    QLineEdit *lines[4];

    lines[0] = smtpServerLine;
    lines[1] = userLine;
    lines[2] = passwdLine;
    lines[3] = nameLine;

    for(int i=0; i<4; i++)
        if(!checkValidation(lines[i]))
        {
            warningLabel->setStyleSheet("QLabel{border: 2px solid red;"
                                        "border-radius: 5px;}");
            switch(i)
            {
            case 0:
                warningLabel->setText(QString::fromUtf8("Wrong server address!"));
                break;
            case 1:
                warningLabel->setText(QString::fromUtf8("Wrong e-mail address!"));
                break;
            case 2:
                warningLabel->setText(QString::fromUtf8("Not valid password!"));
                break;
            case 3:
                warningLabel->setText(QString::fromUtf8("Not valid name!"));
                break;
            }
            return;
        }

    conf->smtpServer = smtpServerLine->text();
    conf->smtpPort = smtpPortBox->currentText().toInt();
    conf->encryption = encryptionBox->currentIndex();
    conf->login = userLine->text();
    conf->password = passwdLine->text();
    conf->name = nameLine->text();

    if(saveCheck->isChecked())
    {
        QFile *config = new QFile("config");
        config->open(QIODevice::WriteOnly);
        config->write(conf->smtpServer.toUtf8()+"\n");
        config->write(QString().number(conf->smtpPort).toUtf8()+"\n");
        config->write(QString().number(conf->encryption).toUtf8()+"\n");
        config->write(conf->login.toUtf8()+"\n");
        config->write(conf->password.toUtf8()+"\n");
        config->write(conf->name.toUtf8()+"\n");
        config->close();
        delete config;
    }

    if(!Parent->isVisible())
        Parent->show();
    this->close();
}
