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
    moving = false;
    scaling = false;

    map = "";
    style = "";

#ifdef Q_OS_UNIX
     map = "/home/bartek/osmscout-map/3poland/";
    style = "/home/bartek/QtProjects/OSMNavi/styles/standard.oss.xml";
#endif

#ifdef Q_OS_WIN
    map = "/ResidentFlash/ZPI/map";
    style = "/ResidentFlash/ZPI/standard.oss.xml";
#endif


    translatePoint = QPoint(0, 0);
    lastPoint = QPoint(0, 0);

    width = 480;
    height = 272;
    lat = 51.1;
    lon = 17.03;

    zoom = 10000;

    pixmap = QPixmap(width, height);
    pixmap.fill(QColor(200, 200, 200));

    zoomLevels.append(100);
    zoomLevels.append(200);
    zoomLevels.append(300);
    zoomLevels.append(400);
    zoomLevels.append(500);
    zoomLevels.append(600);
    zoomLevels.append(700);
    zoomLevels.append(800);
    zoomLevels.append(900);
    zoomLevels.append(1000);
    zoomLevels.append(5000);
    zoomLevels.append(10000);
    zoomLevels.append(20000);
    zoomLevels.append(40000);
    zoomLevels.append(50000);
    zoomLevels.append(60000);
    zoomLevels.append(70000);
    zoomLevels.append(80000);
    zoomLevels.append(90000);
    zoomLevels.append(100000);

    this->ui->zoomSlider->setValue(zoom);
    this->ui->zoomSlider->setMinimum(100);
    this->ui->zoomSlider->setMaximum(100000);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    DrawMap();
}


void MainWindow::mousePressEvent(QMouseEvent *e)
{
    moving = true;
    startPoint = e->globalPos();
    lastPoint = e->globalPos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    moving = false;

    finishPoint = e->globalPos();
    QLineF line(startPoint, finishPoint);

    double dx = line.dx();
    double dy = line.dy();

    lon -= dx/zoom;
    lat += dy/zoom;

    translatePoint = QPoint(0, 0);
    startPoint = QPoint(0, 0);

    repaint();
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (moving)
    {
        translatePoint = e->globalPos() - startPoint;

        repaint();
    }
}

int MainWindow::DrawMap()
{
    // std::cerr << "DrawMapQt <map directory> <style-file> <width> <height> <lon> <lat> <zoom> <output>" << std::endl;
//    std::cerr << "Default values!";

    if (moving)
    {
        int x = translatePoint.x();
        int y = translatePoint.y();


        QPainter *windowPainter = new QPainter(this);
        windowPainter->drawPixmap(x, y, pixmap);

    }

    else if (scaling)
    {

    }

    else
    {
        osmscout::DatabaseParameter databaseParameter;
        osmscout::Database          database(databaseParameter);

		if (!database.Open((const char*)map.toAscii())) {
            std::cerr << "Cannot open database" << std::endl;
            return 1;
        }

        osmscout::StyleConfig styleConfig(database.GetTypeConfig());

        if (!osmscout::LoadStyleConfig((const char*)style.toAscii(),styleConfig)) {
            std::cerr << "Cannot open style" << std::endl;
        }

        QPainter* painter = new QPainter(&pixmap);

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

            QPainter *windowPainter = new QPainter(this);
            windowPainter->drawPixmap(0, 0, pixmap);
        }
        else {
            std::cout << "Cannot create QPainter" << std::endl;
        }
    }
}

void MainWindow::on_zoomSlider_valueChanged(int value)
{
    zoom = value;
    repaint();
}

void MainWindow::on_plusZoom_clicked()
{
    int index = getIndexOfActualZoom(zoom);
    if (index < zoomLevels.count() - 1)
    {
        MainWindow::ui->zoomSlider->setValue(zoomLevels.at(index + 1));
    //    on_zoomSlider_valueChanged(zoomLevels.at(index + 1));
    }
}

void MainWindow::on_minusZoom_clicked()
{
    int index = getIndexOfActualZoom(zoom);
    if (index > 0)
    {
        MainWindow::ui->zoomSlider->setValue(zoomLevels.at(index + 1));
    }
}

int MainWindow::getIndexOfActualZoom(int actZoom)
{
    return zoomLevels.lastIndexOf(actZoom);
}


