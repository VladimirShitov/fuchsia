#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QDialog>
#include <QLabel>
#include <QMap>
#include <QtMath>
#include <QTime>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleOpenFileButton();

private:
    Ui::MainWindow *ui;
    QPushButton *openFileButton;
    QTableWidget *dataTable;
    QString separator = " ";
    qint8 class_column = 0;

};
#endif // MAINWINDOW_H
