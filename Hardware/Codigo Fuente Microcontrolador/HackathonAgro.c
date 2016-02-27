
/*
 ***********************************************************************
 *                      LIBRERIAS y CONSTANTES
 * *********************************************************************
 */

/****** INICIO MODULO RELOJ ******/

#include <virtuabotixRTC.h> //
// Creation of the Real Time Clock Object
virtuabotixRTC myRTC(21, 20, 19); // CLK,DAT,RST -- SCLK -> 21, I/O -> 20, CE -> 19

/****** FIN MODULO RELOJ ******/

/***** INICIO LCD ******/

#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 10, 9, 8, 7); // LiquidCrystal(rs, enable, d4, d5, d6, d7)

/***** FIN LCD ******/

/***** INICIO MODULO GPS ******/
#include <SoftwareSerial.h>
SoftwareSerial mySerial(13, 31); // RX, TX

#include "TinyGPS.h"
TinyGPS gps;

#define GPS_TX_DIGITAL_OUT_PIN 13
#define GPS_RX_DIGITAL_OUT_PIN 31

#define MAX_BUFFER 500

unsigned long momento_anterior = 0;
unsigned long bytes_recibidos = 0;

long startMillis;
long secondsToFirstLocation = 0;

float latitude = 0.0;
float longitude = 0.0;

char latit[12];
char longi[12];
/***** FIN MODULO GPS ******/

/****** INICIO HUMEDAD Y TEMPERATURA AMBIENTE ******/

#include "DHT.h"
#define DHTPIN 6
#define DHTTYPE DHT22 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);
float h;
float t;

/****** FIN HUMEDAD Y TEMPERATURA AMBIENTE ******/

/******* INICIO TEMPERATURA DEL SUELO **********/
int temp_pin = A7;
float TempC;
/******* FIN TEMPERATURA DEL SUELO **********/

/******* INICIO HUMEDAD DEL SUELO **********/
float HS;
/******* FIN HUMEDAD DEL SUELO **********/

/****** INICIO UV ******/
int uvLevel;
int refLevel;
float outputVoltage;
volatile float uvIntensity;

// Hardware pin definitions MP8511
int UVOUT = A0; // Output from the sensor
int REF_3V3 = A1; // 3.3V power on the Arduino board

/****** FIN UV ******/

/****** INICIO GASES ******/
float CO, COV;
/****** FIN GASES ******/

/****** INICIO MP ******/
float MP;
int measurePin = A5;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
/****** FIN MP ******/

/****** INICIO CONTROL DE LECTURAS ******/
unsigned long time;
int alertaUV;
/****** FIN CONTROL DE LECTURAS ******/

/****** INICIO CONTADOR SET POINT *****/
const int boton = 2; // Interrupcion 0
volatile int contador = 0;
int lastInt = millis();
/****** FIN CONTADOR SET POINT *****/

/****** INICIO MODULO SD ******/

// #include <SD.h>
// File myFile;
/****** FIN MODULO SD ******/

/**
 * FUNCION DE CONFIGURACION INICIAL
 */
void setup()
{

    // Wire.begin();
    // RTC.begin();
    // Si quitamos el comentario de la linea siguiente, se ajusta la hora
    // y la fecha con la del ordenador
    // RTC.adjust(DateTime(__DATE__, __TIME__));
    myRTC.setDS1302Time(00, 25, 4, 6, 27, 2, 2016); // seg, min, hora,
    // dia de la
    // semana, dia del
    // mes, mes, año

    Serial.begin(9600);
    Serial.println("Iniciando..");
    mySerial.begin(9600);
    // pinMode(53, OUTPUT);

    dht.begin();

    pinMode(GPS_TX_DIGITAL_OUT_PIN, INPUT);
    pinMode(GPS_RX_DIGITAL_OUT_PIN, INPUT);

    lcd.begin(16, 2);

    // Antirebote para pulsador por software
    attachInterrupt(0, SetPoint, FALLING);

    // /////////////////////////////////////////////////////

    // Serial.print("Initializing SD card...");
    //
    // pinMode(53, OUTPUT);
    //
    // if (!SD.begin(53)) {
    // Serial.println("initialization failed!");
    // return;
    // }
    // Serial.println("initialization done.");
}


/**
 * FUNCION DE BUCLE PRINCIPAL
 */
void loop()
{
    // Start printing elements as individuals
    myRTC.updateTime();
    Serial.print("Current Date / Time: ");
    Serial.print(myRTC.dayofmonth);
    Serial.print("/");
    Serial.print(myRTC.month);
    Serial.print("/");
    Serial.print(myRTC.year);
    Serial.print(" ");
    Serial.print(myRTC.hours);
    Serial.print(":");
    Serial.print(myRTC.minutes);
    Serial.print(":");
    Serial.println(myRTC.seconds);
    // Delay so the program doesn't print non-stop
    delay(5000);

    /*
     * myFile = SD.open("test.txt", FILE_WRITE);
     * 
     * // if the file opened okay, write to it: if (myFile) {
     * Serial.print("Writing to test.txt...");
     * 
     * DateTime now = RTC.now(); myFile.print(now.day(), DEC);
     * myFile.print("/"); myFile.print(now.month(), DEC);
     * myFile.print("/"); myFile.print(now.year(), DEC); myFile.print("
     * "); myFile.print(now.hour(), DEC); myFile.print(":");
     * myFile.print(now.minute(), DEC); myFile.print(":");
     * myFile.print(now.second(), DEC); myFile.print(" ");
     * myFile.print("Sensores: "); myFile.print(h); myFile.print(",");
     * myFile.print(t); myFile.print(","); myFile.print(uvindex);
     * myFile.print(","); myFile.print(CO); myFile.print(",");
     * myFile.print(COV); myFile.print(","); myFile.print(MP);
     * myFile.print(","); myFile.print(latit); myFile.print(",");
     * myFile.println(longi); myFile.close(); Serial.println("Hecho.");
     * // close the file: myFile.close(); Serial.println("done."); } else
     * { // if the file didn't open, print an error: Serial.println("error 
     * opening test.txt"); }
     * 
     * // re-open the file for reading: myFile = SD.open("test.txt"); if
     * (myFile) { Serial.println("test.txt:");
     * 
     * // read from the file until there's nothing else in it: while
     * (myFile.available()) { Serial.write(myFile.read()); } // close the
     * file: myFile.close(); } else { // if the file didn't open, print an 
     * error: Serial.println("error opening test.txt"); }
     * 
     */

    GetGPS();
    GetHT();
    GetTs();
    GetHS();
    GetUV();
    GetFecha();

    Imprime_registros();
    Imprime_registros2();
    Imprime_registros3();
    Imprime_registros4();
}

/****** INICIO FUNCIONES ADICIONALES ******/

/**
 * Se utiliza para cargar los datos del modulo GPS y se guardan en las variables
 * globales latit y longi.
 */
void GetGPS()
{

    bool newData = false;
    unsigned long chars = 0;
    unsigned short sentences, failed;

    for (unsigned long start = millis(); millis() - start < 1000;) {
        while (mySerial.available()) {
            int c = mySerial.read();
            ++chars;
            if (gps.encode(c))
                newData = true;
        }
    }

    if (newData) {

        if (secondsToFirstLocation == 0) {
            secondsToFirstLocation = (millis() - startMillis) / 1000;
        }

        unsigned long age;
        gps.f_get_position(&latitude, &longitude, &age);

        Serial.print("Location: ");
        Serial.print(latitude, 6);
        Serial.print(" , ");
        Serial.print(longitude, 6);
        Serial.println("");
    }

    dtostrf(latitude, 6, 6, latit);
    dtostrf(longitude, 6, 6, longi);
    Serial.println(latit);
    Serial.println(longi);
}

/**
 * Se obtiene la humedad relativa (aire) y se guarda en las variables globales h y t.
 */
void GetHT()
{
    h = dht.readHumidity();
    t = dht.readTemperature();
    // Serial.print("Humidity: ");
    // Serial.print(h);
    // Serial.print("% ");
}

/**
 * Se obtiene la temperatura por medio de la lectura analoga del pin temp_pin
 */
void GetTs()
{
    TempC = analogRead(temp_pin);
    TempC = (5.0 * TempC * 100.0) / 1024.0;
    // Serial.print(temperatura);
    // Serial.println(" oC");
    // delay(1000);
}

/**
 * Se optiene la humedad del suelo por medio del PIN analogo A2.
 */
void GetHS()
{
    int val2 = analogRead(A2);
    HS = val2 * (5.0 / 1023.0);
    // Serial.print("Humedad Suelo:");
    // delay(1000);
    // Serial.print(HS);
    // delay(1000);
    // Serial.print("\n ");
}

/**
 * Se obtiene la radiacion ultravioleta
 */
void GetUV()
{
    uvLevel = averageAnalogRead(UVOUT);
    refLevel = averageAnalogRead(REF_3V3);

    // Use the 3.3V power pin as a reference to get a very accurate output
    // value from sensor
    outputVoltage = 3.3 / refLevel * uvLevel;
    uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0); // (mW/cm^2)

    // Serial.print("Nivel MP8511:");
    // Serial.print(uvLevel);
    // delay(1000);
    // Serial.print("\n ");
    // Serial.print(" Voltaje MP8511 : ");
    // Serial.print(outputVoltage);
    // delay(1000);
    // Serial.print("\n ");
    // Serial.print("Intensidad (mW/cm^2): ");
    // Serial.print(uvIntensity);
    // delay(500);
    // Serial.print("\n ");
    // delay(500);
}

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
/********************Arreglo Set Point ****************/
void SetPoint()
{
    if ((millis() - lastInt) > 500) {
        contador++;
        lastInt = millis();
    }
}

/*
     * void GetSD() {
     * 
     * 
     * myFile = SD.open("test.txt", FILE_WRITE);
     * 
     * // if the file opened okay, write to it: if (myFile) {
     * 
     * DateTime now = RTC.now(); Serial.print("Writing to test.txt...");
     * 
     * myFile.print("Sensores: "); myFile.print(h); myFile.print(",");
     * myFile.print(t); myFile.print(","); myFile.print(uvindex);
     * myFile.print(","); myFile.print(CO); myFile.print(",");
     * myFile.print(COV); myFile.print(","); myFile.print(MP);
     * myFile.print(","); myFile.print(latit); myFile.print(",");
     * myFile.println(longi); myFile.close(); Serial.println("Hecho.");
     * // close the file: myFile.close(); Serial.println("done."); } else
     * { // if the file didn't open, print an error: Serial.println("error 
     * opening test.txt"); }
     * 
     * // re-open the file for reading: myFile = SD.open("test.txt"); if
     * (myFile) { Serial.println("test.txt:");
     * 
     * // read from the file until there's nothing else in it: while
     * (myFile.available()) { Serial.write(myFile.read()); } // close the
     * file: myFile.close(); } else { // if the file didn't open, print an 
     * error: Serial.println("error opening test.txt"); }
     * 
     * 
     * 
     * 
     * } 
     */

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
void Imprime_registros()
{

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("H:");
    lcd.print(h);
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(t);
    lcd.setCursor(9, 0);
    lcd.print("Hs:");
    lcd.print(HS);
    lcd.setCursor(9, 1);
    lcd.print("Ts:");
    lcd.print(TempC);
    delay(3000);
}

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
void Imprime_registros2()
{

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("UV level:");
    lcd.print(uvLevel);
    lcd.setCursor(0, 1);
    lcd.print("UV Inten:");
    lcd.print(uvIntensity);
    delay(3000);
}

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
void Imprime_registros3()
{

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("La:");
    lcd.print(latit);
    lcd.setCursor(0, 1);
    lcd.print("Lo:");
    lcd.print(longi);
    delay(3000);
}

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
void Imprime_registros4()
{

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Point:");
    lcd.print(contador);
    delay(3000);
}

/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
void GetFecha()
{
    // DateTime now = RTC.now();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("FECHA:");
    lcd.print(myRTC.dayofmonth);
    lcd.print("/");
    lcd.print(myRTC.month);
    lcd.print("/");
    lcd.print(myRTC.year);
    lcd.setCursor(0, 1);
    lcd.print("HORA:");
    lcd.print(myRTC.hours);
    lcd.print(":");
    lcd.print(myRTC.minutes);
    lcd.print(":");
    lcd.print(myRTC.seconds);
    delay(2000);
}


/*************Funcion para promedio de muestreo******************/
/**
 * example of basic @param usage
 * @param bool $baz 
 * @return mixed 
 */
int averageAnalogRead(int pinToRead)
{
    byte numberOfReadings = 8;
    unsigned int runningValue = 0;

    for (int x = 0; x < numberOfReadings; x++)
        runningValue += analogRead(pinToRead);
    runningValue /= numberOfReadings;

    return (runningValue);
}

/************Arreglo para mapeo de punto flotante UVSensor*********/
/**
 * el param
 * @param float x es alguna cosa
 * @return float  
 */
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/****** FIN FUNCIONES ADICIONALES ******/