#include <SPI.h>
#include <UIPEthernet.h>

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

EthernetClient client;
char server[] = "api.thingspeak.com";
const String API_KEY = "API_KEY_GOES_HERE";
const unsigned long SERVER_SYNC_INTERVAL = 30 * 1000UL;

static const int GREEN_BUTTON = 9;
static const int BLUE_BUTTON = 8;
static const int RED_BUTTON = 7;

static const int GREEN_LED = 2;
static const int BLUE_LED = 3;
static const int RED_LED = 4;

static const int NOISE_DIGITAL_INPUT = 6;
static const int NOISE_ANALOG_INPUT = A1;

static const byte buttons[3] = {GREEN_BUTTON, BLUE_BUTTON, RED_BUTTON};
byte moodCounters[3] = {0, 0, 0};

void setup() {
  initStatusLeds ();
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(RED_LED, HIGH);

  Serial.println("booting...");
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  setupEthernet();
  initButtons();
  initNoise();

  Serial.println("Initialized!");
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(RED_LED, LOW);  
}

void loop() {
  maintainEthernet();
  handleButtons();
  handleResponses();
  handleNoise();
  handleCommunication(); 
}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

void setupEthernet() {
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  // print your local IP address:
  printIPAddress();
}

void maintainEthernet() {
   switch (Ethernet.maintain())
  {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");

      //print your local IP address:
      printIPAddress();
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");

      //print your local IP address:
      printIPAddress();
      break;

    default:
      //nothing happened
      break;

  }
}

void initButtons() {
   for (int i = 0; i < sizeof(buttons) / sizeof(*buttons) ; i++) {
    pinMode(buttons[i], INPUT_PULLUP);       
  }

}

byte previousState = 0;
void handleButtons() {
  byte currentState = 0;
  for (byte i = 0; i < sizeof(buttons) / sizeof(*buttons); i++) {
    if (digitalRead(buttons[i]) == LOW) {      
      currentState |= 1 << i;
    } else {      
      currentState &= ~(1 << i);
    }
  }
  if (currentState != previousState) {
    for (byte i = 0; i < sizeof(buttons) / sizeof(*buttons); i++) {      
      if (((previousState >> i) & 1) == 0 && ((currentState >> i) & 1) == 1 ) {        
        
        moodCounters[i]++;
      }
    }
    printCounters();
    digitalWrite(BLUE_LED, HIGH);
  }
  previousState = currentState;  
  
}
void printCounters() {
    for (byte i = 0; i < sizeof(moodCounters) / sizeof(*moodCounters); i++) {                   
      Serial.print(i);
      Serial.print("=");
      Serial.print(moodCounters[i]);      
      Serial.print(" ");
    } 
    Serial.println();
}

static int sendData() {
   printCounters();
  
   if (client.connect(server, 80)) {
    Serial.println("connected");
    digitalWrite(GREEN_LED, HIGH);
    client.print("GET /update?api_key=");
    client.print(API_KEY);
    if (moodCounters[0] > 0) {
      client.print("&field1=");
      client.print(moodCounters[0]);
    }
    if (moodCounters[1] > 0) {
      client.print("&field2=");
      client.print(moodCounters[1]);
    }
     if (moodCounters[3] > 0) {
      client.print("&field3=");
      client.print(moodCounters[2]);
    }
    
   
    client.println(" HTTP/1.0");    
    client.println("Connection: close");
    client.println();
    return 0;
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    return 1;
  }    
}

void handleResponses() {
   if (client.available()) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, LOW);
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {    
    client.stop();
  }
}

void initNoise() {
  pinMode(NOISE_ANALOG_INPUT,INPUT);
  pinMode(NOISE_DIGITAL_INPUT, INPUT);

}





void handleNoise() {

}

static unsigned long lastSampleTime = 0;
void handleCommunication() {
  unsigned long now = millis();
  if (now - lastSampleTime >= SERVER_SYNC_INTERVAL)
  {
    digitalWrite(GREEN_LED, LOW);    
    digitalWrite(RED_LED, HIGH);    
    
    lastSampleTime = now;
    int result = sendData();
    if ( result == 0) {
      dataSent();
    }
  }  
}

void dataSent() {

    for (byte i = 0; i < sizeof(moodCounters) / sizeof(*moodCounters); i++) {           
      moodCounters[i] = 0;
    }    
  
}

void initStatusLeds() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
}


