#include <WiFi.h>
#include "BaseOTA.h"
#include <time.h>
#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifdef ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 10;  // 14 on a ESP32-C3 causes a boot loop.
#else  // ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 15;
#endif  // ARDUINO_ESP32C3_DEV

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC

const uint8_t kTimeout = 50;
#else   // DECODE_AC

const uint8_t kTimeout = 15;
#endif  // DECODE_AC
const uint16_t kMinUnknownSize = 12;

const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

#define LEGACY_TIMING_INFO false

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  
String irSignalString = "";

// Replace with your network credentials
const char* ssid = "GPON-c588"; 
const char* password = "378jb59a";

// Set web server port number to 80
WiFiServer server(80);
String header;
String output27State = "off";

const int output27 = 27;
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

const uint16_t kIrLed = 4;  
IRDaikinESP ac(kIrLed);  
  WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
int trenutniSati = 0;
int trenutneMinute = 0;
int trenutnoVreme = -1;
int danUNedelji, danNedeljaStampa = 0;
 int onTimer1, onTimer2, offTimer1, offTimer2, sati, minuti;
int prethodneMinute = -1;
int prethodneMinute2 = -1;
int IzabraniDan = 0;
int IzabranFan =-1;
bool ponovi = false;
String satiStr = ""; 
int bag1 =-1, bag2=-1,bag3=-1,bag4=-1;
String FixNull,FixNull2, FixNullV2,FixNull2V2;

  bool signalSent = false;
  bool daljinac = false;
  bool dugme = false;
  bool mojTajmer = false;
  bool onTajmer = false;
  bool offTajmer = false;
  bool PowerfullOn = false;
  bool QuietOn = false;
  bool SensorOn = false;
  bool MouldOn = false;
  bool ComfortOn = false;
  bool SwinghOn = false;
  bool SwingvOn = false;
  bool WeeklyOn = false;
   const int maxParametra = 10;
struct Parametri2 {
  int offSati;
  int offMinuti;
  int infoDan2;
  bool ponovi2;
};
Parametri2 parametri2[10];

   int tempZaStrutc = 0;
   String modeZaStrutc = "";
unsigned long lastExecutionTime = 0;
unsigned long executionInterval = 1000;

struct Parametri {
  String mode;
  int temp;
  int fan;
  int onSati;
  int onMinuti;
  int infoDan;
  bool ponovi;
  bool powerfullOn;
  bool quiet;
  bool sensor;
  bool mould;
  bool comfort;
  bool swingh;
  bool swingv;
  bool weekly;
};

Parametri parametri[10]; // Niz od 10 struktura Parametri
int brojacParametra = 0;
int brojacParametra2 = 0;
int obrisaniIndeksi[10]; // Niz za praćenje indeksa za brisanje
int brojacZaBrisanje = 0;


void ispisiParametre(const Parametri& parametar) {
  Serial.println("Mode: " + parametar.mode);
  Serial.println("Temperatura: " + String(parametar.temp));
  Serial.println("Fan: " + String(parametar.fan));
  Serial.println("On sati: " + String(parametar.onSati));
  Serial.println("On minuti: " + String(parametar.onMinuti));
   Serial.println("Izabran dan: " + String(parametar.infoDan));
  Serial.println(parametar.ponovi ? "true" : "false");
  Serial.println(parametar.powerfullOn ? "true" : "false");
  Serial.println(parametar.quiet ? "true" : "false");
  Serial.println(parametar.sensor ? "true" : "false");
  Serial.println(parametar.mould ? "true" : "false");
  Serial.println(parametar.comfort ? "true" : "false");
  Serial.println(parametar.swingh ? "true" : "false");
  Serial.println(parametar.swingv ? "true" : "false");
  Serial.println(parametar.weekly ? "true" : "false");
  Serial.println();
}

void ispisiParametre2(const Parametri2& parametar2) {

  Serial.println("Off sati: " + String(parametar2.offSati));
  Serial.println("Off minuti: " + String(parametar2.offMinuti));
     Serial.println("Izabran dan: " + String(parametar2.infoDan2));
  Serial.println(parametar2.ponovi2 ? "true" : "false");
  Serial.println();
}
int a = 0;
bool vecIspisano = false;
void mojTimer(const Parametri& parametar) {
  unsigned long currentMillis = millis();

 // Serial.println(parametar.onSati);
  for (int i = 0; i < 10; i++) {
    if ( currentMillis - lastExecutionTime >= executionInterval) {
      //Serial.println("Stiglo je u strukturu pa u funkciju");
      // Računanje trenutnog vremena i vremena za akciju za trenutni set parametara
      struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    for (int i = 0; i < 10; i++) {
       
      
      int trenutniSati = timeinfo.tm_hour;
      int trenutneMinute = timeinfo.tm_min;
      time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  if ( parametri[i].infoDan != 0 ){
 a= (timeinfo.tm_wday + 6) % 7 + 1;
  }

      if ( parametri[i].onSati == trenutniSati &&  parametri[i].onMinuti ==  trenutneMinute && vecIspisano == false && parametri[i].infoDan == a ) {
        // Vreme za akciju je stiglo, izvršite akciju
        prethodneMinute = trenutneMinute;
        bag1= parametri[i].onSati;
        bag2=parametri[i].onMinuti;
    
       Serial.println("Jednakooooo");

if( parametri[i].mode == "modeFan"){
            ac.setMode(kDaikinFan);
        }if (parametri[i].mode == "modeCool"){
              ac.setMode(kDaikinCool);
        } if (parametri[i].mode == "modeDry"){
              ac.setMode(kDaikinDry);
        } if (parametri[i].mode == "modeAuto"){
              ac.setMode(kDaikinAuto);
        } if (parametri[i].mode == "modeHeat"){
              ac.setMode(kDaikinHeat);
        }if (parametri[i].powerfullOn == true){
              ac.setPowerful(true);
        }
       
          if (parametri[i].quiet == true){
            ac.setQuiet(true);
        }
         
        
         if (parametri[i].sensor == true){
               ac.setSensor(true);
        }
        
         if (parametri[i].mould == true){
              ac.setMold(true);
        }
    
      
       if (parametri[i].comfort == true){
              ac.setComfort(true);
        }
      
       
          if (parametri[i].swingh == true){
              ac.setSwingHorizontal(true);
        }
       
       
        if (parametri[i].swingv == true){
             ac.setSwingVertical(true);
        }
    
        
        if (parametri[i].weekly == true){
             ac.setWeeklyTimerEnable(true);
        }
       
       ac.setTemp(parametri[i].temp);
          ac.setFan(parametri[i].fan);
           //   Prikaz onoga što ćete poslati
        Serial.println(ac.toString());
          
           ac.on();
            ac.send();

           ac.setPowerful(false);
             ac.setQuiet(false);
              ac.setSensor(false);
              ac.setMold(false);
              ac.setComfort(false);
               ac.setSwingHorizontal(false);
                ac.setSwingVertical(false);
                 ac.setWeeklyTimerEnable(false);
                 ac.setMode(kDaikinAuto);
                  ac.setTemp(25);





            IzabranFan =-1;
           tempZaStrutc = 0;
           modeZaStrutc = "";
            PowerfullOn = false;
            QuietOn = false;
            SensorOn = false;
            MouldOn = false;
            ComfortOn = false;
            SwinghOn = false;
            SwingvOn = false;
            WeeklyOn = false;
        if( parametri[i].ponovi == false){
       obrisiParametre(i);
        obrisaniIndeksi[brojacZaBrisanje] = i;
        brojacZaBrisanje++;
       
        }
         vecIspisano = true;
      }
      if (prethodneMinute != trenutneMinute ){
       vecIspisano = false;
        prethodneMinute = -1;
         bag1=-1;
         bag2=-1;
      }
    }
  }

  }
  }
}
int b=0;
bool vecIspisano2 =false;
void mojTimer2(const Parametri2& parametar2) {
  unsigned long currentMillis2 = millis();

 // Serial.println(parametar.onSati);
  for (int i = 0; i < 10; i++) {
    if ( currentMillis2 - lastExecutionTime >= executionInterval) {
      //Serial.println("Stiglo je u strukturu pa u funkciju");
      // Računanje trenutnog vremena i vremena za akciju za trenutni set parametara
      struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    for (int i = 0; i < 10; i++) {
       
      
      int trenutniSati2 = timeinfo.tm_hour;
      int trenutneMinute2 = timeinfo.tm_min;
      time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  if ( parametri2[i].infoDan2 != 0 ){
 b= (timeinfo.tm_wday + 6) % 7 + 1;
  }
     // Serial.println(  trenutniSati);
      // Serial.println( trenutneMinute);
      if ( parametri2[i].offSati == trenutniSati2 &&  parametri2[i].offMinuti ==  trenutneMinute2 && vecIspisano2 == false && parametri2[i].infoDan2 == b) {
        // Vreme za akciju je stiglo, izvršite akciju
        prethodneMinute2 = trenutneMinute2;
        bag3= parametri2[i].offSati;
        bag4=parametri2[i].offMinuti;
    
       Serial.println("Jednakooooo222222222222");
          
           ac.off();
            ac.send();

        if( parametri2[i].ponovi2 == false){
       obrisiParametre2(i);
        obrisaniIndeksi[brojacZaBrisanje] = i;
        brojacZaBrisanje++;}
       vecIspisano2 = true;
      }
      if (prethodneMinute2 != trenutneMinute2 ){
       vecIspisano2 = false;
        prethodneMinute2 = -1;
         bag3=-1;
         bag4=-1;
      }
    }
  }

  }
  }
}




void obrisiParametre(int index) {
  // Postavljanje parametara na nulu ili prazne vrednosti
  parametri[index].mode = "";
  parametri[index].temp = 0;
  parametri[index].fan = 0;
  parametri[index].onSati = -1;
  parametri[index].onMinuti = -1;
  parametri[index].infoDan = 0;
   parametri[index].ponovi = false;
  parametri[index].powerfullOn = false;
  parametri[index].quiet = false;
  parametri[index].sensor = false;
  parametri[index].mould = false;
  parametri[index].comfort = false;
  parametri[index].swingh = false;
  parametri[index].swingv = false;
  parametri[index].weekly = false;
   // Pomeranje preostalih indeksa unazad
  for (int i = index; i < brojacParametra - 1; i++) {
    parametri[i] = parametri[i + 1];
  }
  // Smanjenje brojača parametara
  brojacParametra--;
}

void obrisiParametre2(int index) {
 
  parametri2[index].offSati = -1;
  parametri2[index].offMinuti = -1;
  parametri2[index].infoDan2 = 0;
   parametri2[index].ponovi2 = false;
   // Pomeranje preostalih indeksa unazad
  for (int i = index; i < brojacParametra2 - 1; i++) {
    parametri2[i] = parametri2[i + 1];
  }
  // Smanjenje brojača parametara
  brojacParametra2--;
}


void setup() {
  
  OTAwifi();  // start default wifi (previously saved on the ESP) for OTA
#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif  // ESP8266
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  // Perform a low level sanity checks that the compiler performs bit field
  // packing as we expect and Endianness is as we expect.
  assert(irutils::lowLevelSanityCheck() == 0);

  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
  OTAinit();  // setup OTA handlers and show IP
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver
     ac.begin();
  Serial.begin(115200);
pinMode(output27, OUTPUT);
  // Set outputs to LOW
 
  digitalWrite(output27, LOW);
// Connecting to Wi-Fi network with SSID and password
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
// Print local IP address and start web server
Serial.println("");
Serial.println("WiFi connected.");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
server.begin();
 timeClient.begin();
  timeClient.setTimeOffset(3600);
configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
while (!time(nullptr)) {
    delay(1000);
    
  }
  time_t now;
struct tm timeinfo;

time(&now);
localtime_r(&now, &timeinfo);
Parametri noviParametri;
Parametri2 noviParametri2;
}


tmElements_t pomeriDatum(int brojDanaZaPomeraj) {
  timeClient.update();
  time_t trenutnoVreme = timeClient.getEpochTime();
  tmElements_t novoVremeElementi;
  breakTime(trenutnoVreme + (brojDanaZaPomeraj * 24 * 60 * 60), novoVremeElementi); // Pomeraj za brojDanaZaPomeraj dana

  return novoVremeElementi;}





// The repeating section of the code
void loop() {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
    irSignalString = resultToSourceCode(&results);
  }
  OTAloopHandler();
      
Parametri noviParametri;
mojTimer(noviParametri);
Parametri2 noviParametri2;
mojTimer2(noviParametri2);
  WiFiClient client = server.available(); // Listen for incoming clients

if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

bool isInjectRequest = false; 
    

    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;


String irAcDescription = IRAcUtils::resultAcToString(&results);
if( irAcDescription != 0 ) {
daljinac = true;
dugme = false;
}

time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  trenutniSati = timeinfo.tm_hour;
  trenutneMinute = timeinfo.tm_min;
 
danUNedelji = (timeinfo.tm_wday + 6) % 7 + 1;
danNedeljaStampa=  (timeinfo.tm_wday + 6) % 7 + 1;
danUNedelji=danUNedelji +1;

 



if (header.indexOf("X-Requested-With: appinventor.ai_ilija_ilija_maksimovic.inject") != -1) {
          isInjectRequest = true;
             if( daljinac == true && dugme == false){

         client.println("<p>" + irAcDescription + "</p>");
} else if(dugme == true && daljinac == false){

client.println("<p>" + ac.toString() + "</p>");
}
int z;
String a="1",b="2",c="3",d="4",e="5",f="6",g="7",h="8",j="9",k="0",null = "0";

if(String(parametri[z].onSati) == a || String(parametri[z].onSati) == b || String(parametri[z].onSati) == c || String(parametri[z].onSati) == d || String(parametri[z].onSati) == e || String(parametri[z].onSati) == f || String(parametri[z].onSati) == g || String(parametri[z].onSati) == h || String(parametri[z].onSati) == j || String(parametri[z].onSati) == k  ){
  FixNull = null + String(parametri[z].onSati); 
}else {FixNull = String(parametri[z].onSati); }

if(String(parametri[z].onMinuti) == a || String(parametri[z].onMinuti) == b || String(parametri[z].onMinuti) == c || String(parametri[z].onMinuti) == d || String(parametri[z].onMinuti) == e || String(parametri[z].onMinuti) == f || String(parametri[z].onMinuti) == g || String(parametri[z].onMinuti) == h || String(parametri[z].onMinuti) == j  || String(parametri[z].onMinuti) == k ){
  FixNull2 = null + String(parametri[z].onMinuti); 
}else {FixNull2 = String(parametri[z].onMinuti); }
int razlikaDana = 0;

if( parametri[z].infoDan == 0 && parametri[z].onSati < trenutniSati ){

razlikaDana= razlikaDana +1;

}else if(parametri[z].infoDan == 0 && parametri[z].onSati == trenutniSati && parametri[z].onMinuti < trenutneMinute){
razlikaDana= razlikaDana +1;
} else if( parametri[z].infoDan == 0  && parametri[z].onSati > trenutniSati ){
razlikaDana = 0;
}else if( parametri[z].infoDan == 0  && parametri[z].onSati == trenutniSati && parametri[z].onMinuti > trenutneMinute ){
razlikaDana = 0;
}else{

if ( parametri[z].infoDan > danNedeljaStampa) {
  razlikaDana = parametri[z].infoDan - danNedeljaStampa ;
} else {
  razlikaDana = 7 - (danNedeljaStampa - parametri[z].infoDan);
}
}
int brojDanaZaPomeraj = razlikaDana;
  tmElements_t novoVreme = pomeriDatum(brojDanaZaPomeraj);




  String tajmerInfo = "Tajmer " + String(brojacParametra) + " pali se u  " + FixNull + ":" + FixNull2 +  " Datum ";

// Dodaj nulu ispred dana ako je jednocifren
if (novoVreme.Day < 10) {
  tajmerInfo += "0" + String(novoVreme.Day);
} else {
  tajmerInfo += String(novoVreme.Day);
}

// Dodaj nulu ispred meseca ako je jednocifren
if (novoVreme.Month < 10) {
  tajmerInfo += "-0" + String(novoVreme.Month);
} else {
  tajmerInfo += "-" + String(novoVreme.Month);
}

tajmerInfo += "-" + String(novoVreme.Year + 1970);

client.println("<p>" + tajmerInfo + "</p>");


  


String referer = "";

// Pronalaženje indeksa "Referer:" u zaglavlju
int refererIndex = header.indexOf("Referer:");

// Ako je pronađen indeks "Referer:", izdvojite vrednost referera
if (refererIndex != -1) {
  int endOfLineIndex = header.indexOf('\n', refererIndex); // Pronalaženje kraja reda nakon "Referer:" stringa
  if (endOfLineIndex != -1) {
    referer = header.substring(refererIndex + 8, endOfLineIndex); // +8 zbog dužine "Referer:" stringa
    referer.trim(); // Uklonite eventualne praznine na početku i kraju referera
  }
}

if (!referer.isEmpty()) {
  // Ako je Referer pronađen, ispišite ga
  Serial.println("Referer: " + referer);
  
  // Razdvojite Referer URL na segmente koristeći znak "/"
  String segments[200]; // Pravite dovoljno veliki niz za segmente
  int numSegments = 0; // Broj segmenta

  char *str = strdup(referer.c_str()); // Konvertujte String u C string
  char *token = strtok(str, "/");
 
  

mojTajmer =false;
ponovi = false;
int mojTimerIndex = header.indexOf("mojTimer");
if( mojTimerIndex != -1){
  mojTajmer =true;

}



int onTimerIndex = header.indexOf("onTimer");

// Ako je pronađen indeks "onTimer", izdvojite vrednosti
if (onTimerIndex != -1 ) {
  onTajmer = true;
  // Pomerite se za 7 karaktera iza "onTimer" kako biste preskočili taj deo.
  onTimerIndex += 7;

  // Izdvojite sate (prva dva broja posle ':')
   satiStr = header.substring(onTimerIndex, onTimerIndex + 2);
  onTimer1 = satiStr.toInt();

  // Izdvojite minute (sledeća dva broja posle ':')
  String minutiStr = header.substring(onTimerIndex + 3, onTimerIndex + 5);
  onTimer2 = minutiStr.toInt();
  if ( mojTajmer ==false )
  ac.enableOnTimer(onTimer1 * 60 + onTimer2);
  // Sada imate vrednosti u intiger1 i intiger2
}

// Pronalaženje indeksa "offTimer" u zaglavlju
int offTimerIndex = header.indexOf("offTimer");

// Ako je pronađen indeks "offTimer", izdvojite vrednosti
if (offTimerIndex != -1) {
  offTajmer = true;
  // Pomerite se za 8 karaktera iza "offTimer" kako biste preskočili taj deo.
  offTimerIndex += 8;

  // Izdvojite sate (prva dva broja posle ':')
  String satiStr = header.substring(offTimerIndex, offTimerIndex + 2);
   offTimer1 = satiStr.toInt();

  // Izdvojite minute (sledeća dva broja posle ':')
  String minutiStr = header.substring(offTimerIndex + 3, offTimerIndex + 5);
   offTimer2 = minutiStr.toInt();
   if ( mojTajmer ==false )
  ac.enableOffTimer(offTimer1 * 60 + offTimer2);
  // Sada imate vrednosti u intiger1 i intiger2 za "offTimer"
}




// Pronađite poziciju znaka '*' u referer URL-u
int asteriskIndex = referer.indexOf('*');

// Proverite da li je '*' pronađen u stringu
if (asteriskIndex != -1) {
  // Izmestite se za jedan karakter iza '*'
  asteriskIndex++;

  // Izvucite sate (prva dva broja posle '*')
  String satiStr = referer.substring(asteriskIndex, asteriskIndex + 2);
   sati = satiStr.toInt();

  // Izvucite minute (sledeća dva broja posle ':')
  String minutiStr = referer.substring(asteriskIndex + 3, asteriskIndex + 5);
   minuti = minutiStr.toInt();

  // minuti u obliku intigera
  
  ac.setCurrentTime(sati * 60 + minuti);
  

}

int tempponIndex = header.indexOf("danpon");
if( tempponIndex != -1){
  IzabraniDan =1;
  
}
int temputoIndex = header.indexOf("danuto");
if( temputoIndex != -1){
  IzabraniDan = 2;
}
int tempsreIndex = header.indexOf("dansre");
if( tempsreIndex != -1){
  IzabraniDan =3;

}
int tempcetIndex = header.indexOf("dancet");
if( tempcetIndex != -1){
  IzabraniDan =4;

}
int temppetIndex = header.indexOf("danpet");
if( temppetIndex != -1){
  IzabraniDan =5;

}
int tempsubIndex = header.indexOf("dansub");
if( tempsubIndex != -1){
  IzabraniDan =6;

}
int tempnedIndex = header.indexOf("danned");
if( tempnedIndex != -1){
  IzabraniDan =7;

}

int tempponoviIndex = header.indexOf("ponoviOn");
if( tempponoviIndex != -1){
  ponovi = true;

}
int temp18Index = header.indexOf("temp18");
if( temp18Index != -1){
  tempZaStrutc = 18;
}

int temp19Index = header.indexOf("temp19");
if( temp19Index != -1){
  tempZaStrutc = 19;
}

int temp20Index = header.indexOf("temp20");
if( temp20Index != -1){
  tempZaStrutc = 20;
}

int temp21Index = header.indexOf("temp21");
if( temp21Index != -1){
  tempZaStrutc = 21;
}

int temp22Index = header.indexOf("temp22");
if( temp22Index != -1){
  tempZaStrutc = 22;
}

int temp23Index = header.indexOf("temp23");
if( temp23Index != -1){
  tempZaStrutc = 23;
}

int temp24Index = header.indexOf("temp24");
if( temp24Index != -1){
  tempZaStrutc = 24;
}

int temp25Index = header.indexOf("temp25");
if( temp25Index != -1){
  tempZaStrutc = 25;
}

int temp26Index = header.indexOf("temp26");
if( temp26Index != -1){
  tempZaStrutc = 26;
}

int temp27Index = header.indexOf("temp27");
if( temp27Index != -1){
  tempZaStrutc = 27;
}

int temp28Index = header.indexOf("temp28");
if( temp28Index != -1){
  tempZaStrutc = 28;
}

int temp29Index = header.indexOf("temp29");
if( temp29Index != -1){
  tempZaStrutc = 29;
}

int temp30Index = header.indexOf("temp30");
if( temp30Index != -1){
  tempZaStrutc = 30;
}

int temp31Index = header.indexOf("temp31");
if( temp31Index != -1){
  tempZaStrutc = 31;
}

int temp32Index = header.indexOf("temp32");
if( temp32Index != -1){
  tempZaStrutc = 32;
}

int modeFanIndex = header.indexOf("modeFan");
if( modeFanIndex != -1){
   modeZaStrutc = "modeFan";
}

int modeCoolIndex = header.indexOf("modeCool");
if( modeCoolIndex != -1){
   modeZaStrutc = "modeCool";
}

int modeDryIndex = header.indexOf("modeDry");
if( modeDryIndex != -1){
   modeZaStrutc = "modeDry";
}

int modeHeatIndex = header.indexOf("modeHeat");
if( modeHeatIndex != -1){
   modeZaStrutc = "modeHeat";
}

int modeAutoIndex = header.indexOf("modeAuto");
if( modeAutoIndex != -1){
   modeZaStrutc = "modeAuto";
}
int fan1Index = header.indexOf("fan1");
if( fan1Index != -1){
  
  IzabranFan = 1;
}
int fan2Index = header.indexOf("fan2");
if( fan2Index != -1){
  
  IzabranFan = 2;
}
int fan3Index = header.indexOf("fan3");
if( fan3Index != -1){
  IzabranFan = 3;
}
int fan4Index = header.indexOf("fan4");
if( fan4Index != -1){
  IzabranFan = 4;
}
int fan5Index = header.indexOf("fan5");
if( fan5Index != -1){
  IzabranFan = 5;
}
int fan10Index = header.indexOf("fan10");
if( fan10Index != -1){
  IzabranFan = 10;
}
int fan11Index = header.indexOf("fan11");
if( fan11Index != -1){
  IzabranFan = 11;
}
int powerfullIndex = header.indexOf("powerfullOn");
if( powerfullIndex != -1){
  PowerfullOn= true;
}
int quietIndex = header.indexOf("quietOn");
if( quietIndex != -1){
   QuietOn = true;
}
int sensorIndex = header.indexOf("sensorOn");
if( sensorIndex != -1){
  SensorOn = true;
}
int mouldIndex = header.indexOf("mouldOn");
if( mouldIndex != -1){
  MouldOn = true;
}
int comfortIndex = header.indexOf("comfortOn");
if( comfortIndex != -1){
  ComfortOn = true;
}
int swinghIndex = header.indexOf("swinghOn");
if( swinghIndex != -1){
  SwinghOn = true;
}
int swingvIndex = header.indexOf("swingvOn");
if( swingvIndex != -1){
  SwingvOn = true;
}
int weeklyIndex = header.indexOf("weeklyOn");
if( weeklyIndex != -1){
  WeeklyOn = true;
}
  while (token != NULL) {
    segments[numSegments++] = token;
    token = strtok(NULL, "/");
  }
  
  free(str); // Oslobadjanje memorije

  // Ispitivanje svakog segmenta pojedinačno
  for (int i = 0; i < numSegments; i++) {


 if (segments[i].equals("On")) {
       ac.on();
    }


else if (segments[i].equals("Off")) {
       ac.off();
    }



    else if (segments[i].equals("modeFan")) {
        modeZaStrutc = "";
      ac.setMode(kDaikinFan);
      
    }



else if (segments[i].equals("modeHeat")) {

     modeZaStrutc = "";
      ac.setMode(kDaikinHeat);
     
    }


else if (segments[i].equals("modeCool")) {
   modeZaStrutc = "";
      ac.setMode(kDaikinCool);
    }

 else if (segments[i].equals("modeAuto")) {
   modeZaStrutc = "";
      ac.setMode(kDaikinAuto);
      
    } else if (segments[i].equals("modeDry")) {
      modeZaStrutc = "";
      ac.setMode(kDaikinDry);
    }
    else if (segments[i].equals("temp18")) {
       tempZaStrutc = 0;
      ac.setTemp(18);
    
    } 
    else if (segments[i].equals("temp19")) {
  
      tempZaStrutc = 0;
      ac.setTemp(19);
      
    }
    
    else if (segments[i].equals("temp20")) {
      tempZaStrutc = 0;
      ac.setTemp(20);
    }
    
    else if (segments[i].equals("temp21")) {
      tempZaStrutc = 0;
      ac.setTemp(21);
    }
    
    else if (segments[i].equals("temp22")) {
      tempZaStrutc = 0;
      ac.setTemp(22);
    }


    else if (segments[i].equals("temp23")) {
      tempZaStrutc = 0;
      ac.setTemp(23);
    }

else if (segments[i].equals("temp24")) {
  tempZaStrutc = 0;
      ac.setTemp(24);
    }
else if (segments[i].equals("temp25")) {
  tempZaStrutc = 0;
      ac.setTemp(25);
    }
else if (segments[i].equals("temp26")) {
  tempZaStrutc = 0;
      ac.setTemp(26);
    }
    
    else if (segments[i].equals("temp27")) {
      tempZaStrutc = 0;
      ac.setTemp(27);
    }

else if (segments[i].equals("temp28")) {
  tempZaStrutc = 0;
      ac.setTemp(28);
    }


else if (segments[i].equals("temp29")) {
  tempZaStrutc = 0;
      ac.setTemp(29);
    }

else if (segments[i].equals("temp30")) {
  tempZaStrutc = 0;
      ac.setTemp(30);
    }

else if (segments[i].equals("temp31")) {
  tempZaStrutc = 0;
      ac.setTemp(31);
    }

else if (segments[i].equals("temp32")) {
  tempZaStrutc = 0;
      ac.setTemp(32);
    }

    else if (segments[i].equals("fan1")) {
      IzabranFan =-1;
      ac.setFan(1);
    }
     
     else if (segments[i].equals("fan2")) {
       IzabranFan =-1;
      ac.setFan(2);
    }
    else if (segments[i].equals("fan3")) {
      IzabranFan =-1;
      ac.setFan(3);
    }
    else if (segments[i].equals("fan4")) {
      IzabranFan =-1;
      ac.setFan(4);
    }
    else if (segments[i].equals("fan5")) {
      IzabranFan =-1;
      ac.setFan(5);
    }
    else if (segments[i].equals("fan10")) {
      IzabranFan =-1;
      ac.setFan(10);
    }else if (segments[i].equals("fan11")) {
      IzabranFan =-1;
      ac.setFan(11);
    }
    else if (segments[i].equals("powerfullOff")) {
      ac.setPowerful(false);
    }
    else if (segments[i].equals("powerfullOn")) {
       PowerfullOn = false;
      ac.setPowerful(true);
    }
    else if (segments[i].equals("quietOff")) {
      ac.setQuiet(false);
    }
    else if (segments[i].equals("quietOn")) {
      QuietOn = false;
      ac.setQuiet(true);
    }
    else if (segments[i].equals("sensorOff")) {
       ac.setSensor(false);
    }
    else if (segments[i].equals("sensorOn")) {
      SensorOn = false;
       ac.setSensor(true);
    }
    else if (segments[i].equals("mouldOff")) {
      ac.setMold(false);
    }
    else if (segments[i].equals("mouldOn")) {
      MouldOn = false;
      ac.setMold(true);
    }
    else if (segments[i].equals("comfortOff")) {
      ac.setComfort(false);
    }
    else if (segments[i].equals("comfortOn")) {
       ComfortOn = false;
      ac.setComfort(true);
    }
    else if (segments[i].equals("swinghOff")) {
       ac.setSwingHorizontal(false);
    }
    else if (segments[i].equals("swinghOn")) {
      SwinghOn = false;
       ac.setSwingHorizontal(true);
    }
    else if (segments[i].equals("swingvOff")) {
      ac.setSwingVertical(false);
    } 
    else if (segments[i].equals("swingvOn")) {
      SwingvOn = false;
       ac.setSwingVertical(true);
    }
else if (segments[i].equals("weeklyOn")) {
        WeeklyOn = false;
        ac.setWeeklyTimerEnable(true);
    }

    else if (segments[i].equals("ponedeljak")) {
       ac.setCurrentDay(2);
    }
     else if (segments[i].equals("utorak")) {
        ac.setCurrentDay(3);
    }
    
    else if (segments[i].equals("sreda")) {
        ac.setCurrentDay(4);
    }
    else if (segments[i].equals("cetvrtak")) {
       ac.setCurrentDay(5);
    }
    else if (segments[i].equals("petak")) {
       ac.setCurrentDay(6);
    }
    else if (segments[i].equals("subota")) {
       ac.setCurrentDay(7);
    }
    else if (segments[i].equals("nedelja")) {
       ac.setCurrentDay(1);
    }
    else if (segments[i].equals("serbia")) {
       ac.setCurrentTime( trenutniSati * 60 + trenutneMinute);
       ac.setCurrentDay(danUNedelji);
       
    }
    

    else if (segments[i].equals("sendiR")) {
     

      client.println("<p>" + ac.toString() + "</p>");

      dugme =true;
      daljinac = false;
       Serial.println("Sendir sendovano");
      goto simulateButtonClick;
       
         }
      else if( mojTimerIndex != -1 && brojacParametra <= 10 &&(offTimerIndex != -1 || onTimerIndex != -1)){
  
       
    Serial.println("Upit valja");
    Serial.println(ponovi);
    if(bag1!=onTimer1 || bag2!= onTimer2){
      parametri[brojacParametra].onSati = onTimer1 ;
      parametri[brojacParametra].onMinuti = onTimer2;
       parametri[brojacParametra].temp = tempZaStrutc;
       parametri[brojacParametra].fan = IzabranFan;
     parametri[brojacParametra].mode= modeZaStrutc;
     parametri[brojacParametra].infoDan = IzabraniDan;
     parametri[brojacParametra].ponovi = ponovi;
     parametri[brojacParametra].powerfullOn = PowerfullOn;
     parametri[brojacParametra].quiet = QuietOn;
    parametri[brojacParametra].sensor = SensorOn;
    parametri[brojacParametra].comfort = ComfortOn;
     parametri[brojacParametra].mould = MouldOn;
     parametri[brojacParametra].swingh = SwinghOn;
     parametri[brojacParametra].swingv = SwingvOn;
     parametri[brojacParametra].weekly = WeeklyOn;
     z=brojacParametra;
    brojacParametra++;
    }
if(bag3!=offTimer1 || bag4!= offTimer2){
      parametri2[brojacParametra2].offSati = offTimer1 ;
      parametri2[brojacParametra2].offMinuti = offTimer2;
      parametri2[brojacParametra2].infoDan2 = IzabraniDan;
      parametri2[brojacParametra2].ponovi2 = ponovi;
    brojacParametra2++;
    }
      IzabranFan = -1;
     mojTajmer =false;
     IzabraniDan=0;
     ponovi = false;
     PowerfullOn = false;
     QuietOn = false;
     SensorOn = false;
     MouldOn = false;
     ComfortOn = false;
     SwinghOn = false;
     SwingvOn = false;
     WeeklyOn = false;
         // parametri[brojacParametra] = noviParametri;
           for (int i = 0; i < 10; ++i) {
    Serial.print("Parametri ");
    Serial.println(i);
    ispisiParametre(parametri[i]);
      
  }
  for (int i = 0; i < 10; ++i) {
    Serial.print("Parametri2 ");
    Serial.println(i);
    ispisiParametre2(parametri2[i]);
      
  }
  


    goto izlazPetlja;
     }
      else if (brojacParametra >= 10) {
          Serial.println("Nema više mesta za dodavanje novih setova parametara.");
        } 
     }

     }
     


}








        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
// checking if header is valid
// dXNlcjpwYXNz = 'user:pass' (user:pass) base64 encode
// Finding the right credential string, then loads web page
if(header.indexOf("dXNlcjpwYXNz") >= 0) {



  





          
        









// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
             izlazPetlja:
            // turns the GPIOs on and off
           
             if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
                ac.setFan(-1);
                Serial.println("Podešavanje brzine ventilatora na 10");
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
                    ac.setFan(0);
                Serial.println("Podešavanje brzine ventilatora na 0");

            }


              if (header.indexOf("GET /Upali/on") >= 0) {
                Serial.println("Paljenje");
                ac.on();
            } else if (header.indexOf("GET /Ugasi/off") >= 0) {
                Serial.println("Gasenje");
                ac.off();
            }





            if (header.indexOf("GET /ventilator/1") >= 0) {
                Serial.println("Podešavanje brzine ventilatora na 1");
                ac.setFan(1);
            } else if (header.indexOf("GET /ventilator/2") >= 0) {
                Serial.println("Podešavanje brzine ventilatora na 2");
               ac.setFan(2);
            } else if (header.indexOf("GET /ventilator/3") >= 0) {
                Serial.println("Podešavanje brzine ventilatora na 3");
                 ac.setFan(3);
            } else if (header.indexOf("GET /ventilator/4") >= 0) {
                Serial.println("Podešavanje brzine ventilatora na 4");
                ac.setFan(4);
            } else if (header.indexOf("GET /ventilator/5") >= 0) {
                Serial.println("Podešavanje brzine ventilatora na 5");
                ac.setFan(5);
            } else if (header.indexOf("GET /mode/auto") >= 0) {
                Serial.println("Proba");
                 
                 ac.setMode(kDaikinAuto);
              
            } else if (header.indexOf("GET /mode/cool") >= 0) {
                Serial.println("Proba");
                
               ac.setMode(kDaikinCool);
            } else if (header.indexOf("GET /mode/dry") >= 0) {
                Serial.println("Proba");
                
               ac.setMode(kDaikinDry);
            }else if (header.indexOf("GET /mode/heat") >= 0) {
                Serial.println("Proba");
                
               ac.setMode(kDaikinHeat);
            } else if (header.indexOf("GET /mode/fan") >= 0) {
                Serial.println("Proba");
                
               ac.setMode(kDaikinFan);
            }   
                 
 
 
  ac.setTemp(25);
  ac.setSwingVertical(false);
  ac.setSwingHorizontal(false);

  // Set the current time to 1:33PM (13:33)
  // Time works in minutes past midnight
 
  // Turn off about 1 hour later at 2:30PM (14:30)
 

               if (header.indexOf("GET /sendIR") >= 0 && !signalSent) {
    Serial.println("Sending...");
    simulateButtonClick:
    // Slanje signala i ostatak koda...
       // Now send the IR signal.
             signalSent = true;
           #if SEND_DAIKIN
             // Prikaz onoga što ćete poslati
        Serial.println(ac.toString());
           
            ac.send();
          client.println("<script>");
          client.println("setTimeout(function(){ window.location.replace('/'); }, 3000);");
           client.println("</script>");

        // Čekajte da sprečite kontinuirano slanje
        delay(1000);
    // Obeležite signal kao poslat
        signalSent = false;
   #endif 
    
}                 
      
                    
                
              client.println("<p><a href=\"/sendIR\"><button class=\"button\">Posalji IR Signal</button></a></p>");

              

String irAcDescription = IRAcUtils::resultAcToString(&results);

// Ispis osnovnog HTML koda
client.println("<!DOCTYPE html><html>");
client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
client.println("<link rel=\"icon\" href=\"data:,\">");
client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
client.println(".button2 {background-color: #555555;}</style></head>");

client.println("<p><a href=\"/Upali/on\"><button class=\"button\">Upali</button></a></p>");
client.println("<p><a href=\"/Ugasi/off\"><button class=\"button\">Ugasi</button></a></p>");


client.println("<p>Brzina Ventilatora:</p>");
client.println("<p><a href=\"/ventilator/1\"><button class=\"button\">Brzina 1</button></a></p>");
client.println("<p><a href=\"/ventilator/2\"><button class=\"button\">Brzina 2</button></a></p>");
client.println("<p><a href=\"/ventilator/3\"><button class=\"button\">Brzina 3</button></a></p>");
client.println("<p><a href=\"/ventilator/4\"><button class=\"button\">Brzina 4</button></a></p>");
client.println("<p><a href=\"/ventilator/5\"><button class=\"button\">Brzina 5</button></a></p>");
client.println("<p><a href=\"/mode/auto\"><button class=\"button\">Mode Auto</button></a></p>");
client.println("<p><a href=\"/mode/dry\"><button class=\"button\">Mode Dry</button></a></p>");
client.println("<p><a href=\"/mode/cool\"><button class=\"button\">Mode Cool</button></a></p>");
client.println("<p><a href=\"/mode/heat\"><button class=\"button\">Mode Heat</button></a></p>");
client.println("<p><a href=\"/mode/fan\"><button class=\"button\">Mode Fan</button></a></p>");

if (irAcDescription.indexOf("Power: On") >= 0) {
  client.println("<p>Klima uredjaj je ukljucen.</p>");
  client.println("<p>Snimljeni IR signal: " + irAcDescription + "</p>");
} else if (irAcDescription.indexOf("Power: Off") >= 0) {
  client.println("<p>Klima uredjaj je iskljucen.</p>");
} else if (irAcDescription.length() == 0) {
  client.println("<p>Klima uredjaj je iskljucen.</p>");
}

// JavaScript kod za automatsko osvežavanje svakih 5 sekundi
//client.println("<script>setTimeout(function(){ location.reload(); }, 2000);</script>");

client.println("</body></html>");


        

                    

              
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
           
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();




break;
}
// Wrong user or password, so HTTP request fails... 
else { 
client.println("HTTP/1.1 401 Unauthorized");
client.println("WWW-Authenticate: Basic realm=\"Secure\"");
client.println("Content-Type: text/html");
client.println();
client.println("<html>Authentication failed</html>");
break;
} 
} else { // if you got a newline, then clear currentLine
currentLine = "";
}
} else if (c != '\r') { // if you got anything else but a carriage return character,
currentLine += c; // add it to the end of the currentLine
}
}
}





// Clear the header variable
header = "";
// Close the connection
client.stop();
Serial.println("Client disconnected.");
Serial.println("");
// Obeležite signal kao neposlat
signalSent = false;
Serial.println("Signal marked as unsent");

}
}