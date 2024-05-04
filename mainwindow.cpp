#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <QTime>
#include <QTimer>
#include <qmessagebox.h>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QString>
#include <QByteArray>
//#include <QSerialPortInfo>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ringTx.buf=TX;
    ringRx.buf=RX;
    ringTx.iW=0;
    ringTx.iR=0;
    ringRx.iW=0;
    ringRx.iR=0;
    ringRx.header=0;

    timerUSB = new QTimer(this);
    connect(timerUSB, &QTimer::timeout, this, &MainWindow::verificarYConectarUSB);
    timerUSB->start(1000); // Intervalo de tiempo en milisegundos

    serial = new QSerialPort(this);
    /*//serial->setPortName("COM4"); // Ajusta el nombre del puerto a tu puerto correcto.
        serial->setPortName("COM12");
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->open(QSerialPort::ReadWrite);
        serial->setDataTerminalReady(true);
        connect(serial, &QSerialPort::readyRead, this, &MainWindow::OnQSerialPort1Rx);*/
    conectarMicro();

    // inicio();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::conectarMicro(){
    // Enumerar los puertos disponibles y buscar tu dispositivo específico
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (((info.description().contains("USB Serial Device")) || (info.description().contains("Dispositivo serie USB"))) && (info.serialNumber() == "")) {
            serial->setPort(info);
            serial->setBaudRate(QSerialPort::Baud115200); // Configura según tu dispositivo
            // Configura otros parámetros si es necesario

            if (serial->open(QSerialPort::ReadWrite)) {
                serial->setDataTerminalReady(true);
                qDebug() << "Conectado con éxito a" << info.portName();
                                                       break; // Salir del bucle una vez conectado
            } else {
                qDebug() << "Error al abrir el puerto:" << serial->errorString();
            }
        }
    }
}

void MainWindow::verificarYConectarUSB() {

    if (serial->isOpen()) {
        // El dispositivo ya está conectado, no es necesario hacer nada.
        qDebug() << "Dispositivo ya conectado.";
    } else {
        // Intentar reconectar
        qDebug() << "Dispositivo desconectado. Intentando reconectar...";
        conectarMicro();
    }
    if (!serial->isOpen() || serial->error() == QSerialPort::ResourceError) {
        qDebug() << "Problema detectado en la conexión. Intentando reconectar...";
                    serial->close(); // Cierra el puerto si está abierto
        conectarMicro(); // Intenta reconectar
    }
}

void MainWindow::OnQSerialPort1Rx(){

    QByteArray data = serial->readAll();
    if (data.isEmpty()) {
        return;
    }

    QString strhex;
    for (int i = 0; i < data.size(); i++)
        strhex = strhex + QString("%1").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();

    //strData = QString::fromLatin1(data);

    for (int i = 0; i < data.size(); i++)
    {
        ringRx.buf[ringRx.iW] = data[i];
        ringRx.iW++;
        //Decode();
    }

    //ui->lineEdit->setText(strhex);
}

//void MainWindow::inicio(){

//    crearArrayCMD(POSITION_MODE,ID_M_DIREC);
//    EnviarComando(0x0B, POSITION_MODE, payloadCAN);

//    crearArrayCMD(VELOCITY_MODE,ID_M_VEL);
//    EnviarComando(0x0B, VELOCITY_MODE, payloadCAN);
//}

void MainWindow::Decode(){
    if(ringRx.iW == ringRx.iR)
        return;

    switch (ringRx.header)
    {
    case 0:
        if (ringRx.buf[ringRx.iR] == 'U')
            ringRx.header++;
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;
    case 1:
        if (ringRx.buf[ringRx.iR] == 'N')
            ringRx.header++;
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;
    case 2:
        if (ringRx.buf[ringRx.iR] == 'E')
            ringRx.header++;
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;
    case 3:
        if (ringRx.buf[ringRx.iR] == 'R'){
            ringRx.header++;

        }
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;
    case 4:
        ringRx.nBytes = ringRx.buf[ringRx.iR];
        ringRx.header++;
        break;
    case 5:
        if (ringRx.buf[ringRx.iR] == 0x00)
            ringRx.header++;
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;
    case 6:
        if (ringRx.buf[ringRx.iR] == ':')
        {
            ringRx.cks= 'U'^'N'^'E'^'R'^ringRx.nBytes^0x00^':';
            ringRx.header++;
            ringRx.iData = ringRx.iR+1;
            //LED_RED_TOGGLE();
        }
        else{
            ringRx.header = 0;
            //ringRx.iR--;
        }
        break;

    case 7:
        UpdateChecksum();
        CheckBytesLeft();
        if(ringRx.nBytes == 0)
        {
            CheckChecksumAndReceiveData();
        }
        break;
    default:
        ringRx.header = 0;
        break;
    }
    ringRx.iR++;
}

void MainWindow::UpdateChecksum()
{
    if(ringRx.nBytes > 1)
    {
        ringRx.cks ^= ringRx.buf[ringRx.iR];
    }
}

void MainWindow::CheckBytesLeft()
{
    ringRx.nBytes--;
    if(ringRx.nBytes == 0)
    {
        ringRx.header = 0;
    }
}

void MainWindow::CheckChecksumAndReceiveData()
{
    if(ringRx.cks == ringRx.buf[ringRx.iR])
    {
        RecibirDatos(ringRx.iData);
    }
}

void MainWindow::RecibirDatos(uint8_t head){
    static uint8_t cont=0;
    cont++;
    switch (ringRx.buf[head++]){
        case MOTOR_DIR_DATA1_CMD:
        RealPositionDIR.i8[0] = ringRx.buf[head++];
        RealPositionDIR.i8[1] = ringRx.buf[head++];
        RealPositionDIR.i8[2] = ringRx.buf[head++];
        RealPositionDIR.i8[3] = ringRx.buf[head++];
        StatusWordDIR.u16[0] = ringRx.buf[head++];
        StatusWordDIR.u16[1] = ringRx.buf[head++];
        RealCurrentDIR.i16[0] = ringRx.buf[head++];
        RealCurrentDIR.i16[1] = ringRx.buf[head++];
        break;
        case MOTOR_DIR_DATA2_CMD:
        //ta vacio
        break;
        case MOTOR_SPEED_DATA1_CMD:
        RealSpeedVEL.i8[0] = ringRx.buf[head++];
        RealSpeedVEL.i8[1] = ringRx.buf[head++];
        RealSpeedVEL.i8[2] = ringRx.buf[head++];
        RealSpeedVEL.i8[3] = ringRx.buf[head++];
        StatusWordVEL.u16[0] = ringRx.buf[head++];
        StatusWordVEL.u16[1] = ringRx.buf[head++];
        RealCurrentVEL.i16[0] = ringRx.buf[head++];
        RealCurrentVEL.i16[1] = ringRx.buf[head++];
        break;
        case MOTOR_SPEED_DATA2_CMD:
        //ta vacio
        break;
        case FAULT_CMD:
        flagFaults.byte = ringRx.buf[head++];
        break;
    default:
        break;
    }
}

void MainWindow::crearArrayCMD(uint8_t cmd, uint8_t id){

    switch(cmd){ //la data se guarda en el mensaje poniendo el byte menos sig primero
    case 0xAF:
        payloadCAN[0] = id;
        payloadCAN[1] = 0x00;
        payloadCAN[2] = 0x00;
        payloadCAN[3] = 0x00;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = distance_sensor.u8[0];
        payloadCAN[6] = distance_sensor.u8[1];
        payloadCAN[7] = distance_sensor.u8[2];
        payloadCAN[8] = distance_sensor.u8[3];
        break;
    case 0xBF:

        payloadCAN[7] = 0;
        payloadCAN[8] = 0;
        payloadCAN[0] = id;
        payloadCAN[1] = 0x00;
        payloadCAN[2] = 0x00;
        payloadCAN[3] = 0x00;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x00;
        payloadCAN[6] = 0x00;
        payloadCAN[7] |= (uint8_t)ui->sens0->isChecked();//enviamos el estado del sensor 0 en el bit 0
        payloadCAN[7] |= ((uint8_t)ui->sens1->isChecked()<<1);//enviamos el estado del sensor 1 en el bit 1
        payloadCAN[7] |= ((uint8_t)ui->sens2->isChecked()<<2);//enviamos el estado del sensor 2 en el bit 2
        payloadCAN[7] |= ((uint8_t)ui->sens3->isChecked()<<3);//enviamos el estado del sensor 3 en el bit 3

        payloadCAN[8] |= (uint8_t)ui->sens4->isChecked();
        payloadCAN[8] |= ((uint8_t)ui->sens5->isChecked()<<1);
        payloadCAN[8] |= ((uint8_t)ui->sens6->isChecked()<<2);
        payloadCAN[8] |= ((uint8_t)ui->sens7->isChecked()<<3);
        break;
    case 0xCF:
        payloadCAN[0] = id;
        payloadCAN[1] = KP_SteeringMotor.u8[0];
        payloadCAN[2] = KP_SteeringMotor.u8[1];
        payloadCAN[3] = KP_SteeringMotor.u8[2];
        payloadCAN[4] = KP_SteeringMotor.u8[3];
        payloadCAN[5] = KD_SteeringMotor.u8[0];
        payloadCAN[6] = KD_SteeringMotor.u8[1];
        payloadCAN[7] = KD_SteeringMotor.u8[2];
        payloadCAN[8] = KD_SteeringMotor.u8[3];
        break;
    case VELOCITY_MODE: //coloca el motor en modo velocidad
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2F;
        payloadCAN[2] = 0x60;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x03;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case POSITION_MODE://coloca el motor en modo posicion
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2F;
        payloadCAN[2] = 0x60;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x01;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case ENABLE://envia la señal que habilita el motor
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2B;
        payloadCAN[2] = 0x40;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x0F;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case DISABLE://envia la señal que deshabilita el motor
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2B;
        payloadCAN[2] = 0x40;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x06;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case TARGET_SPEED://envia la velocidad a la que se desea que vaya el motor
        payloadCAN[0] = id;
        payloadCAN[1] = 0x23;
        payloadCAN[2] = 0xFF;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = velocidad_cmd.u8[0];
        payloadCAN[6] = velocidad_cmd.u8[1];
        payloadCAN[7] = velocidad_cmd.u8[2];
        payloadCAN[8] = velocidad_cmd.u8[3];
        break;
    case TARGET_POS://envia la posicion a la que se desea que se desplace el motor
        payloadCAN[0] = id;
        payloadCAN[1] = 0x23;
        payloadCAN[2] = 0x7A;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = pos_cmd.u8[0];
        payloadCAN[6] = pos_cmd.u8[1];
        payloadCAN[7] = pos_cmd.u8[2];
        payloadCAN[8] = pos_cmd.u8[3];
        break;
    case INVERTIR_1://invierte el giro del motor
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2F;
        payloadCAN[2] = 0x7E;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x01;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case INVERTIR_2://vuelve a invertir el giro
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2F;
        payloadCAN[2] = 0x7E;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x00;
        payloadCAN[6] = 0x00;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    case READY_POS://PONER MOTOR DIR EN ALWAYS READY, necesario para indicar que el motor esta listo para recibir posicion
        payloadCAN[0] = id;
        payloadCAN[1] = 0x2B;
        payloadCAN[2] = 0x40;
        payloadCAN[3] = 0x60;
        payloadCAN[4] = 0x00;
        payloadCAN[5] = 0x3F;
        payloadCAN[6] = 0x10;
        payloadCAN[7] = 0x00;
        payloadCAN[8] = 0x00;
        break;
    default:
        break;
    }

}
void MainWindow::EnviarComando(uint8_t length, uint8_t cmd, uint8_t payloadSEND[]){

    TX[0] = 'U';
    TX[1] = 'N';
    TX[2] = 'E';
    TX[3] = 'R';
    TX[4] = length;
    TX[5] = 0x00;
    TX[6] = ':';
    TX[7] = cmd;
    TX[8] = payloadSEND[0];
    TX[9] = payloadSEND[1];
    TX[10] = payloadSEND[2];
    TX[11] = payloadSEND[3];
    TX[12] = payloadSEND[4];
    TX[13] = payloadSEND[5];
    TX[14] = payloadSEND[6];
    TX[15] = payloadSEND[7];
    TX[16] = payloadSEND[8];


    // Calcular el checksum
    uint8_t cks = 0;
    cks=0;

    for(int i=0; i<TX[4]+6; i++){
        cks ^= TX[i];
    }
    TX[TX[4]+6] = cks;




    if(serial->isOpen()){
        serial->write((char*)TX, 7 + TX[4]);
    }
}


void MainWindow::on_EN_DIR_pressed()
{
    crearArrayCMD(ENABLE,ID_M_DIREC);
    EnviarComando(0x0B, ENABLE, payloadCAN);
}


void MainWindow::on_DIS_DIR_pressed()
{
    crearArrayCMD(DISABLE,ID_M_DIREC);
    EnviarComando(0x0B, DISABLE, payloadCAN);
}


void MainWindow::on_EN_VEL_pressed()
{
    crearArrayCMD(ENABLE,ID_M_VEL);
    EnviarComando(0x0B, ENABLE, payloadCAN);
}


void MainWindow::on_DIS_VEL_pressed()
{
    crearArrayCMD(DISABLE,ID_M_VEL);
    EnviarComando(0x0B, DISABLE, payloadCAN);
}



void MainWindow::on_B_500_RPM_pressed()// la velocidad se calcula con la ecuacion "DEC=[(rpm* 512 * Encoder_Resolution)/1875]"
{
    vel_aux = ((500 * 512) * (10000.0/1875));
    velocidad_cmd.u32 = (uint32_t)vel_aux;

    //QString strTest2 = QString("%1").arg(vel_aux, 0, 'f', 2);
    //QString strTest2 = QString("%1").arg(velocidad_cmd.u32, 8, 16, QChar('0')).toUpper();
    //ui->text_vel->setText(strTest2);

    crearArrayCMD(TARGET_SPEED,ID_M_VEL);
    EnviarComando(0x0B, TARGET_SPEED, payloadCAN);
}


void MainWindow::on_B_1000_RPM_pressed()
{
    vel_aux = ((1000 * 512) * (10000.0/1875));
    velocidad_cmd.u32 = (uint32_t)vel_aux;

    crearArrayCMD(TARGET_SPEED,ID_M_VEL);
    EnviarComando(0x0B, TARGET_SPEED, payloadCAN);
}


void MainWindow::on_B_1500_RPM_pressed()
{
    vel_aux = ((1500 * 512) * (10000.0/1875));
    velocidad_cmd.u32 = (uint32_t)vel_aux;

    crearArrayCMD(TARGET_SPEED,ID_M_VEL);
    EnviarComando(0x0B, TARGET_SPEED, payloadCAN);
}


void MainWindow::on_B_2000_RPM_pressed()
{
    vel_aux = ((2000 * 512) * (10000.0/1875));
    velocidad_cmd.u32 = (uint32_t)vel_aux;

    crearArrayCMD(TARGET_SPEED,ID_M_VEL);
    EnviarComando(0x0B, TARGET_SPEED, payloadCAN);
}


void MainWindow::on_SLID_RPM_valueChanged(int RPM_slid)
{
    ui->spin_rpm->setValue(RPM_slid);
    //buscar forma de pasar el numero del slider a comando
    vel_aux = ((RPM_slid * 512) * (10000.0/1875));
    velocidad_cmd.u32 = (uint32_t)vel_aux;
    vel_slid = vel_aux;

    //QString strTest2 = QString("%1").arg(velocidad_cmd.u32, 8, 16, QChar('0')).toUpper();
    //ui->text_vel->setText(strTest2);
}


void MainWindow::on_POS_BUT_pressed()
{
    //tomar valor de lineEdit y pasarlo a comando
    pos_ing.f = (ui->LINE_POS->text()).toFloat();

    pos_aux = ((pos_ing.f/5.49316)*1000);

    pos_cmd.u32 = (uint32_t)pos_aux;

    //QString strTest = QString("%1").arg(pos_cmd.u32, 8, 16, QChar('0')).toUpper();

    //ui->text_pos->setText(strTest);

    crearArrayCMD(TARGET_POS,ID_M_DIREC);
    EnviarComando(0x0B, TARGET_POS, payloadCAN);
}


void MainWindow::on_vel_slid_bot_pressed()
{
    velocidad_cmd.u32 = (uint32_t)vel_slid;

    //QString strTest2 = QString("%1").arg(velocidad_cmd.u32, 8, 16, QChar('0')).toUpper();
    //ui->text_vel->setText(strTest2);
    crearArrayCMD(TARGET_SPEED,ID_M_VEL);
    EnviarComando(0x0B, TARGET_SPEED, payloadCAN);
}


void MainWindow::on_but_CMD_pressed()
{
    QString Function_Str = ui->line_function->text();

    QString Index_Str = ui->line_Index->text();

    QString SubIndex_Str = ui->line_subIndex->text();

    QString Data_Str = ui->line_Data->text();

    QString Send_Str;

    //ui->line->setText(Index_Str + SubIndex_Str);

    switch(ui->box_ID->currentIndex()){
    case 0:
        //ui->line->setText("01 " + Index_Str + SubIndex_Str);
        Send_Str = ("01 "+Function_Str+Index_Str+SubIndex_Str+Data_Str);
        //ui->line->setText(Send_Str);
        break;
    case 1:
        //ui->line->setText("07 " + Index_Str + SubIndex_Str);
        Send_Str = ("07 "+Function_Str+Index_Str+SubIndex_Str+Data_Str);
        //ui->line->setText(Send_Str);
        break;
    }

    // Divide la cadena usando el espacio como delimitador
    QStringList hexStrings = Send_Str.split(" ");
    // Asegúrate de que el array tenga el tamaño adecuado
    // Asumiendo que sabes el tamaño de antemano. Ajusta según sea necesario.

    qDebug() << "Tamaño:" << (hexStrings.size());
        // Convertir cada componente y asignarlo al array
        for(int i = 0; i < hexStrings.size(); ++i) {
        bool ok;
        uint8_t convertedValue = (uint8_t)(hexStrings.at(i).toUInt(&ok, 16));

        // Imprimir el valor convertido para verificar
        //qDebug() << "Original:" << hexStrings.at(i) << "Convertido:" << payloadCANs[i];

        if(ok) {
            payloadCANs[i] = convertedValue;
            //qDebug().nospace() << "Original: "" << hexStrings.at(i) << "" Convertido: 0x" << hex << payloadCANs[i];
            //qDebug().resetFormat();  // Resetear el formato para futuras impresiones
        } else {
            qDebug() << "Error al convertir:" << hexStrings.at(i);
        }
    }

    QString strTest2;
    for(int b = 0; b < 9;b++)
        strTest2 = strTest2 + QString("%1").arg(payloadCANs[b], 2, 16, QChar('0')).toUpper();
    //ui->line2->setText(strTest2);
    EnviarComando(0x0B, 0xD7, payloadCANs);
}



void MainWindow::on_bot_INV_pressed()
{
    if(!INV_1){
        crearArrayCMD(INVERTIR_1,ID_M_VEL);
        EnviarComando(0x0B, INVERTIR_1, payloadCAN);
        INV_1 = 1;
    }else{
        crearArrayCMD(INVERTIR_2,ID_M_VEL);
        EnviarComando(0x0B, INVERTIR_2, payloadCAN);
        INV_1 = 0;
    }
}


void MainWindow::on_SLID_distance_valueChanged(int DISTANCE_slid)
{
    if(Dist_enable == 1){
        ui->spin_distance->setValue(DISTANCE_slid);
        distance_sensor.f=(float)DISTANCE_slid+0.0;
        crearArrayCMD(0xAF,ID_M_VEL);
        EnviarComando(0x0B,0xAF,payloadCAN);
        //buscar forma de pasar el numero del slider a comando
    }
}


void MainWindow::on_send_sens_pressed()
{
    //    SensorData[6] = 0;
    //    SensorData[7] = 0;

    //    SensorData[6] |= (uint8_t)ui->sens0->isChecked();//enviamos el estado del sensor 0 en el bit 0
    //    SensorData[6] |= ((uint8_t)ui->sens1->isChecked()<<1);//enviamos el estado del sensor 1 en el bit 1
    //    SensorData[6] |= ((uint8_t)ui->sens2->isChecked()<<2);//enviamos el estado del sensor 2 en el bit 2
    //    SensorData[6] |= ((uint8_t)ui->sens3->isChecked()<<3);//enviamos el estado del sensor 3 en el bit 3

    //    SensorData[7] |= (uint8_t)ui->sens4->isChecked();
    //    SensorData[7] |= ((uint8_t)ui->sens5->isChecked()<<1);
    //    SensorData[7] |= ((uint8_t)ui->sens6->isChecked()<<2);
    //    SensorData[7] |= ((uint8_t)ui->sens7->isChecked()<<3);

    crearArrayCMD(0xBF,ID_M_DIREC);
    EnviarComando(0x0B,0xBF,payloadCAN);
}


void MainWindow::on_pushButton_PID_Steering_pressed()
{
    KP_SteeringMotor.u32 = ui->line_kp_steering->text().toUInt();
    KD_SteeringMotor.u32 = ui->line_kd_steering->text().toUInt();
    KI_SteeringMotor.u32 = ui->line_ki_steering->text().toUInt();

    crearArrayCMD(0xCF,ID_M_DIREC);
    EnviarComando(0x0B,0xCF,payloadCAN);


}

void MainWindow::on_back_but_CONTROL_pressed()
{
    ui->stackedWidget->setCurrentIndex(PRINCIPAL);
}


void MainWindow::on_but_go_CONTROL_pressed()
{
    ui->stackedWidget->setCurrentIndex(CONTROL);
}


void MainWindow::on_but_enable_dist_released()
{
    if(Dist_enable == 0){
        Dist_enable = 1;
        ui->label_dist->setText("Habilitada");
    }else{
        Dist_enable = 0;
        ui->label_dist->setText("Deshabilitada");
    }
}


void MainWindow::on_but_go_ESTACION_released()
{
    ui->stackedWidget->setCurrentIndex(SELECCION);
}


void MainWindow::on_but_go_USUARIOS_released()
{
    ui->stackedWidget->setCurrentIndex(LOGIN);
}


void MainWindow::on_but_go_DATOS_released()
{
    ui->stackedWidget->setCurrentIndex(DATOS);
}


void MainWindow::on_back_but_DATOS_released()
{
    ui->stackedWidget->setCurrentIndex(PRINCIPAL);
}


void MainWindow::on_back_but_LOGIN_released()
{
    ui->stackedWidget->setCurrentIndex(PRINCIPAL);
}


void MainWindow::on_back_but_ESTACION_released()
{
    ui->stackedWidget->setCurrentIndex(PRINCIPAL);
}

