#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTcpSocket>
#include <QtCore>

#define MODEPP_INCLUDE_MESSAGE_TYPES_ONLY //prevent including implementations
#include "MoDePP.h"

namespace Ui
{
    class MainWindow;
}

typedef QMap< QString,QList<QString> > FunctionsMap;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void showAboutDialog();
    void on_actionAbout_activated();
    void on_actionAbout_Qt_activated();
    void on_actionSource_activated();
    void on_bConnect_clicked();
    void on_bExecute_clicked();
    void on_cbFunction_activated ( const QString & );

    void onDataAvailable();
    void onConnected();

private:
    Ui::MainWindow *ui;
    bool _connected;
    QTcpSocket *_socket;
    QByteArray _data;
    QMap<QString,QMap<int, QVariant> > _functionValues;
    FunctionsMap _functions;
};

#endif // MAINWINDOW_H
