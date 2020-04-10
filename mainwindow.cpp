#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openFileButton = new QPushButton("Open File", this);
    openFileButton->setGeometry(QRect(QPoint(20, 10), QSize(95, 30)));
    connect(openFileButton, SIGNAL(released()), this, SLOT(handleOpenFileButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// TODO: REMOVE AFTER DEBUGGING
void delay()
{
    QTime dieTime= QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::handleOpenFileButton()
{
    QLabel* debuggingLabel = new QLabel(this);
    debuggingLabel->setGeometry(500, 10, 500, 30);
    debuggingLabel->show();

    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "C:\\Users\\vladi\\OneDrive\\Documents\\Datasets",
                                                    tr("Plain text tables (*.csv *.tsv)"));

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    dataTable = new QTableWidget(this);
    dataTable->setGeometry(20, 50, 560, 525);
    dataTable->show();

    // Set columns number and a header for the table
    QString header = QString::fromUtf8(file.readLine());
    dataTable->setColumnCount(header.count(separator)+1);
    dataTable->setHorizontalHeaderLabels(header.split(separator));

    int row = 0;
    int col = 0;

    // Map for counting classes and their objects
    QMap<QString, int> classes;

    while (!file.atEnd())
    {
        dataTable->insertRow(row);
        col = 0;
        QString line = QString::fromUtf8(file.readLine());

        foreach(QString cell, line.trimmed().split(separator)) {
            QTableWidgetItem* cellItem = new QTableWidgetItem(cell);
            dataTable->setItem(row, col, cellItem);

            // Insert classes in map and count them
            if (col == class_column)
            {
                if (classes.contains(cell)) {
                    classes.insert(cell, classes.value(cell)+1);
                } else {
                    classes.insert(cell, 1);
                }
            }
            col++;
        }
        row++;
    }

    int n_features = dataTable->columnCount();
    int n_objects = dataTable->rowCount();

    // Create labels with numbers of rows and columns
    QLabel* rowCountLabel = new QLabel(this);
    QLabel* colCountLabel = new QLabel(this);

    rowCountLabel->setGeometry(300, 10, 100, 30);
    colCountLabel->setGeometry(400, 10, 100, 30);

    rowCountLabel->setText("Rows: " + QString::number(n_objects));
    colCountLabel->setText("Cols: " + QString::number(n_features));

    rowCountLabel->show();
    colCountLabel->show();

    // Create and fill tables with statistics for classes
    QMap<QString, QTableWidget*> classesStatisticsTables;
    int statisticsTableYPosition = 50;
    const int statisticsTableXPosition = 600;
    const int statisticsTableHeight = 155;
    const int statisticsTableWidth = 420;
    const int offset = 30;

    foreach(QString dataClass, classes.keys()) {

        QLabel* label = new QLabel(this);
        label->setGeometry(statisticsTableXPosition, statisticsTableYPosition-20, statisticsTableWidth, 20);
        label->setText("Statistics of " + dataClass + " class");
        label->show();

        classesStatisticsTables[dataClass] = new QTableWidget(this);
        classesStatisticsTables[dataClass]->setGeometry(statisticsTableXPosition, statisticsTableYPosition,
                                                        statisticsTableWidth, statisticsTableHeight);

        statisticsTableYPosition += statisticsTableHeight + offset;

        classesStatisticsTables[dataClass]->setColumnCount(3); // n, mean and variance
        classesStatisticsTables[dataClass]->setHorizontalHeaderLabels({"n", "Mean", "Variance"});
        classesStatisticsTables[dataClass]->setRowCount(n_features-1);

        int current_col = 0; // We have to track column we fill not counting class column
        for(int i = 0; i < n_features; ++i)
        {
            if (i != class_column)
            {
                classesStatisticsTables[dataClass]->setVerticalHeaderItem(current_col, dataTable->horizontalHeaderItem(i));
                classesStatisticsTables[dataClass]->setItem(current_col, 0, new QTableWidgetItem(QString::number(classes[dataClass])));
                current_col++;
            }
        }
        classesStatisticsTables[dataClass]->show();
    }

    // A map which stores statistics for each class. class->statistics->value
    QMap<QString, QMap<QString, float>> statisticsMap;

    // Go through a table and fill means and variances
    int current_col = 0;
    for (int col = 0; col < n_features; ++col)
    {
        if (col != class_column)
        {
            foreach(QString dataClass, classes.keys()) {
                statisticsMap[dataClass]["Sum"] = 0;
                statisticsMap[dataClass]["Squares_sum"] = 0;
            }
    
            for (int row = 0; row < n_objects; ++row) {
                QString dataClass = dataTable->item(row, class_column)->text();
                float value = dataTable->item(row, col)->text().toFloat();
    
                statisticsMap[dataClass]["Sum"] += value;
                statisticsMap[dataClass]["Squares_sum"] += qPow(value, 2);
            }

            foreach(QString dataClass, classes.keys()) {
                int n = classes.value(dataClass);
                QString mean = QString::number(statisticsMap[dataClass]["Sum"] / n);
                QString variance = QString::number(statisticsMap[dataClass]["Squares_sum"] / n - qPow(statisticsMap[dataClass]["Sum"]/n, 2));
                classesStatisticsTables[dataClass]->setItem(current_col, 1, new QTableWidgetItem(mean));
                classesStatisticsTables[dataClass]->setItem(current_col, 2, new QTableWidgetItem(variance));
            }
            current_col++;  // We do not increment current_col if col is the class column
        }

    }

    // Create and fill distances table
    QTableWidget* distancesTable = new QTableWidget(this);
    distancesTable->setGeometry(20, 620, 450, 150);


    QLabel* distancesTableLabel = new QLabel(this);
    distancesTableLabel->setGeometry(20, 600, 450, 20);
    distancesTableLabel->setText("Table of distances between classes");
    distancesTableLabel->show();

    distancesTable->setRowCount(classes.size());
    distancesTable->setColumnCount(classes.size());
    distancesTable->setHorizontalHeaderLabels(classes.keys());
    distancesTable->setVerticalHeaderLabels(classes.keys());
    distancesTable->show();

    row = 0;
    foreach(QString dataClass_i, classes.keys()) {
        int col = 0;
        foreach(QString dataClass_j, classes.keys()) {
            float distance = 0;
            for(int feature = 0; feature < classesStatisticsTables[dataClass_i]->rowCount(); feature++) {
                distance +=  qPow(classesStatisticsTables[dataClass_i]->item(feature, 1)->text().toFloat() -
                                  classesStatisticsTables[dataClass_j]->item(feature, 1)->text().toFloat(), 2);
            }
            distance = sqrt(distance);
            distancesTable->setItem(col, row, new QTableWidgetItem(QString::number(distance)));
            distancesTable->setItem(row, col, new QTableWidgetItem(QString::number(distance)));
            col++;
        }
        row++;
    }

    // Create prediction quality table
    QTableWidget* qualityTable = new QTableWidget(this);
    qualityTable->setGeometry(statisticsTableXPosition, 620, statisticsTableWidth, 150);
    qualityTable->setRowCount(classes.size());
    qualityTable->setColumnCount(classes.size());
    qualityTable->setHorizontalHeaderLabels(classes.keys());
    qualityTable->setVerticalHeaderLabels(classes.keys());
    qualityTable->show();

    QLabel* qualityTableLabel = new QLabel(this);
    qualityTableLabel->setGeometry(statisticsTableXPosition, 600, statisticsTableWidth, 20);
    qualityTableLabel->setText("Quality of prediction. Rows - true labels, columns - predicted");
    qualityTableLabel->show();

    QMap<QString, int> classesIndexes; // We need to know indexes of classes for tables
    int i = 0;
    foreach(QString dataClass, classes.keys()) {
        classesIndexes[dataClass] = i;
        i++;
    }

    for(int row = 0; row < classes.size(); ++row) {
        for (int col = 0; col < classes.size(); ++col) {
            qualityTable->setItem(row, col, new QTableWidgetItem(QString::number(0)));
        }
    }

    // Predict classes for the same data

    float accuracy = 0;

    dataTable->insertColumn(dataTable->columnCount());
    int predicted_class_col = dataTable->columnCount()-1;
    dataTable->setHorizontalHeaderItem(predicted_class_col, new QTableWidgetItem("predicted"));

    for(int row = 0; row < n_objects; ++row)
    {
        float min_distance = FLT_MAX;
        QString nearest_class = "Undefined";

        foreach(QString dataClass, classes.keys()) // Find the nearest class
        {
            float distance = 0;
            int current_col = 0;
            for(int col = 0; col < n_features; ++col)
            {
                if (col != class_column)
                {
                    distance += qPow(dataTable->item(row, col)->text().toFloat() -
                                     classesStatisticsTables[dataClass]->item(current_col, 1)->text().toFloat(), 2);
                    current_col++;
                }
            }
            distance = sqrt(distance);

            if (distance < min_distance) {
                min_distance = distance;
                nearest_class = dataClass;
            }
        }

        dataTable->setItem(row, predicted_class_col, new QTableWidgetItem(nearest_class));

        QString trueClass = dataTable->item(row, class_column)->text();

        if (nearest_class == trueClass) {
            accuracy += 1;
        }

        int nearestClassIndex = classesIndexes[nearest_class];
        int trueClassIndex = classesIndexes[trueClass];

        qualityTable->setItem(trueClassIndex, nearestClassIndex,
                              new QTableWidgetItem(QString::number(qualityTable->item(trueClassIndex, nearestClassIndex)->text().toInt()+1)));
    }

    accuracy = accuracy * 100 / n_objects;

    QLabel* accuracyLabel = new QLabel(this);
    accuracyLabel->setGeometry(statisticsTableXPosition, 780, statisticsTableWidth, 20);
    accuracyLabel->setText("Accuracy of prediction: " + QString::number(accuracy));
    accuracyLabel->show();

    // Calculate sensitivity, specificity and linear discriminant function
    if (classes.size()==2)
    {
        // Assuming that the first class is negative and the second is positive
        int true_positive = qualityTable->item(1, 1)->text().toInt();
        int false_positive = qualityTable->item(0, 1)->text().toInt();
        int true_negative = qualityTable->item(0, 0)->text().toInt();
        int false_negative= qualityTable->item(1, 0)->text().toInt();

        float sensitivity = (float)true_positive / (true_positive + false_negative);
        float specificity = (float)true_negative / (true_negative + false_positive);

        QLabel* sensitivityLabel = new QLabel(this);
        sensitivityLabel->setGeometry(statisticsTableXPosition, 800, statisticsTableWidth, 20);
        sensitivityLabel->setText("Sensitivity of prediction: " + QString::number(sensitivity));
        sensitivityLabel->show();

        QLabel* specificityLabel = new QLabel(this);
        specificityLabel->setGeometry(statisticsTableXPosition, 820, statisticsTableWidth, 20);
        specificityLabel->setText("Specificity of prediction: " + QString::number(specificity));
        specificityLabel->show();

        float intercept = 0;
        QMap<QString, float> features_coefficients;

        for (int i=0; i < n_features-1; ++i) {
            QString feature = classesStatisticsTables[classes.keys()[0]]->verticalHeaderItem(i)->text().trimmed();
            features_coefficients.insert(feature, 0);
        }

        QString class_1 = classes.keys()[0];
        QString class_2 = classes.keys()[1];

        // Calculate LDF
        for (int feature_row = 0; feature_row < n_features-1; ++feature_row)
        {
            // Differences of the squares of means between class 2 and class 1
            intercept += qPow(classesStatisticsTables[class_2]->item(feature_row, 1)->text().toFloat(), 2) -
                         qPow(classesStatisticsTables[class_1]->item(feature_row, 1)->text().toFloat(), 2);

            // Twice the difference between the means of class 1 and class 2
            QString feature = classesStatisticsTables[classes.keys()[0]]->verticalHeaderItem(feature_row)->text().trimmed();
            features_coefficients[feature] = 2 *
                    (classesStatisticsTables[class_1]->item(feature_row, 1)->text().toFloat()
                    -classesStatisticsTables[class_2]->item(feature_row, 1)->text().toFloat());
        }

        QString result = "";
        foreach(QString feature, features_coefficients.keys())
        {
            result += feature + "*" + QString::number(features_coefficients[feature]) + " + ";
        }

        QLabel* discriminantFunctionLabel = new QLabel(this);
        discriminantFunctionLabel->setGeometry(20, 840, 400, 20);
        discriminantFunctionLabel->setText("d(class_" + class_1 + "|class_" + class_2 + ") = " + result + QString::number(intercept));
        discriminantFunctionLabel->show();
    }






}

