#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;
    QString   map;
    QString   style;
    QString   output;
    size_t        width,height;
    double        lon,lat,zoom;
    // map moving
    bool moving;
    QPoint startPoint;
    QPoint finishPoint;

    void init();
};

#endif // MAINWINDOW_H
