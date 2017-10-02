#include <SoftwareSerial.h>
#include<string.h>
bool first_loop_exec;
const int button = 9 ;
int btnstate = 0;
unsigned long time_1=0;
unsigned long time_2=0;
int a[100];
int t;
 
#define GPS_INFO_BUFFER_SIZE 128
char GPS_info_char;
char GPS_info_buffer[GPS_INFO_BUFFER_SIZE];
unsigned int received_char;
 
int i; // counter
bool message_started;
SoftwareSerial BTserial(10, 11); // RX | TX 
SoftwareSerial mySerial_GPS(7, 8); // 7=RX, 8=TX (needed to communicate with GPS)
 
// REAL TIME SCHEDULER PARAMETERS AND VARIABLES
#define SCHEDULER_TIME 10000 // scheduler interrupt runs every 20000us = 20ms
#define DIVIDER_STD 200 // logging message sent every 100 scheduler times (20ms) 1s
#define DIVIDER_DELAY 500 // delay after forwarding meggages is 3s
unsigned int divider=0;
unsigned int divider_max=DIVIDER_DELAY;
 
 int integer(char a)
 {
   int b = a - '0';
   return(b);
 }
 char character(int a)
 {
  char b = a+'0';
  return(b);
 }
 
 
// SENDS THE POLLING MESSAGE TO GPS
void scheduled_interrupt() 
{
  divider++;
  if (divider==divider_max) {
    divider=0;
    divider_max=DIVIDER_STD;
    time_1 = millis();
    mySerial_GPS.println("$PUBX,00*33"); // data polling to the GPS
  }
}
 
void setup() {
  pinMode(button,INPUT);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Connected");
  BTserial.begin(9600); 
  mySerial_GPS.begin(9600);
  mySerial_GPS.println("Connected");
 
  first_loop_exec=true;
  i=0;
  message_started=false;
 }
 
void loop() { // run over and over
 
  if (first_loop_exec == true){
    delay(2000);
    mySerial_GPS.println(F("$PUBX,40,RMC,0,0,0,0*47")); //RMC OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,VTG,0,0,0,0*5E")); //VTG OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GGA,0,0,0,0*5A")); //CGA OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GSA,0,0,0,0*4E")); //GSA OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GSV,0,0,0,0*59")); //GSV OFF
    delay(100);
    mySerial_GPS.println(F("$PUBX,40,GLL,0,0,0,0*5C")); //GLL OFF
    delay(1000);
    first_loop_exec = false;
  }
 
  // MANAGES THE CHARACTERS RECEIVED BY GPS
  while (mySerial_GPS.available()) {
    GPS_info_char=mySerial_GPS.read();
    if (GPS_info_char == '$'){ // start of message
      message_started=true;
      received_char=0;
    }else if (GPS_info_char == '*'){ // end of message
      time_2 = millis();
      char st[26];
      st[0] =GPS_info_buffer[18];
      st[1]='\0';
      char tmp[2];
      tmp[1]='\0';
        for(i=19;i<=43;i++)
      {
        tmp[0] = GPS_info_buffer[i];
        strcat(st,tmp);
     }
   
     
     char sa1[3] = {st[0],st[1],'\0'};
     char sa2[3]= {st[2],st[3],'\0'};
     char sa3[6] = {st[5],st[6],st[7],st[8],st[9],'\0'};
     
     char lsa1[4] = {st[13],st[14],st[15],'\0'};
     char lsa2[3]= {st[16],st[17],'\0'};
     char lsa3[6] = {st[19],st[20],st[21],st[22],st[23],'\0'};
     
     //int iln1[2],iln2[2],iln3[5],ilg1[3],ilg2[2],ilg3[5];
     int td = 10;
     float lat1=0;
     
     for(i=0;i<=1;i++)
       {
         lat1=lat1 + td * integer(sa1[i]);
         td = td/10;
       }
       
       td= 10;
      float lat2= 0;
       for(int i=0;td!=0;i++)
       {
         lat2=lat2 + td * integer(sa2[i]);
         td = td/10;
       }
        td = 10000;
       float lat3=0;
        for(int i=0;td!=0;i++)
       {
         lat3=lat3 + td * integer(sa3[i]);
         td = td/10;
       }
        td = 100;
       float lng1=0;
        for(int i=0;td!=0;i++)
       {
        lng1=lng1 + td * integer(lsa1[i]);
         td = td/10;
       }
        td = 10;
       float lng2=0;
        for(int i=0;td!=0;i++)
       {
         lng2=lng2 + td * integer(lsa2[i]);
         td = td/10;
       }
         td = 10000;
       float lng3=0;
        for(int i=0;td!=0;i++)
       {
         lng3=lng3 + td * integer(lsa3[i]);
         td = td/10;
       }
      
      float latitude = lat1 +(lat2+lat3/100000)/60;
      float longitude =lng1 +(lng2+ lng3/100000)/60;
      if(st[11]=='S')
        latitude= -latitude;
      if(st[25]=='W')
         longitude = -longitude;
      
      char fstr[100];
      long int inlat = latitude*100000,t1=0,count=6;
      long int inlong= longitude*100000,t2=0;
      long int st5=inlat;
      
      while(inlat!=0)
      {
       t1=inlat%10;
       if(count==1)
       {
         fstr[count]='.';
         count=count-1;
       }
       fstr[count]=character(t1);
       count= count-1;
       inlat=inlat/10;
      }
      fstr[7]=',';
      count=15;

      while(inlong!=0)
      {
       t2=inlong%10;
       if(count==10)
       {
         fstr[count]='.';
         count=count-1;
       }
       fstr[count]=character(t2);
       count= count-1;
       inlong=inlong/10;
      }
      fstr[16]=';';
      fstr[17]='\0';
      
      Serial.println(fstr);
      Serial.println("Latitude : ");
      Serial.println(latitude,5);
      Serial.println("Longitude : ");
      Serial.println(longitude,5);
      Serial.println("__________________________________________________________________");
    
    
      btnstate = digitalRead(button);
      if(btnstate == LOW){
        delay(100);
        btnstate = digitalRead(button);
        if(btnstate == LOW){
        Serial.println("button pressed");
        //BTserial.print(latitude,5); BTserial.print(","); BTserial.print(longitude,5);
        BTserial.print(fstr);
        }
      }
      else
       BTserial.print(fstr);


      delay(20);
      
      message_started=false; // ready for the new message
    }else if (message_started==true){ // the message is already started and I got a new character
      if (received_char<=GPS_INFO_BUFFER_SIZE){ // to avoid buffer overflow
        GPS_info_buffer[received_char]=GPS_info_char;
        received_char++;
      }else{ // resets everything (overflow happened)
        message_started=false;
        received_char=0;
      }
    }
  }
 
  while (Serial.available()) {
    mySerial_GPS.write(Serial.read());
  }
  
}
