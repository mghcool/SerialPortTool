#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    void on_pause_triggered(bool checked);

    void on_start_triggered(bool checked);

    void on_stop_triggered(bool checked);

    void on_btnSend_clicked();

    void Read_Data();

private:
    Ui::MainWindow *ui;

    void GetPortList();
};
#endif // MAINWINDOW_H
