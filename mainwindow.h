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

    void on_clean_triggered();

    void on_btnSend_clicked();

    void slot_PortReceive();

    void slot_UpdatePort();

private:
    Ui::MainWindow *ui;
    QTimer *timerUpdatePort;

    void UpdatePortList();
};
#endif // MAINWINDOW_H
