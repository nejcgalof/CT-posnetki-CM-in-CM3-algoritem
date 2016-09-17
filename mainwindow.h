#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_nalozi_sliko_triggered();

    void on_nalozi_sliko_triggered(bool checked);

    void on_nalozi_paleto_triggered();


    void on_actionCM_triggered();

    void on_actionCM_Dekodiraj_triggered();

    void CM_kodiraj();

    void CM3_kodiraj();

    void string_v_vector();

    void razlika_dveh_plasti();

    void on_actionCM3_triggered();

    void on_actionCM3_Dekodiraj_triggered();

    void obdelam_vektor(int N,string ime);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
