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
    map = "";
    style = "";

    map = "C:\\pilocik\\map\\";
    style = "C:\\pilocik\\style\\partitionMapStyle.xml";

    osmscout::Partitioning part(map, style);
    //part.TestAlgorithm();
    //part.saveToDatabase("c:\\map\\partition.db");

    //part.FindPartition();

    /*TEST CODE
    std::vector <osmscout::Partitioning::PartNode> nodes;
    osmscout::Partitioning::PartWay way;
    way.id=100;
    for(int i=55;i<100;i++){
        osmscout::Partitioning::PartNode node;
        node.lon=i*10;
        node.lat=i*1.3;
        node.cell=0;
        node.id=i;
        nodes.push_back(node);
        way.nodes.push_back(nodes.size()-1);
    }
    std::vector <osmscout::Partitioning::PartWay> ways;
    ways.push_back(way);
    std::vector <osmscout::Partitioning::BoundaryEdge> edges;

    osmscout::PartitionModel pm;
    pm.open("c:\\map\\partition.db");
    pm.exportToDatabase(ways,nodes,edges);
    */

    osmscout::Routing r;
    r.CalculateRoute(258184089, 60145225);
}

