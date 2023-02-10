#include <LiquidCrystal_I2C.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WiFiMulti WiFiMulti;

const char* ssid     = "Dialog 4G 354";
const char* password = "94b00cd1";

auto host = "http://192.168.8.1/goform/goform_get_cmd_process?isTest=false&cmd=system_status";

uint64_t session = 0;

uint64_t char2LL(const char *str)
{
  uint64_t result = 0;
  
  for (int i = 0; str[i] != '\0'; ++i)
    result = result * 10 + str[i] - '0';
  return result;
}

void setup()
{
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();

  lcd.print("Connecting");

  for (uint8_t t = 4; t > 0; t--)
  {
    lcd.print(".");
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop()
{
  lcd.clear();

  if (WiFiMulti.run() == WL_CONNECTED)
  {
    WiFiClient client;
    HTTPClient http;

    if (http.begin(client, host))
    {
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();

        int index = payload.indexOf("downlink_traffic") + 19;
        byte a;

        for (a = 0; a < 255; a++)
        {
          if (payload[index + a] == '"') break;
        }

        uint64_t total = char2LL(payload.substring(index, index + a).c_str());

        index = payload.indexOf("uplink_traffic") + 17;

        for (a = 0; a < 255; a++)
        {
          if (payload[index + a] == '"') break;
        }

        total += char2LL(payload.substring(index, index + a).c_str());
        if(session == 0) session = total;
        uint64_t tmp_total = total;

        lcd.print("Tot ");

        float data = total;

        if(data > 1024 * 1024 * 1024)
        {
            lcd.printf("%.2f GB", data / 1024 / 1024 / 1024);
        }
        else if(data > 1024 * 1024)
        {
            lcd.printf("%.2f MB", data / 1024 / 1024);
        }
        else if(data > 1024)
        {
            lcd.printf("%.2f KB", data / 1024);
        }
        else
        {
            lcd.printf("%.2f B", data);
        }

        lcd.setCursor(0, 1);

        lcd.print("Cur ");
        
        total = tmp_total - session;

        data = total;

        if(data > 1024 * 1024 * 1024)
        {
            lcd.printf("%.2f GB", data / 1024 / 1024 / 1024);
        }
        else if(data > 1024 * 1024)
        {
            lcd.printf("%.2f MB", data / 1024 / 1024);
        }
        else if(data > 1024)
        {
            lcd.printf("%.2f KB", data / 1024);
        }
        else
        {
            lcd.printf("%.2f B", data);
        }
      }

      http.end();
    }
  }
  else
  {
    lcd.print("Reconnecting..");
  }

  delay(2500);
}
