#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include <QObject>

class UltrasonicSensor : public QObject {
    Q_OBJECT
public:
    explicit UltrasonicSensor(int triggerPin, int echoPin, QObject *parent = nullptr);
    ~UltrasonicSensor();

signals:
    void distanceMeasured(double distance);

public slots:
    void measureDistance();

private:
    int triggerPin;
    int echoPin;
    void sendPulse();
    double getDistance(); // Devuelve la distancia en centímetros.
};

#endif // ULTRASONICSENSOR_H
