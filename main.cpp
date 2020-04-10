#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QTime>
#include <QPainter>
#include <QPixmap>

static const int LOAD_TIME_MSEC = 0.5*1000;
static const int PROGRESS_X = 100;
static const int PROGRESS_Y = 400;
static const int PROGRESS_WIDTH = 300;
static const int PROGRESS_HEIGHT = 30;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap("C:\\Users\\vladi\\OneDrive\\Documents\\QT_projects\\Perceptron\\fuchsia.png");

    QSplashScreen loadscreen(pixmap);
    loadscreen.show();

    QTime time;
    time.start();

    while (time.elapsed() < LOAD_TIME_MSEC) {
        qint8 progress = 100. / LOAD_TIME_MSEC * time.elapsed();
        loadscreen.showMessage(QString::number(progress) +  "% loaded", Qt::AlignBottom | Qt::AlignCenter);

        QPainter painter;

        painter.begin(&pixmap);
        painter.fillRect(PROGRESS_X, PROGRESS_Y, progress*1./100*PROGRESS_WIDTH, PROGRESS_HEIGHT, Qt::gray);
        painter.end();

        loadscreen.setPixmap(pixmap);
        a.processEvents();
    }

    MainWindow w;
    w.show();
    return a.exec();
}
