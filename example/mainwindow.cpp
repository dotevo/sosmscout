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

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/Util.h>
#include <osmscout/MapPainterQt.h>

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
   // map = "/home/bartek/QtProjects/OSMNavi/map/";
    map = "/home/bartek/osmscout-map/montenegro/";
    style = "/home/bartek/QtProjects/OSMNavi/styles/standard.oss.xml";
#endif

#ifdef Q_OS_WIN
    map = "/ResidentFlash/ZPI/map";
    style = "/ResidentFlash/ZPI/standard.oss.xml";
#endif

    width = 480;
    height = 272;
    lon = 17.03;

    lat = 51.1;
    zoom = 10000;

    this->ui->zoomSlider->setValue(zoom);
    this->ui->zoomSlider->setMinimum(100);
    this->ui->zoomSlider->setMaximum(100000);
    this->ui->zoomSlider->setSingleStep(1000);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    DrawMap();
}


void MainWindow::mousePressEvent(QMouseEvent *e)
{
    moving = true;
    startPoint = e->globalPos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    moving = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (moving)
    {
        finishPoint = e->globalPos();
        QLineF line(startPoint, finishPoint);

        double dx = line.dx();
        double dy = line.dy();

        lon -= dx/zoom;
        lat += dy/zoom;

        startPoint = finishPoint;

        repaint();
    }
}

int MainWindow::DrawMap()
{
    // std::cerr << "DrawMapQt <map directory> <style-file> <width> <height> <lon> <lat> <zoom> <output>" << std::endl;
//    std::cerr << "Default values!";

    osmscout::DatabaseParameter databaseParameter;
    osmscout::Database          database(databaseParameter);

	if (!database.Open(map)) {
        std::cerr << "Cannot open database" << std::endl;
        return 1;
    }

    osmscout::StyleConfig styleConfig(database.GetTypeConfig());

	if (!osmscout::LoadStyleConfig((const char*)style.toLatin1(),styleConfig)) {
        std::cerr << "Cannot open style" << std::endl;
    }

    QPainter* painter=new QPainter(this);

        if (painter!=NULL) {
            osmscout::MercatorProjection  projection;
            osmscout::MapParameter        drawParameter;
            osmscout::AreaSearchParameter searchParameter;
            osmscout::MapData             data;
            osmscout::MapPainterQt        mapPainter;

            projection.Set(lon,
                           lat,
                           zoom,
                           width,
                           height);


            database.GetObjects(styleConfig,
                                projection.GetLonMin(),
                                projection.GetLatMin(),
                                projection.GetLonMax(),
                                projection.GetLatMax(),
                                projection.GetMagnification(),
                                searchParameter,
                                data.nodes,
                                data.ways,
                                data.areas,
                                data.relationWays,
                                data.relationAreas);

            if (mapPainter.DrawMap(styleConfig,
                                   projection,
                                   drawParameter,
                                   data,
                                   painter)) {
 //               std::cerr << "Drawing!" << std::endl;
 //               std::cerr << "Zoom: " << zoom << std::endl;

            }

            delete painter;
        }
        else {
            std::cout << "Cannot create QPainter" << std::endl;
        }
}

void MainWindow::on_zoomSlider_valueChanged(int value)
{
    zoom = value;
    this->repaint();
}
