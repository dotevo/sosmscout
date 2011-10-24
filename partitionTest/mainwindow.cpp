#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <iomanip>

#include <QFileInfo>
#include <QDir>

#include <QMouseEvent>
#include <QLineF>

#include <QtGui/QPixmap>
#include <QtGui/QApplication>

#include <QDebug>

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/Util.h>
#include <osmscout/MapPainterQt.h>
#include <osmscout/Partitioning.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    map = "";
    style = "";

#ifdef Q_OS_UNIX
     map = "/home/bartek/osmscout-map/3poland/";
    style = "/home/bartek/QtProjects/OSMNavi/styles/standard.oss2.xml";
#endif

#ifdef Q_OS_WIN
    map = "C:/map";
    style = "C:/map/standard.oss.xml";
#endif

    osmscout::Partitioning part(map, style);
    //part.TestAlgorithm();
    part.FindPartition();
}

