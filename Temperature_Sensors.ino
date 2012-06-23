#include <OneWire.h>
#include <LiquidCrystal.h>
#include <dht11.h> // I'll copy this over eventually
dht11 DHT11;
#define DHT11PIN 4

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
// DS18S20 Temperature chip i/o
OneWire ds(2);  // on pin 10

uint8_t degree[8]  = {140,146,146,140,128,128,128,128};

byte ds_addr[8];

void setup(void) {
  // initialize inputs/outputs
  // start serial port
  Serial.begin(9600);
  lcd.createChar(0, degree);
  lcd.begin(16,2);
  lcd.print("    Thermal");
  lcd.setCursor(0,1);
  lcd.print("  Comparison");
  //  lcd.print((char)0);
  delay(1000);
  lcd.clear();
}

void loop(void) {
  Serial.println("----------------------------------------");
  // Get the DS18B20 temp
  RequestDS18B20Temp();
  delay(750); // it takes a miminum of 750ms to return a temperature...
  int temp = GetDS18B20Temp();
  float ftemp = ( (float) temp / 100);
  ftemp = (ftemp * 1.8) + 32;
  Serial.print(" DS18B20 Temp F: ");
  Serial.println(ftemp);
  lcd.setCursor(0,0);
  lcd.print("D:");
  lcd.print(ftemp);
  
  // MCP9700A temperature
  float mcp9temp = analogRead(0)*5/1024.0;
  mcp9temp = mcp9temp - 0.5;
  mcp9temp = mcp9temp / 0.01;
  mcp9temp = (mcp9temp*1.8)+32;
  Serial.print("MCP9700A Temp F: ");
  Serial.println(mcp9temp);
  lcd.setCursor(9,0);
  lcd.print("M:");
  lcd.print(mcp9temp);

  // DHT11 Temperature
  int chk = DHT11.read(DHT11PIN);
  if(chk == 0) {
    int temperature = (1.8*DHT11.temperature+32);
    int humidity = DHT11.humidity;
        
    lcd.setCursor(0,1);
    lcd.print("T:"); lcd.print(temperature); lcd.print((char)0); lcd.print(humidity); lcd.print("%");

    Serial.print("   DHT11 Temp F: "); Serial.print(temperature, DEC);
    Serial.print(" Humidity: "); Serial.println(humidity, DEC);
  }

  // LM35 Temperature
  float lmtemp = analogRead(2)*5/1024.0;
  lmtemp = lmtemp / 0.01;
  lmtemp = (lmtemp*1.8)+32;
  Serial.print("  LM35DZ Temp F: ");
  Serial.println(lmtemp);

  lcd.setCursor(9,1);
  lcd.print("L:");
  lcd.print(lmtemp);

}


void RequestDS18B20Temp(void) {
  if(!ds.search(ds_addr)) {
    ds.reset_search();
  }

  if ( OneWire::crc8( ds_addr, 7) != ds_addr[7]) {
    return;
  }

  ds.reset();
  ds.select(ds_addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
}

int GetDS18B20Temp(void) { 
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  byte data[12];


  ds.reset();
  ds.select(ds_addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( byte i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  LowByte = data[0]; HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) { // negative
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  //  Tc_100 = (Tc_100*1.8)+3200;
  //  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  //  Fract = Tc_100 % 100;

  return(Tc_100); 
}
