/****************************************************************
V1.0 of my bluetooth audio HUD

Uses BC127 bluetooth audio chip to stream music from my phone to my car's aux in
and displays song title and artist info on LCD keypad shield

Future updates:
-Add looping scroll of title
-Implement LCD keypad buttons for song and volume control
-Utilize larger LCD display to show more info

Author: NBetrabet
****************************************************************/

#include <SparkFunbc127.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //establishes lcd display for use, for keypad shield these pins are assigned

SoftwareSerial swPort(2,3); //RX = 2, TX = 3

BC127 btModu(&swPort); //creates bluetooth module to send and recieve metadata over  software serial

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  swPort.begin(9600);
  lcd.begin(16,2); //define characteristics of lcd display, 16 cols x 2 rows

}

String buffer = ""; //buffer here because this is how I saw it in the sample code for the BC127
const String titleFind = "AVRCP_MEDIA TITLE:"; //quick and dirty method for isolating title from metadata
const String artistFind = "AVRCP_MEDIA ARTIST:"; //same method to isolate artist from metadata
//String currentTitle = "";
//String currentArtist = "";
//char title[50];
String title = ""; //holds title so that we can access it to scroll title if necessary
String artist = ""; //holds artist name

void singleScroll(String info, int line){ //Scrolls text on a single line
  int strLen = info.length(); //gets length of string
  Serial.println(strLen);
  if(strLen > 16){ //only scroll if the text has more characters than there are on the display
    int shift = strLen - 16; //determines how many times we have to shift
    for(int i = 0; i < shift; i++){ //controls number of times we print as well as dictating which character will be the first one printed in each iteration
      for(int y = 0; y < 16; y++){ //ensures 16 characters are printed
        lcd.setCursor(y, line); //moves cursor one space at a time
        lcd.print(info[y + i]); //prints character one character at a time
      }
      delay(500); //delay so that we can actually read the text
    }
    return;
  }
  else{ //if the text is short enough, just print it 
    lcd.setCursor(0, line);
    lcd.print(info);
  }
  return;
}

void loop() {
   //resetFlag used to monitor state of bluetooth connection
  static boolean resetFlag = false;
  
  // if the module is not connected and it hasn't been reset
  if (btModu.connectionState() == BC127::CONNECT_ERROR  && !resetFlag)
  {  
    Serial.println("Connection lost! Resetting...");
    //restore config of module
    btModu.restore();
    
    // set module as audio sink
    btModu.setClassicSink();
    
    // write current config to module
    btModu.writeConfig();
    // reset module
    btModu.reset();
    // Change the resetFlag, so we know we've restored the module to a usable state.
    resetFlag = true;
  }

  if(btModu.connectionState() == BC127::SUCCESS){ //while connected
    resetFlag = false;
    String line = ""; //holds current line of text from metadata stream
    int lines = 0; //records the number of lines we have looked at
    int rec = 0; //holds current recorded info from metadata stream, is 0 just for default
    while(lines < 10){ //read arbitrary number of lines to ensure we get title and artist
      while(rec != 10){ //while not a new line char
        rec = swPort.read(); //read the data stream from BC127
        if(rec > 10){ //if the char is not a new line char
          Serial.print(char(rec)); //printed just for monitoring
          line += char(rec); //append to current line of data
        }
      } 
      if(line.indexOf(titleFind) >= 0){ //if we find the notification for title
        //String holder = line.substring(20);
        //holder.toCharArray(title, holder.length());
        title = line.substring(20); //assign to title var
        Serial.print(title); //print to monitor
        lcd.clear(); //clear display since title is the first piece of data sent for each new song
        lcd.setCursor(0,0);
        lcd.print(title); //print on first line of display, will wait to scroll
        //singleScroll(title, 0);
      }
      else if(line.indexOf(artistFind) >= 0){ //if we find the notification for artist
        artist = line.substring(21); //assign to artist var
        Serial.print(artist); //print for monitoring
        lcd.setCursor(0,1);
        lcd.print(artist); //print artist on second line
      }
      line = ""; 
      rec = 0;
      lines++; //iterate through while loop while setting local vars to default values
    }
    lcd.setCursor(0,1);
    lcd.print(artist); //print artist
    singleScroll(title, 0); //scroll through title if necessary
    }
    
}
