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
#include "User.h"
#include "ultrasonicsensor.h"


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

typedef union{
    struct{
        uint8_t b0: 1;
        uint8_t b1: 1;
        uint8_t b2: 1;
        uint8_t b3: 1;
        uint8_t b4: 1;
        uint8_t b5: 1;
        uint8_t b6: 1;
        uint8_t b7: 1;
    }bit;
    uint8_t byte;
}_sFlag;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startMeasurement();

public slots: // Agrega esta sección
    void updateDistance(double distance); // Asegúrate de que acepta un parámetro double

private slots:

    //void inicio();

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

    void Decode();

    void RecibirDatos(uint8_t head);

    void on_but_go_ESTACION_released();

    void on_but_go_USUARIOS_released();

    void on_but_go_DATOS_released();

    void on_back_but_DATOS_released();

    void on_back_but_LOGIN_released();

    void on_back_but_ESTACION_released();

    void on_but_EST_1_released();

    void on_but_EST_2_released();

    void on_but_EST_3_released();

    void on_but_EST_4_released();

    void on_but_EST_5_released();

    void on_but_EST_6_released();

    void on_but_EST_7_released();

    void on_back_but_viaje_released();

    void on_back_but_viaje_2_released();

    void on_but_cambio_mode_released();


    void on_back_but_LLEGADA_released();

    void on_BUT_REG_released();

    void on_BUT_LOG_released();

    bool checkPermission(const QString &action);

    void but_ERROR_SENS_CORRIG();

    void on_back_but_ERROR_SENS_released();

    void on_back_but_CARGA_released();

    void on_but_GO_CARGA_released();

private:
    Ui::MainWindow *ui;

    QSerialPort *serial;
    QTimer *timerUSB,*ALIVEHMI;

    User *user1 = new User("user", "user");
    User *usergenerico = new User("user", "user");

    UltrasonicSensor *sensor;
    QThread* sensorThread;

    void OnQSerialPort1Rx();
    void conectarMicro();
    void verificarYConectarUSB();
    void AliveHMI();
    void crearArrayCMD(uint8_t cmd, uint8_t id);
    void EnviarComando(uint8_t length, uint8_t cmd, uint8_t payloadSEND[]);

    uint8_t TX[256], payloadCAN[256],RX[256],indiceRX_r=0,indiceRX_t=0, sensorDats[9];
    uint8_t payloadCANs[9], INV_1 = 0, Dist_enable = 0,  destino = 0, index_dist = 0, Sesion_iniciada = 0, carga_full = 0, cargador_conec = 2;

    float vel_aux = 0, vel_slid = 0, pos_aux=0, dist_aux=0;

    float Dist_prom[5];

    int velocidadReal = 0;

    _sWork pos_cmd, pos_ing, velocidad_cmd, distance_sensor,KP_SteeringMotor,KD_SteeringMotor,KI_SteeringMotor;
    _sWork RealSpeedVEL,StatusWordVEL,RealCurrentVEL; //Variables para almacenar datos enviados del motor velocidad por TPDO1
    _sWork RealPositionDIR,StatusWordDIR,RealCurrentDIR;//Variables para almacenar datos enviados del motor direccion por TPDO1
    _sWork RealDistance, voltaje_bat;

    volatile _sFlag flagFaults;

    volatile _rx ringRx;
    volatile _tx ringTx;
//ID de motores
#define ID_M_DIREC                          0x01
#define ID_M_VEL                            0x07
//Comandos
#define VELOCITY_MODE                       0xA3
#define POSITION_MODE                       0xA1
#define ENABLE                              0x01
#define DISABLE                             0x02
#define INVERTIR_1                          0x03
#define INVERTIR_2                          0x04
#define READY_POS                           0x05
#define TARGET_SPEED                        0x06
#define TARGET_POS                          0x07

    //Manual
#define MANUAL_CMD                          0xD7
#define CHANGE_MODE_CMD                     0xD8


#define DISTANCE_SENSOR_CMD                 0xA9

//Defines para pantallas
    const int PRINCIPAL = 1;
    const int CONTROL = 0;
    const int SELECCION = 2;
    const int VIAJANDO = 3;
    const int LLEGADA = 4;
    const int LOGIN = 5;
    const int DATOS = 6;
    const int EROR_LINEA = 7;
    const int CARGA = 8;

    //Defines para comandos para recibir datos
#define MOTOR_SPEED_DATA1_CMD               0xB0
#define MOTOR_SPEED_DATA2_CMD               0xB1
#define MOTOR_DIR_DATA1_CMD                 0xB2
#define MOTOR_DIR_DATA2_CMD                 0xB3

    //falla
#define FAULT_CMD                           0xF0

    //Defines de flags para fallas
#define GLOBAL_FAULT                        flagFaults.bit.b0
#define COMMUNICATION_FAULT                 flagFaults.bit.b1
#define sss                                 flagFaults.bit.b2
#define ssss                                flagFaults.bit.b3
#define sssss                               flagFaults.bit.b4
#define ssssss                              flagFaults.bit.b5
#define sssssss                             flagFaults.bit.b6
#define ssssssss                            flagFaults.bit.b7

    //defines de placa
#define ENABLE_MOTOR_CMD 						0x01 // COMANDO ENABLE MOTOR
#define DISABLE_MOTOR_CMD 						0x02 // COMANDO DISABLE MOTOR
#define INVERTIR_1_CMD 							0x03
#define INVERTIR_2_CMD 							0x04
#define READY_POSI_CMD 							0x05
#define SPEED_MOTOR_CMD 						0x06
#define POS_MOTOR_CMD 							0x07
#define POSITION_MODE_CMD 						0xA1
#define SPEED_MODE_CMD 							0xA3
//#define MANUAL_CMD 								0xA5
#define ACC_SPEED_CMD 							0xA6
#define DEC_SPEED_CMD 							0xA7
#define SPEED_POS_CMD 							0xA8
#define FAULT_CMD								0xF0
#define MOTOR_SPEED_DATA1_CMD					0xB0
#define MOTOR_SPEED_DATA2_CMD					0xB1
#define MOTOR_DIR_DATA1_CMD						0xB2
#define MOTOR_DIR_DATA2_CMD						0xB3
//#define FAULT_CMD								0xB4
#define PID_PARAMETERS_CMD						0xC0
#define MAG_SENSOR_SIM_CMD						0xC1
#define HMI_ALIVE_CMD							0xD2
#define DESTINATIONSTATION_CMD					0xD4
#define DESTINO_ALCANZADO_CMD                   0xD5
#define ORIGEN_ALCANZADO_CMD                    0xD6
#define OUT_OF_LINE_CMD                         0xD9
#define MODO_CARGA_CMD                          0xDA
#define CARGA_COMPLETA                          0xDB
#define CARGADOR_CON                            0xDC

};
#endif // MAINWINDOW_H
