#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc; 
 
#define LED_BUILTIN 16
#define SCL 5
#define SDA 4
/* 
const int addrGetTemporizador=10;
const int addrGetTiempo1=20;
const int addrGetTiempo2=30;
const int addrGetTiempo3=40; */

String detoner;

DateTime fecha;
String tiempo;

String getTemporizador="";
String gettiempo1="";
String gettiempo2="";
String gettiempo3="";

String tiempo1Alto;
String tiempo2Alto;
String tiempo3Alto;

String getON="";
String getOFF="";

String pagina = "<html>"
"<head>"
"    <meta charset='UTF-8' content='text/html'>"
"    <title>Config</title>"
"<style>"
"    input{"
"        margin: 10px;"
"    }"
"    *{padding-left:10px;}"
"</style>"
"</head>"
"<body>"
"    <h1>Smart Watering</h1>"
"    <form action='config' method='GET' target='respuesta'> "
"        <h3>Selector automatico</h3>"
"            <input type='submit' name='ON' value='Encendido'></input>   "
"            <input type='submit' name='OFF' value='Apagado'></input>"
"        <br>"
"        <h3>Inserta los tiempos para riego</h3>"
"            Tiempo de riego: <input type='number' name='temporizador' min='1' max='60'><br>"
"            Tiempo 1: <input type='time' name='tiempo1' placeholder='Inserta el tiempo 1'><label for='tiempo1'></label><br>"
"            Tiempo 2: <input type='time' name='tiempo2' placeholder='Inserta el tiempo 2'><label for='tiempo2'></label><br>"
"            Tiempo 3: <input type='time' name='tiempo3' placeholder='Inserta el tiempo 3'><label for='tiempo3'></label><br>"
"            <input type='submit' value='Guardar'></input>"
"    </form>"
"    <form>"
"        <iframe id='respuesta' name='respuesta' src='' frameborder='0'></iframe>"
"    </form> "
"</body>"
"</html>";

ESP8266WebServer server(80);

//*******  G R A B A R  EN LA  E E P R O M  ***********
void graba(int addr, String a) {
  int tamano = (a.length() + 1);
  Serial.print(tamano);
  char inchar[30];    //'30' Tama�o maximo del string
  a.toCharArray(inchar, tamano);
  EEPROM.write(addr, tamano);
  for (int i = 0; i < tamano; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
  EEPROM.commit();
}

//*******  L E E R   EN LA  E E P R O M    **************
String lee(int addr) {
  String nuevoString;
  int valor;
  int tamano = EEPROM.read(addr);
  for (int i = 0; i < tamano; i++) {
    addr++;
    valor = EEPROM.read(addr);
    nuevoString += (char)valor;
  }
  return nuevoString;
}


String arregla_simbolo(String a){
    a.replace("%3A", ":");
    return a;
}

void recibirDatos(){

    getON=server.arg("ON");
    getOFF=server.arg("OFF");
    getTemporizador=server.arg("temporizador");
    gettiempo1=server.arg("tiempo1");
    gettiempo2=server.arg("tiempo2");
    gettiempo3=server.arg("tiempo3");

    if(getON=="Encendido"){digitalWrite(LED_BUILTIN,LOW); }
    if(getOFF=="Apagado"){digitalWrite(LED_BUILTIN,HIGH); }

    gettiempo1=arregla_simbolo(gettiempo1);
    gettiempo2=arregla_simbolo(gettiempo2);
    gettiempo3=arregla_simbolo(gettiempo3);

    graba(10, getTemporizador);
    graba(40, gettiempo1);
    graba(70, gettiempo2);
    graba(100, gettiempo3);

    server.send(200, "text/html", String(
                                        "<br>" + fecha.timestamp(DateTime::TIMESTAMP_DATE) + " " + fecha.timestamp(DateTime::TIMESTAMP_TIME) +
                                        "<br>Riego: " + getON + getOFF + 
                                        "<br>Tiempo de riego: " + getTemporizador + " m"
                                        "<br>Tiempo 1: " + gettiempo1 + "-" + calculaFin(gettiempo1, getTemporizador) +
                                        "<br>Tiempo 2: " + gettiempo2 + "-" + calculaFin(gettiempo2, getTemporizador) +
                                        "<br>tiempo 3: " + gettiempo3 + "-" + calculaFin(gettiempo3, getTemporizador) ));

}

String calculaFin(String hora, String temporizador){
  int auxhora=0;
  int auxmin=0;
  String fin;
  String h;
  String m;
    //Serial.print(hora + " " + temporizador + " ");
    auxmin= (hora.substring(3,5)).toInt() + temporizador.toInt();
    if(auxmin>59){
      auxhora=(hora.substring(0,3)).toInt() + 1;
      auxmin=auxmin%60;
    }else{
      auxhora=(hora.substring(0,3)).toInt();
    }
    //Serial.print(auxhora);Serial.print(":");Serial.println(auxmin);
    if(String(auxhora).length()<2){ h="0" + String(auxhora);}else{h=String(auxhora);}
    if(String(auxmin).length()<2){ m="0" + String(auxmin);}else{m=String(auxmin);}   
    fin=h + ":" + m;
  return fin;
}

void setup(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    EEPROM.begin(4096);
    WiFi.softAP("ESP8266Watering");
    server.on("/", []() {
        server.send(200, "text/html", pagina);
    });
    server.on("/config", recibirDatos);
    server.begin();
    //Serial.println("Servidor Iniciado");
    Wire.begin(SDA, SCL);
    if(!rtc.begin()){
      Serial.println("Modulo RTC no encontrado");
      while(1);
    }
    rtc.adjust(DateTime(__DATE__, __TIME__));
    getTemporizador=lee(10);
    gettiempo1=lee(40);
    gettiempo2=lee(70);
    gettiempo3=lee(100);

    Serial.println(getTemporizador); 
    Serial.println(gettiempo1); 
    Serial.println(gettiempo2); 
    Serial.println(gettiempo3); 

}

void loop(){
    server.handleClient();  
    fecha=rtc.now();
    Serial.println(String(fecha.timestamp(DateTime::TIMESTAMP_DATE) + " " + fecha.timestamp(DateTime::TIMESTAMP_TIME)));
    delay(2000);

    detoner=(fecha.timestamp(DateTime::TIMESTAMP_TIME)).substring(0,5);
    Serial.println("Detonador: " + detoner);
   
    tiempo1Alto = calculaFin(gettiempo1, getTemporizador);
    tiempo2Alto = calculaFin(gettiempo2, getTemporizador);
    tiempo3Alto = calculaFin(gettiempo2, getTemporizador);
    
    //Serial.println("----------------------------");
    Serial.print("Tiempo 1: "); Serial.println(gettiempo1 + " " + getTemporizador + " " + calculaFin(gettiempo1, getTemporizador));
    Serial.print("Tiempo 2: "); Serial.println(gettiempo2 + " " + getTemporizador + " " + calculaFin(gettiempo2, getTemporizador));
    Serial.print("Tiempo 3: "); Serial.println(gettiempo3 + " " + getTemporizador + " " + calculaFin(gettiempo3, getTemporizador));
  
    if((detoner==gettiempo1)||(detoner==gettiempo2)||(detoner==gettiempo3)){
        digitalWrite(LED_BUILTIN,LOW); 
        Serial.println("==========ENCENDIDO===========");
    }
    if((detoner==tiempo1Alto)||(detoner==tiempo2Alto)||(detoner==tiempo3Alto)){
        digitalWrite(LED_BUILTIN,HIGH);
        Serial.println("===========APAGADO============");
    }
    Serial.println("==============================");
} 
