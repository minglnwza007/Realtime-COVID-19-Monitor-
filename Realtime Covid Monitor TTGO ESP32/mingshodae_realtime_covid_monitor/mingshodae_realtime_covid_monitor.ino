#include <BlynkGOv2.h> //MINGSHODAE LNWZA007 
#include <HTTPClient.h>
#include <ArduinoJson.h> // v6.xxx

//TTGO ESP32
#define SSID        "Ming123ZA_2.4G" // ชื่อ wifi
#define PASSWORD    "12345678" // รหัส wifi

FONT_DECLARE(sukhumvit_40);
FONT_DECLARE(sukhumvit_35);
FONT_DECLARE(sukhumvit_30);
FONT_DECLARE(sukhumvit_25);
FONT_DECLARE(sukhumvit_20);
GMultiScreen multiscreen;
GWiFiManager wifi_manager;

GLabel lb_title;
GLabel lb_new_case;
GLabel lb_new_recovered;
GLabel lb_new_death;



GLabel lb_clock;
GLabel lb_date;

GTimer timer_covid19, timer_multiscreen , timer_multiscreen_autoloop;
String json_str;

void    graphic_design();
String  http_GET(String url);

void multiscreen_autoloop(){
    timer_multiscreen_autoloop.setInterval(5000, [](){  // ปรับเวลาเปลี่ยนหน้าจอ 1000 = 1 วินาที่ 
        if( multiscreen.current_screen_id() < multiscreen.num()-1) {
            multiscreen.current_screen( multiscreen.current_screen_id()+1);
        }else{
            multiscreen.current_screen( 0, false);
        }
    });
}

void multiscreen_autoloop_stop(){
    timer_multiscreen_autoloop.del();
}

void setup(){
    Serial.begin(115200); Serial.println();
    BlynkGO.begin();
    graphic_design();

    WiFi.begin(SSID, PASSWORD);

    timer_covid19.setInterval(5000,[](){
        json_str = http_GET("https://covid19.ddc.moph.go.th/api/Cases/today-cases-all"); // ดึงข้อมูล 1000 = 1 วินาที
    });

    multiscreen_autoloop();
}

/*

 [{"txn_date":"2022-02-06","new_case":10879,"total_case":2497001,"new_case_excludeabroad":10701,"total_case_excludeabroad":2480674,"new_death":20,"total_death":22291,"new_recovered":8285,"total_recovered":2383673,"update_date":"2022-02-06 07:24:48"}]

*/


void loop(){
    BlynkGO.update();

    if( json_str != "") {
        StaticJsonDocument<1024> json;
        auto error = deserializeJson(json, json_str);
        json_str = "";

        if(!error){
            lb_new_case       = StringX::printf("ผู้ติดเชื้อรายใหม่  : %d", json[0]["new_case"].as<int>());
            lb_new_death      = StringX::printf("เสียชีวิตเพิ่ม        : %d",  json[0]["new_death"].as<int>());
            lb_new_recovered  = StringX::printf("หายป่วยวันนี้       : %d", json[0]["new_recovered"].as<int>());
        }
    }
}


// WIFI_CONNECTED(){
//     rect.color(TFT_GREEN);
// }

// WIFI_DISCONNECTED(){
//     rect.color(TFT_RED);
// }

NTP_SYNCED(){
    static GTimer timer;
    timer.setInterval(1000,[](){
        lb_date = StringX::printf("%02d:%02d:%02d", day(), month(), year() ); // 0
        lb_clock = StringX::printf("%02d:%02d", hour(), minute() ); // 23:32:05
        
    });
}



String http_GET(String url){
  if(!WiFi.isConnected()) return "";
  std::unique_ptr<WiFiClientSecure>client (new WiFiClientSecure);
  if(client) {
    client->setInsecure();  // สำหรับ WiFiClientSecure  443 แบบไม่ต้องเช็ค secure
    {
      HTTPClient http;
      http.begin(*client, url);
      int http_code = http.GET();
    //   Serial.println(http_code);
      if( http_code == 200) {
        String payload = http.getString();
        return payload;
      }
    }
  }
  return String("");
}



void graphic_design(){
    multiscreen.addScreen(2);
    
        
        lb_new_case.parent( multiscreen[1]);
        lb_new_recovered.parent(multiscreen[1]);
        lb_new_death.parent(multiscreen[1]);
        lb_new_case = "ผู้ติดเชื้อรายใหม่";
        lb_new_recovered = "หายป่วยวันนี้";
        lb_new_death = "เสียชีวิตเพิ่ม";
        lb_new_case.align(lb_new_recovered,ALIGN_TOP);
        lb_new_death.align(lb_new_recovered,ALIGN_BOTTOM);
        lb_new_case.color(TFT_BLUE);
        lb_new_recovered.color(TFT_GREEN);
        lb_new_death.color(TFT_RED);

        lb_title.parent(multiscreen[0]);
        lb_title = "ประเทศไทย";
        lb_title.font(sukhumvit_40);
        lb_title.align(ALIGN_TOP);
        lb_title.color(TFT_CYAN);
        wifi_manager.parent( multiscreen[0]);  // 0, 1, 2  ---> 3 หน้า
        wifi_manager.align(ALIGN_BOTTOM);
        lb_date.parent(multiscreen[0]);
        lb_date.font(sukhumvit_25);
        lb_date.align(ALIGN_TOP,0,35);
        lb_date.color(TFT_CYAN);
        lb_clock.parent(multiscreen[0]);
        lb_clock.font(sukhumvit_25);
        lb_clock.align(ALIGN_BOTTOM,0,-40);
        lb_clock.color(TFT_CYAN);

    BTN2.onPressed([](ButtonISR* btn) {
        if( multiscreen.current_screen_id() < multiscreen.num()-1) {
            multiscreen.current_screen( multiscreen.current_screen_id()+1);
        }else{
            multiscreen.current_screen( 0, false);
        }
        multiscreen_autoloop_stop();
        timer_multiscreen.setOnce(10000, multiscreen_autoloop );
    });
}
