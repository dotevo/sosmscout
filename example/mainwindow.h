#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPainter>
#include <QPoint>
#include <QPixmap>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int DrawMap();
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

private slots:
    void on_zoomSlider_valueChanged(int value);

    void on_plusZoom_clicked();

    void on_minusZoom_clicked();

private:
    Ui::MainWindow *ui;

    QString   map;
    QString   style;
    QString   output;

    size_t        width,height;
    double        lon,lat,zoom;

    QPixmap pixmap;
    // map operations
    bool moving;
    bool scaling;

    QPoint startPoint;  // mouse is pressed
    QPoint finishPoint; // mouse is released
    QPoint lastPoint;   // last position of mouse
    QPoint translatePoint;  // translation point

    QList<int> zoomLevels;
    int getIndexOfActualZoom(int actZoom);


    void init();
};

#endif // MAINWINDOW_H
