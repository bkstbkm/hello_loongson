#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Servo.h>
#include <math.h>

Servo O;  // 电磁阀
Servo P;  // 真空泵

// WiFi配置
String STA_SSID = "SepMHz的nova 11 SE"; // 默认连接的路由器SSID
String STA_PSK = "1234876590";          // 默认连接的路由器密码

// 网络服务
DNSServer DNS;
ESP8266WebServer Web(80);
ESP8266HTTPUpdateServer Updater;
WiFiServer server(8000);  // TCP服务器端口
WiFiClient C;             // TCP客户端
WiFiUDP U;                // UDP对象
int UPort = 8888;         // UDP端口

// 机械臂配置
Servo S[6];               
int ss = 4;               
String XYZE[6] = {"X","Y","Z","E","O","P"}; 
int Pin[6] = {D2, D3, D0, D1, D5, D6};     
int start[6] = {90, 90, 180, 90, 0, 0};       
float current[6] = {90, 90, 180, 90, 0, 0};     
float target[6] = {90, 90, 180, 90, 0, 0};     
int Min[6] = {0, 46, 60, 0, 0, 0};         
int Max[6] = {180, 180, 180, 180, 180, 180}; 
unsigned long end = 0;  
bool maxdelay = false;       

// 机械臂参数
float YZmin = 225.0;      
float YZmax = 336.0;      
int maxtime = 1500;                     
float speed = 1.0;                      

// 命令处理
String Cmd = "", Cmdret = "";          
bool sync = true;                      
unsigned long delaytime = 0;           

String Command(String a, bool b=false);
String CheckCmd(String S);
String getFirstCmd();
String output();
void loadConfig();
void handleUserRequest();
void handleFileUpload();
void FileUpload_OK();
String getType(String filename);
bool Config();

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    SPIFFS.begin();
    loadConfig();
    
    // 舵机初始化
    for(int I=0; I<ss; I++) {
        S[I].attach(Pin[I], 500, 2500);
        current[I] = start[I] * 1.0;
        target[I] = start[I] * 1.0;
        S[I].write(start[I]);
    }
    if(Max[3] == 180) { 
        // 电磁阀和真空泵初始化
        O.attach(Pin[4], 500, 2500); 
        O.write(0);
        P.attach(Pin[5], 500, 2500);
        P.write(0);
    }
    
    // 连接路由器
    if(STA_SSID != "" && STA_PSK != "") {
        Serial.printf("\n连接WiFi: %s\n", STA_SSID.c_str());
        WiFi.begin(STA_SSID, STA_PSK);
        
        for(int i=0; i<15; i++) {
            if(WiFi.status() == WL_CONNECTED) break;
            Serial.print(".");
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(500);
        }
        
        if(WiFi.status() == WL_CONNECTED) {
            Serial.print("\nIP地址: ");
            Serial.println(WiFi.localIP());
        }
    }
    
    // 启动TCP/UDP服务器
    server.begin();
    U.begin(UPort);
    Serial.printf("TCP端口: %d, UDP端口: %d\n", 8000, UPort);
    
    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    DNS.processNextRequest();
    Web.handleClient();

    // 处理延时状态
    if (delay) {
        if (millis() >= end) {
            maxdelay = false;
        } else {
            return;  
        }
    }
    
    // 处理TCP连接
    if(!C || !C.connected()) {
        C = server.available();
        if(C) Serial.println("新TCP客户端连接");
    } else if(C.available()) {
        String S = C.readStringUntil(';');
        if(S.startsWith("GET") || S.startsWith("POST")) {
            C.stop();
            Serial.println("TCP客户端断开");
        } else {
            Serial.println("TCP数据: " + S);
            String ret = CheckCmd(S);
            C.print(ret);
        }
    }
    
    // 处理UDP数据
    if(U.parsePacket()) {
        char data[255];
        int len = U.read(data, 255);
        if(len > 0) {
            data[len] = '\0';
            Serial.print("UDP数据: ");
            Serial.println(data);
            String ret = CheckCmd(String(data));
            U.beginPacket(U.remoteIP(), U.remotePort());
            U.print(ret);
            U.endPacket();
        }
    }
    
    // 处理串口输入
    if(Serial.available() > 0) {
        String S = Serial.readString();
        Serial.println(S);
        Serial.println(CheckCmd(S));
    }
    
    // 处理命令队列
    if(millis() > delaytime && Cmd.length() > 0) {
        Cmdret = getFirstCmd();
    }
}

// 加载配置文件
void loadConfig() {
    File F = SPIFFS.open("/config.json", "r");
    if(!F) return;
    
    String S = F.readString();
    F.close();
    
    if(S.length() > 32) {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, S);
        
        STA_SSID = String(doc["STA_SSID"]);
        STA_PSK = String(doc["STA_PSK"]);
        
        doc.clear();
    }
}

// 处理用户请求
void handleUserRequest() {
    String url = Web.uri();
    
    if(url == "/cmd") {
        if(Web.args() == 0) {
            Web.send(200, "text/plain", output());
        } else {
            String message = "";
            for(int i=0; i<Web.args(); i++) {
                String cmd = Web.argName(i);
                if(cmd == "cmd") {
                    message = CheckCmd(Web.arg(i));
                }
            }
            Web.send(200, "text/plain", message);
        }
    } else if(SPIFFS.exists(url)) {
        File file = SPIFFS.open(url, "r");
        Web.streamFile(file, getType(url));
        file.close();
    } else {
        Web.send(404, "text/plain", "404 Not Found");
    }
}

// 处理文件上传
void handleFileUpload() {
    static File F;
    HTTPUpload& UP = Web.upload();
    
    if(UP.status == UPLOAD_FILE_START) {
        String filename = UP.filename;
        if(!filename.startsWith("/")) filename = "/" + filename;
        F = SPIFFS.open(filename, "w");
    } else if(UP.status == UPLOAD_FILE_WRITE) {
        if(F) F.write(UP.buf, UP.currentSize);
    } else if(UP.status == UPLOAD_FILE_END) {
        F.close();
    }
}

// 文件上传完成
void FileUpload_OK() {
    Web.send(200, "text/html", "<h1>上传成功</h1>");
}

// 获取文件类型
String getType(String filename) {
    if(filename.endsWith(".html")) return "text/html";
    else if(filename.endsWith(".css")) return "text/css";
    else if(filename.endsWith(".js")) return "application/javascript";
    else if(filename.endsWith(".txt")) return "text/plain";
    return "text/plain";
}

// 检查并执行命令
String CheckCmd(String S) {
    S.trim();
    S.replace("；", ";");
    
    if(S.indexOf(";") > 0 || S.indexOf("\n") > 0) {
        Cmd += S + "\n";
        return output();
    } else {
        return Command(S, true);
    }
}

// 从命令队列中获取第一条命令
String getFirstCmd() {
    if(Cmd.length() < 1) return "";
    Cmd.trim();
    
    int i = Cmd.indexOf(";");
    int j = Cmd.indexOf("\n");
    String S = "";
    
    if(i == -1 && j == -1) {
        S = Cmd;
        Cmd = "";
    } else if(i != -1 && j == -1) {
        S = Cmd.substring(0, i);
        Cmd.remove(0, i+1);
    } else if(i == -1 && j != -1) {
        S = Cmd.substring(0, j);
        Cmd.remove(0, j+1);
    } else if(i < j) {
        S = Cmd.substring(0, i);
        Cmd.remove(0, i+1);
    } else {
        S = Cmd.substring(0, j);
        Cmd.remove(0, j+1);
    }
    
    String ret = Command(S);
    if(ret == "") ret = output();
    return ret;
}

// 执行命令
String Command(String t, bool CheckCmd) {
    String S[5] = {"","","","",""};
    int i = 0, j = 0;
    
    // 拆分命令和参数
    for(i=0; i<5; i++) {
        t.trim();
        if(t == "") break;
        if(i == 0) j = t.indexOf(" ");
        else j = t.indexOf(",");
        S[i] = t.substring(0, j);
        S[i].trim();
        if(j == -1) { t = ""; break; }
        t = t.substring(j+1);
    }
    
    // 处理基本命令
    if(S[0] == "?") {
        return output();
    } else if(S[0] == "H") {
        for(int I=0; I<ss; I++) {
            ServoGo(I, start[I]);
        }
        return output();
    } else if(S[0] == "sync") {
        sync = (S[1].toInt() != 0);
        return "";
    } else if(S[0] == "1") {
        Cmd = "Y 115;Z 160;delay 500;P 3000;delay 500;H;delay 1000;X 140;delay 1000;O 1000;H;" + Cmd;
        return "执行预设动作: Y115;Z160";
    } else if(S[0].equalsIgnoreCase("delay")) {  
        unsigned long delayTime = S[1].toInt();
        if(delayTime > 0) {
            end = millis() + delayTime;
            maxdelay = true;
            return "Delay " + String(delayTime) + "ms";
        }
        return "Invalid delay time";
    }

    if(S[0].equalsIgnoreCase("O")) {             // 电磁阀控制
        if(S[1].toInt() > 10000) S[1] = "10000"; // 最大进气时间为10秒
        if(S[1].toInt() != 0) {                  
            P.write(0);                          
            delay(10);
            O.write(180);                        
            if(S[1].toInt() > 0) {               
                if(S[1].toInt() < 800) S[1] = "800";
                delaytime = millis() + S[1].toInt();   // 插入延时
                Cmd = "O 0\n" + Cmd;             
            }
        } else {
            O.write(0);                          
        }
        delay(10);
        return "O " + String(S[1]);
    }

    if(S[0].equalsIgnoreCase("P")) {             // 真空泵控制
        if(S[1].toInt() > 30000) S[1] = "30000"; 
        if(S[1].toInt() != 0) {                  // 非0 开泵
            O.write(0);                          
            delay(10);
            P.write(180);                        // 开泵
            if(S[1].toInt() > 0) {               
                delaytime = millis() + S[1].toInt();   // 插入延时
                Cmd = "P 0\n" + Cmd;             
            }
        } else {
            P.write(0);                         
        }
        delay(10);
        return "P " + String(S[1]);
    }
    
    // 处理舵机命令
    for(int I=0; I<ss; I++) {
        if(S[0].equalsIgnoreCase(XYZE[I])) {
            float f = 0.0;
            if(S[1] == "++") f = target[I] + 1.0;
            else if(S[1] == "--") f = target[I] - 1.0;
            else if(S[1].startsWith("+")) f = target[I] + S[1].substring(1).toFloat();
            else if(S[1].startsWith("-")) f = target[I] - S[1].substring(1).toFloat();
            else f = S[1].toFloat();
            
            ServoGo(I, f);
            return output();
        }
    }
    
    if(CheckCmd) {
        Cmd += S[0] + " " + t + ";";
        return output();
    }
    
    return "未知命令: " + S[0];
}

void Servo4(int I){                   
    float v=target[1]+target[2];            

    if(I==1){                        
      if(v<YZmin){                     
         target[2]+=YZmin-v;
         if(target[2]>Max[2]) target[1]+=target[2]-Max[2];
      }   
      if(v>YZmax)                     
         target[2]-=v-YZmax;
         if(Min[2]>target[2]) target[1]-=Min[2]-target[2];
    } 
    if(I==2){                        
     if(v<YZmin)                       
         target[1]+=YZmin-v; 
         if(target[1]>Max[1]) target[2]+=target[1]-Max[1];
     if(v>YZmax)                       
         target[1]-=v-YZmax;
         if(Min[1]>target[1]) target[2]-=Min[1]-target[1];
    }
}

bool ServoGo(int I,float Value) {                 
  int MAX=0;

  if(I>=0 && I<ss){               
    target[I]=constrain(Value,Min[I],Max[I]);         
    Servo4(I);                            
  } 


  if(speed>0.0)speed=1.0;                                    
  for(I=0;I<ss;I++){MAX=max(MAX,(int)abs(target[I]-current[I]));} 
  float p[6]={0.0,0.0,0.0,0.0};
  for(I=0;I<ss;I++){p[I]=abs(target[I]-current[I])/MAX*speed;}    
  unsigned long ms=millis();                              
  if(speed>0.0){
    for(int J=0;J<(MAX/speed);J++){             
      for(I=0;I<ss;I++){
       if(current[I]==target[I]) continue;            
         if(current[I]<target[I]) {             
           current[I]+=p[I];                        
        } else {
           current[I]-=p[I];                        
        }
        S[I].writeMicroseconds(toPWM(I,current[I]));           
      }
        
      yield(); 
    }
    Serial.printf("耗时:%d ms\n",millis()-ms);  
    delay(16);
  }

  for(I=0;I<ss;I++){                        
    S[I].writeMicroseconds(toPWM(I,target[I])); 
    current[I]=target[I];                           
  }
  ms+=MAX*4;             
  if(ms>millis()){       
     delay(constrain(ms-millis(),0,maxtime));
  }
  return MAX!=0;         
}

// PWM控制
int toPWM(int I, float Value) {
    int type = 180;
    if(I >= 0 && I < ss) {
        Value = constrain(Value, Min[I], Max[I]);
        if(Max[I] == 270) type = 270;
    } else {
        Value = constrain(Value, 0, 180);
    }
    Value = (2000/type) * Value + 500;
    return (int)Value;
}

// 输出当前状态
String output() {
    DynamicJsonDocument doc(256);
    for(int I=0; I<ss; I++) {
        doc[XYZE[I]] = String(target[I], 1);
    }
    doc["Cmd"] = Cmd.length();
    
    String ret;
    serializeJson(doc, ret);
    doc.clear();
    return ret;
}