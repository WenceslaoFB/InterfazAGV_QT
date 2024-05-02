#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct {
    uint8_t *buf;
    uint8_t iW;
    uint8_t iR;
    uint8_t header;
    uint16_t timeout;
    uint8_t nBytes;
    uint8_t iData;
    uint8_t cks;
}__attribute__((packed, aligned(1))) _rx;

typedef struct {
    uint8_t *buf;
    uint8_t iW;
    uint8_t iR;
    uint8_t cks;
}__attribute__((packed, aligned(1))) _tx;

typedef union{
    uint8_t u8[4];
    int8_t  i8[4];

    uint16_t u16[2];
    int16_t  i16[2];

    uint32_t u32;
    int32_t  i32;

    float    f;
}_sWork;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    void inicio();

    void on_EN_DIR_pressed();

    void on_DIS_DIR_pressed();

    void on_EN_VEL_pressed();

    void on_DIS_VEL_pressed();

    void on_B_500_RPM_pressed();

    void on_B_1000_RPM_pressed();

    void on_B_1500_RPM_pressed();

    void on_B_2000_RPM_pressed();

    void on_SLID_RPM_valueChanged(int RPM_slid);

    void on_POS_BUT_pressed();

    void on_vel_slid_bot_pressed();

    void on_but_CMD_pressed();

    void on_bot_INV_pressed();

    void on_SLID_distance_valueChanged(int value);

    void on_send_sens_pressed();

    void on_pushButton_PID_Steering_pressed();

    void on_back_but_CONTROL_pressed();

    void on_but_go_CONTROL_pressed();

    void on_but_enable_dist_released();

private:
    Ui::MainWindow *ui;

    QSerialPort *serial;
    QTimer *timerUSB;
    void OnQSerialPort1Rx();
    void conectarMicro();
    void verificarYConectarUSB();
    void crearArrayCMD(uint8_t cmd, uint8_t id);
    void EnviarComando(uint8_t length, uint8_t cmd, uint8_t payloadSEND[]);

    uint8_t TX[256], payloadCAN[256],RX[256],indiceRX_r=0,indiceRX_t=0, sensorDats[9];
    uint8_t payloadCANs[9], INV_1 = 0, Dist_enable = 0;
    float vel_aux = 0, vel_slid = 0, pos_aux=0;

    _sWork pos_cmd, pos_ing, velocidad_cmd, distance_sensor,KP_SteeringMotor,KD_SteeringMotor,KI_SteeringMotor;

    volatile _rx ringRx;
    volatile _tx ringTx;
//ID de motores
#define ID_M_DIREC 0x01
#define ID_M_VEL 0x07
//Comandos
#define VELOCITY_MODE 0xA3
#define POSITION_MODE 0xA1
#define ENABLE 0x01
#define DISABLE 0x02
#define INVERTIR_1 0x03
#define INVERTIR_2 0x04
#define READY_POS 0x05
#define TARGET_SPEED 0x06
#define TARGET_POS 0x07

    //Manual
#define MANUAL_CMD 0xD7

//Defines para pantallas
#define PRINCIPAL 1
#define CONTROL 0
#define SELECCION 2
#define VIAJANDO 3
#define LLEGADA 4
#define LOGIN 5
};
#endif // MAINWINDOW_H
