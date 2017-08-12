#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include <QMessageBox>
#include <QStringList>
#include <QTimer>
#include <QTextStream>
#include <QFileDialog>

const static int DEBUG_PORT = 4545;

QStringList Responses;
Ui::MainWindow *GlobUi=0;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),_connected(false), _socket(0)
{
    ui->setupUi(this);
    GlobUi = ui;
    QTimer *t=new QTimer(this);
   // connect(t, SIGNAL(timeout()), this, SLOT(pollResponse()));
    t->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showAboutDialog()
{
    new About;
}

void MainWindow::on_actionAbout_activated()
{
  (new About)->show();
}

void MainWindow::on_actionAbout_Qt_activated()
{
  QApplication::aboutQt();
}

void MainWindow::on_bConnect_clicked()
{
    if(_connected)
    {
        delete _socket;
        _socket=0;
        this->ui->bConnect->setText("Connect");
    }
    else
    {
        _socket = new QTcpSocket(this);
        connect(_socket,SIGNAL(readyRead()), this,SLOT(onDataAvailable()) );
        connect(_socket,SIGNAL(connected()), this,SLOT(onConnected()) );
        _socket->connectToHost( ui->eAddress->text(), ui->ePort->text().toInt() );
        this->ui->bConnect->setText("Close");
    }
    _connected = !_connected;
}

void MainWindow::on_bExecute_clicked()
{
    if(_socket)
    {
        QTextStream ts(_socket);
        ts.setFieldWidth(4);
        ts.setPadChar( QChar('0') );
        ts.setIntegerBase(16);
        QString fname, p1,p2,p3,p4,p5;
        QString msg;
        fname=ui->cbFunction->currentText();
        p1=ui->eParam1->text();
        p2=ui->eParam2->text();
        p3=ui->eParam3->text();
        p4=ui->eParam4->text();
        p5=ui->eParam5->text();
        int len=fname.length();
        int entirelen = fname.length()+p1.length()+p2.length()+p3.length()+p4.length()+p5.length()+4*6;


        ts << entirelen << MsgCallFunction << fname.length();
        ts.setFieldWidth(0);
        ts<<fname;
        ts.setFieldWidth(4);
        ts<<p1.length();
        ts.setFieldWidth(0);
        ts<<p1;
        ts.setFieldWidth(4);
        ts<<p2.length();
        ts.setFieldWidth(0);
        ts<<p2;
        ts.setFieldWidth(4);
        ts<<p3.length();
        ts.setFieldWidth(0);
        ts<<p3;
        ts.setFieldWidth(4);
        ts<<p4.length();
        ts.setFieldWidth(0);
        ts<<p4;
        ts.setFieldWidth(4);
        ts<<p5.length();
        ts.setFieldWidth(0);
        ts<<p5;
        _functionValues[fname][1]=p1;
        _functionValues[fname][2]=p2;
    }
}

void MainWindow::on_actionSource_activated()
{
    QString prjdir = QFileDialog::getExistingDirectory(this, "Directory to extract sources");
    if (!prjdir.isEmpty())
    {
		QDir resdir(":/src");
		foreach( QString fn, resdir.entryList() )
		{
			QFile sf(fn);
			QString path=prjdir+"/"+fn;
			sf.copy(path);
		}
    }

}

void MainWindow::on_cbFunction_activated ( const QString & fname )
{
    ui->eParam1->setText( _functionValues[fname][1].toString());
    ui->eParam2->setText( _functionValues[fname][2].toString());
    ui->eParam3->setText( _functionValues[fname][3].toString());
    ui->eParam4->setText( _functionValues[fname][4].toString());
    ui->eParam5->setText( _functionValues[fname][5].toString());
    static QLineEdit* paramContainers[]={ ui->eParam1,ui->eParam2,ui->eParam3,ui->eParam4,ui->eParam5 };
    static QLabel* paramNames[]={ ui->label_p1,ui->label_p2,ui->label_p3,ui->label_p4,ui->label_p5 };
    QList<QString> params = _functions[fname];    
    for (int j=0; j<5; ++j)
    {
        paramContainers[j]->hide();
    }
    int i=0;
    foreach ( QString p, params  )
    {

        if (!p.isEmpty())
        {
			static QLineEdit* paramContainers[]={ ui->eParam1,ui->eParam2,ui->eParam3,ui->eParam4,ui->eParam5 };
    static QLabel* paramNames[]={ ui->label_p1,ui->label_p2,ui->label_p3,ui->label_p4,ui->label_p5 };
            //ui->tResponse->append(p+", ");
            paramContainers[i]->show();
            paramNames[i]->setText(p);
        }
        ++i;
    }
}

void MainWindow::onConnected()
{
    if(_socket)
    {
        ui->cbFunction->clear();
        QTextStream ts(_socket);
        ts << "0000";
        ts.setFieldWidth(4); ts.setIntegerBase(16);
        ts<<MsgListFunctions;
        for (int j=0; j<5; ++j)
        {
			static QLineEdit* paramContainers[]={ ui->eParam1,ui->eParam2,ui->eParam3,ui->eParam4,ui->eParam5 };
    static QLabel* paramNames[]={ ui->label_p1,ui->label_p2,ui->label_p3,ui->label_p4,ui->label_p5 };
            paramContainers[j]->hide();
			paramNames[j]->hide();
        }
    }
}

void MainWindow::onDataAvailable()
{
    if(_socket)
    {
       _data+= _socket->readAll();
       //ui->tResponse->append(_data+"\n");
       if (_data.size() >=HEADER_LEN )
       {
           QTextStream ts(_data);
           while( !ts.atEnd() )
           {
               int cmd=0,len=0;
               bool ok;
               len = ts.read(4).toInt(&ok,16);
               cmd = ts.read(4).toInt(0,16);
               if (cmd == MsgAddFunction)
               {
                   QString tmp = ts.read(len);
                   QTextStream ts(&tmp);
                   QString fn,tmpparam;
                   QList<QString> fp;
                   ts >> fn;
                   while(!ts.atEnd())
                   {
                       ts >> tmpparam;
                       fp.append(tmpparam);
                   }
                   _functions.insert(fn, fp);
                   ui->cbFunction->addItem(fn);
               }
               else if (cmd == MsgTrace)
               {
                   QString tmp = ts.read(len);
                   ui->tResponse->append( QString("TRC: ")+tmp );
               }
               else if (cmd == MsgReturn)
               {
                   QString tmp = ts.read(len);
                   ui->tResponse->append( QString("RET: ")+tmp );
               }
               else
               {
                    ui->tResponse->append( QString("ERROR: Unknown message[") + cmd + "] " +ts.readAll() );
               }
           }
           _data.clear();
       }
    }
}
