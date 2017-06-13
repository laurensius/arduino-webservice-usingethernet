#include<LiquidCrystal.h>
#include<dht11.h>
#include <Ethernet.h>
#include <SPI.h>

#define DHT_SENSOR_PIN 24
#define STATUS_CONNECTED 1
#define STATUS_DISCONNECTED 0

char namaServer[] = "169.254.110.195";
//char namaServer[] = "iot-project.laurensius-dede-suhardiman.com";

//byte IP_eth[] = {192,168,0,110};
byte IP_eth[] = {169,254,110,196};
byte MAC_eth[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


int respon_dht11;
int counter = 0;

boolean startRead = false; 

char inString[32];
char charFromWeb[9];

dht11 sensor_dht;
EthernetClient myEthernet;

int iterasi = 0;

void setup(){
  Serial.begin(9600);
  Serial.println("--------------------------------------------------"); 
  Serial.println("Setting Perangkat");
  Serial.println("Mohon menunggu . . . ");
  Serial.println("Setting Ethernet MAC Address dan IP Address");
  Serial.println("Mohon menunggu . . . ");
  if (Ethernet.begin(MAC_eth) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(MAC_eth,IP_eth);
  }
 // Ethernet.begin(MAC_eth,IP_eth);
  Serial.println("Setting Sensor Suhu dan Kelembaban");
  Serial.println("Mohon menunggu . . . ");
  Serial.print("Versi Library DHT : ");
  Serial.println(DHT11LIB_VERSION);
  delay(1000);
  Serial.println("Setting Perangkat selesai!");
  Serial.println("--------------------------------------------------");
}

void loop() {
  iterasi++;
  Serial.print("Iterasi ke : ");
  Serial.println(iterasi);
  inisialisasi_dht11();
  String a = ambil_data_dht11();
  int resultBukaKoneksi = bukaKoneksi();
  if(resultBukaKoneksi==1){
      kirimData(a);
      Serial.println();
  }
  delay(1000); 
  Serial.println("--------------------------------------------------");
}

int bukaKoneksi(){
  Serial.print("Mencoba sambungan ke server http://"); 
  Serial.println(namaServer);  
  Serial.println("Mohon menunggu . . . ");
  if(myEthernet.connect(namaServer,80)){
    Serial.println("Sambungan ke server berhasil!");
    return STATUS_CONNECTED; 
  }else{
    Serial.print("Sambungan ke server gagal!");
    Serial.println();
    return STATUS_DISCONNECTED;
  }
}

void kirimData(String a){
    Serial.println("Menjalankan perintah kirim data");
    String data = " Arduino";
    int ln = data.length();
    String uri_segment;
    uri_segment = "/iot_server/index.php/device/post_data/" + a;
//    uri_segment = "/iot_server/index.php/device/post_data/" + a; 
    myEthernet.print("GET ");
    myEthernet.print(uri_segment); 
    Serial.print("Data yang dikirim di ke server : ");
    Serial.println(a);
    myEthernet.println(" HTTP/1.0");
    myEthernet.print( "Host: " );
    myEthernet.println(" iot-project.laurensius-dede-suhardiman.com \r\n");
//    myEthernet.println(" 192.168.0.102 \r\n");
    Serial.println("Host OK");
    myEthernet.println( "Content-Type: application/x-www-form-urlencoded \r\n" );
    Serial.println("Content type OK");
    myEthernet.print( "Content-Length: " );
    myEthernet.print(ln);
    myEthernet.print(" \r\n");
    myEthernet.println( "Connection: close" );
    myEthernet.println();
    String res;
    res = bacaWebText();
    if(res.equals("")==false){
      Serial.println("Data suhu dan kelembaban tersimpan.");
      Serial.print("Jumlah rows database ada : ");
      Serial.println(res);
    }
//------------warning selalu tutup koneksi ya........
//    myEthernet.stop();
//    myEthernet.flush();
//--------------------------------------------------
}

String bacaWebText(){
  unsigned int time;
  Serial.println("Baca respon dari server . . . "); 
  Serial.println("Mohon menunggu . . . ");
  time = millis();
  Serial.print("Timer Millis () : ");
  Serial.println(time);
  int stringPos = 0;
  memset( &inString, 0, 1024 );
  int unvailable_ctr = 0;
  while(true){
    if (myEthernet.available()) {
      char c = myEthernet.read();
      Serial.print(c);
      if (c == '#' ) { 
        Serial.print("Menemukan start key # dengan isi : ");
        startRead = true;  
      }else if(startRead){
        if(c != '^'){ 
          inString[stringPos] = c;
          stringPos ++;
        }else{
          startRead = false;
          Serial.println();
          Serial.println("Baca respon dari server selesai!");
          myEthernet.stop();
          myEthernet.flush();
          Serial.println("Sambungan diputuskan . . . ");
          return inString;
        }
      }
    }else{
       //Serial.println("ethernet unavailable");
       delay(50);
       unvailable_ctr++;
       if(unvailable_ctr == 25){
         myEthernet.stop();
         myEthernet.flush();
         Serial.println("Koneksi mengalami time out");
         Serial.println("Sambungan diputuskan . . . ");
         Serial.println("Reset...");
         return inString;
       }
    }
  }
}

void inisialisasi_dht11(){
  respon_dht11 = sensor_dht.read(DHT_SENSOR_PIN);
  switch (respon_dht11){
    case DHTLIB_OK:  
      Serial.println("Sensor DHT status : OK");
      break;
    case DHTLIB_ERROR_CHECKSUM: 
      Serial.println("Sensor DHT status : Checksum error");
      break;
    case DHTLIB_ERROR_TIMEOUT: 
      Serial.println("Sensor DHT status : Time out error");
      break;
    default: 
      Serial.println("Sensor DHT status : Unknown error"); 
      break;
  }
  delay(1000);
}

String ambil_data_dht11(){
  Serial.print("Temperatur : ");
  Serial.print(sensor_dht.temperature);
  Serial.print(",\t");
  Serial.print("Kelembaban : ");
  Serial.println(sensor_dht.humidity);
  delay(2000);
  String temperature = String(sensor_dht.temperature);
  String humidity = String(sensor_dht.humidity);
  String dew_point = String(dewPoint(sensor_dht.temperature,sensor_dht.humidity));
  String data = temperature + "/" + humidity + "/" + dew_point ;
  return data;
}

double dewPoint(double celsius, double humidity){
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);
  double VP = pow(10, RHS - 3) * humidity;
  double T = log(VP/0.61078);   
  return (241.88 * T) / (17.558 - T);
}

