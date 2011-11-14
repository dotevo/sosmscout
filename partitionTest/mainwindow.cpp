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
#include <osmscout/Partitionmodel.h>
#include <osmscout/Routing.h>

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
    osmscout::Partitioning part;

    //part.InitData();
    //part.SaveData("D:\\pilocik\\map\\partition.txt");

    //part.LoadData("D:\\pilocik\\map\\partTest.txt");
    //part.CalculatePriorities();
    //part.SavePriorities("D:\\pilocik\\map\\prioTest.txt");

    part.LoadData("D:\\pilocik\\map\\partTestA.txt");
    //part.LoadPriorities("D:\\pilocik\\map\\prioTest.txt");
    osmscout::Partitioning::DatabasePartition dbPart=part.FindPartition();
    part.saveToDatabase("D:\\pilocik\\map\\partTestA.db",dbPart);

    //osmscout::Routing r;
    //r.CalculateRoute(1, 34);

    part.Delete();
}

