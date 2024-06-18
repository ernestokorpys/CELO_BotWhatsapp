#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"  // Incluye el archivo de configuración

//const char* ssid = "modem.vdsl";
//const char* password = "celo2018";
const char* ssid = "Telefonia";
const char* password = "telefonia";
//const char* ssid = "TIMBO_VIEJO";
//const char* password = "wilson261";

String url;

const int dacPin1 = 25;  // DAC1
const int inputPin = 22; // Pin digital para leer el estado (HIGH o LOW)
const int adcPin = 34;   // Pin adc para testeos
const int led1=12;       // Led de control 1
const int led2=14;       // Led de control 2

int dacValue1;            // Variable para almacenar el valor DAC1
float tension_dac=1.2;    // Valor de tension deseado

int adcValue = 0;         // Variable para almacenar el valor ADC leído
int inputState = 0;       // Variable para almacenar el estado del pin digital
int estadoActual=LOW;     // Estado del pin 22
bool antispam=false;      // Variable para evitar el espameo de Mensajes
int minutos_de_espera=3;  // Cada cuantos minutos se preguntara al puerto si hay corte de luz o no

void setup() {
  // Inicializa la comunicación serie
  Serial.begin(115200);
  float aux = (tension_dac/3.3)*255;
  int dacValue1 = aux;
  // Configura el pin GPIO26 como salida DAC con un valor fijo
  dacWrite(dacPin1, dacValue1);
  // Configura puertos entrada salida 
  pinMode(inputPin, INPUT); // pin 22 como entrada digital
  pinMode(led1, OUTPUT);    // pin 12 como salida digital
  pinMode(led2, OUTPUT);    // pin 14 como salida digital
  // Intenta conectar al WiFi
  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  int estadoActual = digitalRead(inputPin); // Leer el estado del pin digital (GPIO22)
  if (estadoActual == HIGH && antispam==false) {
    Serial.println("Hay Corte de Luz");
    digitalWrite(led1, HIGH);
    antispam=true;
    delay(5000);
    message_to_whatsapp("Fallo en el suministro de energía");
  }
  if(estadoActual == LOW && antispam==false){
    Serial.println("Hay energía");
    digitalWrite(led1, LOW);
    delay(60000*minutos_de_espera); 
  }

  if(antispam==true){
    delay(60000*minutos_de_espera);        //espera otros N minutos para ver el puerto.
     int estadoActual = digitalRead(inputPin);
     if (estadoActual == HIGH) {
       Serial.println("Aun no ha vuelto la energía eléctrica.");
    }
    if (estadoActual == LOW) {
      antispam=false;
      message_to_whatsapp("Energia reestablecida");
      Serial.println("Energía electrica reestablecida.");   
    }
  }
}

void conectarWiFi() {
  Serial.println("Conectando al WiFi...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) { // Intenta durante 10 segundos
    delay(500);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Se ha conectado al wifi con la ip: ");
    Serial.println(WiFi.localIP());
    digitalWrite(led2, LOW);
    delay(8000);

  } else {
    Serial.println("");
    Serial.println("No se pudo conectar al WiFi. Intentando nuevamente en el loop.");
    digitalWrite(led2, HIGH);
  }
}

void message_to_whatsapp(String message) {
  int numPhones = sizeof(phone_numbers) / sizeof(phone_numbers[0]);
  for (int i = 0; i < numPhones; i++) {
    // Generar la URL para cada número de teléfono con su correspondiente clave API
    url = "https://api.callmebot.com/whatsapp.php?phone=" + phone_numbers[i] + "&apikey=" + api_keys[i] + "&text=" + urlencode(message);

    postData(); // Llamando a postData para ejecutar la URL generada anteriormente una vez para que reciba un mensaje.
  }
}

void postData()     //postData es otra función definida por el usuario que se utiliza para enviar la URL mediante el método HTTP GET.
{
  int httpCode;     // variable utilizada para obtener el código http de respuesta después de llamar a api
  HTTPClient http;  // Declarar objeto de clase HTTPClient
  http.begin(url);  // comience el objeto HTTPClient con la URL generada
  httpCode = http.POST(url); // Finalmente, publique la URL con esta función y almacenará el código http
  if (httpCode == 200)      // Compruebe si el código http de respuesta es 200
  {
    Serial.println("MENSAJE ENVIADO CORRECTAMENTE"); // imprimir MENSAJE ENVIADO CORRECTAMENTE
  }
  else                      // si el código HTTP de respuesta no es 200, significa que hay algún error.
  {
    Serial.println("Error."); // Imprimir mensaje de error.
  }
  http.end();          // Después de llamar a la API, finalice el objeto de cliente HTTP.
}


String urlencode(String str)  // Función utilizada para codificar la url
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}
