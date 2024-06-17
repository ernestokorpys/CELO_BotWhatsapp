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
const int adcPin = 34;
const int led1=12;
const int led2=14;
// Variable para almacenar el valor DAC1
int dacValue1;
float tension_dac=2.5;

// Variable para almacenar el valor ADC leído
int adcValue = 0;
// Variable para almacenar el estado del pin digital
int inputState = 0;
int estadoActual=LOW;
bool antispam=false;

void setup() {
  // Inicializa la comunicación serie
  Serial.begin(115200);
  float aux = (tension_dac/3.3)*255;
  int dacValue1 = aux;
  // Configura el pin GPIO26 como salida DAC con un valor fijo
  dacWrite(dacPin1, dacValue1);
  // Configura el pin 22 como entrada digital
  pinMode(inputPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  // Intenta conectar al WiFi
  conectarWiFi();
}

void loop() {
  // Leer el estado del pin digital (GPIO22)
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  int estadoActual = digitalRead(inputPin);
  if (estadoActual == LOW && antispam==false) {
    Serial.println("Hay Corte de Luz");
    digitalWrite(led1, HIGH);
    antispam=true;
    delay(5000);
    message_to_whatsapp("Hay un corte de energía");
  }
  if(estadoActual == HIGH && antispam==false){
    Serial.println("Hay energía");
    digitalWrite(led1, LOW);
    delay(60000); //delay(180000); //3minutos
  }

  if(antispam==true){
    delay(60000); //Delay 5minutos
     int estadoActual = digitalRead(inputPin);
     if (estadoActual == LOW) {
       Serial.println("Aun no ha vuelto la energía eléctrica.");
       //espera otros N minutos.
    }
    if (estadoActual == HIGH) {
      antispam=false;
      message_to_whatsapp("Energia reestablecida");
      Serial.println("Energía electrica reestablecida.");   
    }//Termina if
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

void  message_to_whatsapp(String message)       // es nuestra función definida por el usuario que enviará mensajes a WhatsApp messenger.
{
  //agregando todos los números, su clave api, su mensaje en una url completa
  url = "https://api.callmebot.com/whatsapp.php?phone=" + phone_number + "&apikey=" + apiKey + "&text=" + urlencode(message);

  postData(); // llamando a postData para ejecutar la URL generada anteriormente una vez para que reciba un mensaje.
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
