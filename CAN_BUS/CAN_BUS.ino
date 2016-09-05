//Equipe 4 - Erick Modesto Campos, Daniel Breno Guiomarino Menezes, Suzane Santos dos Santos, Israel Lucas Barros de Amorim, Rodrigo Santos do Amor Divino Lima

/*
Se vocês quiserem, podem usar essa mesma lógica.
Não simplesmente dêem crtl+c crtl+v, porque esse código
foi todo feito voltado para o can da minha equipe: configurações, 
cálculos, Sensor...
*/

#include <Thermistor.h>
#include <math.h>
#include <SPI.h>
#include "mcp_can.h"
const int SPI_CS_PIN = 9;
//MCP_CAN CAN(SPI_CS_PIN);       
char mensagem[44];
unsigned char enviar0[8]={ 'm','e','n','s','a','g','e','m'}; 
unsigned char enviar1[8]={'m','e','n','s','a','g','e','m'};
unsigned char enviar2[8]={'m','e','n','s','a','g','e','m'};
unsigned char enviar3[8]={'m','e','n','s','a','g','e','m'};
unsigned char enviar4[8]={'m','e','n','s','a','g','e','m'};
unsigned char graus[6] = {' ', 'g','r','a','u','s'}; 
unsigned char sensor[8];
unsigned char estadoLed[8];
unsigned char estadoLed1[2];
unsigned char padrao[1]={(char)0};
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char receber[44];
char dezena, unidade;
int cont=0;
void setup(){
  Serial.begin(115200);
START_INIT:
  if(CAN_OK == CAN.begin(CAN_500KBPS)){
    Serial.println("CAN BUS Shield init ok!");
  }
  else{
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
    goto START_INIT;
  }       
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  estadoLed[0]='d';
  estadoLed[1]='e';
  estadoLed[2]='s';                        // desliga o led
  estadoLed[3]='a';
  estadoLed[4]='t'; 
  estadoLed[5]='i'; 
  estadoLed[6]='v'; 
  estadoLed[7]='a'; 
  estadoLed1[0]='d'; 
  estadoLed1[1]='o'; 

  attachInterrupt(0, MCP2515_ISR, FALLING);

  CAN.init_Mask(0, 0, 0x00F);
  CAN.init_Mask(1, 0, 0x00F);
  CAN.init_Filt(0, 0, 0x004);
  CAN.init_Filt(1, 0, 0x000);

}

void MCP2515_ISR(){
  flagRecv = 1;
}

void loop(){
  
  
/*********************************************************************************************************
  Inicio - Enviar mensagem
*********************************************************************************************************/
  for(int i=0; i<43; i++){
    if(Serial.available()>0){
      mensagem[i]=Serial.read();
      delay(1);                           // lê a mensagem que será digitada no serial monitor
    }
    else{
      mensagem[i]=(char)0;
    }
  }
  while(Serial.available()>0)Serial.read();

  int a = ((char)mensagem[1]-48);
  int b = (char)mensagem[2]-48;
  int meudestino = (a*10)+b;             // identifica qual será o nó destino
  int MEU_ID = (4<<4)+meudestino;       // monta o frame: xxxEEEDDDD, EEEE: nó emissor (nosso nó) , DDDD: nó destino, xxx: irrelevante 

  if(mensagem[0]!=(char)0){
    if(mensagem[0]=='#' && mensagem[3]=='#'){
      for(int i=0; i<8; i++){
        enviar0[i]=mensagem[i+4];
        enviar1[i]=mensagem[i+12];             // Separa a mensagem a ser enviada de 8 em 8 bytes
        enviar2[i]=mensagem[i+20];
        enviar3[i]=mensagem[i+28];
        enviar4[i]=mensagem[i+36];
      }
    }
    else if(mensagem[0]=='#' && mensagem[3]=='%'){
      for(int i=0; i<8; i++){
        enviar0[i]=mensagem[i+3];
        enviar1[i]=mensagem[i+11];                     // Separa a mensagem a ser enviada de 8 em 8 bytes
        enviar2[i]=mensagem[i+19];
        enviar3[i]=mensagem[i+27];
        enviar4[i]=mensagem[i+35];
      }
    }
    Serial.println("-----------------------------------------------------------------------------------");
    Serial.println(mensagem);
    Serial.println("-----------------------------------------------------------------------------------");
    CAN.sendMsgBuf(MEU_ID, 0, 8, enviar0);
    delay(100);                       // send data per 100ms
    CAN.sendMsgBuf(MEU_ID, 0, 8, enviar1);
    delay(100);                       // send data per 100ms
    CAN.sendMsgBuf(MEU_ID, 0, 8, enviar2);
    delay(100);                       // send data per 100ms                            // Envia a mensagem
    CAN.sendMsgBuf(MEU_ID, 0, 8, enviar3);
    delay(100);                       // send data per 100ms
    CAN.sendMsgBuf(MEU_ID, 0, 8, enviar4);
    delay(100);                       // send data per 100ms
  }
  for(int i=0; i<44; i++){
    mensagem[i]=(char)0;  /// preenche novamente o vetor mensagem com valores nulos
  }

  for(int i=0;i<8;i++){
    enviar0[i]=(char)0;
    enviar1[i]=(char)0;
    enviar2[i]=(char)0;
    enviar3[i]=(char)0;
    enviar4[i]=(char)0;
  }
  
  
/*********************************************************************************************************
  Fim - Enviar mensagem
*********************************************************************************************************/

  
 
/*********************************************************************************************************
  Inicio - Receber mensagem
*********************************************************************************************************/ 
  
  
  if(flagRecv) {
    flagRecv = 0;                // clear flag
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    for(int i = 0; i<len; i++){
      receber[i+cont]=(char)buf[i];
    }
    cont+=8;
  }


  int OUTRO_ID = CAN.getCanId();           // Pega o ID contendo o frame de outro nó, caso ele seja filtrado corretamente          
  int No_Emissor = (OUTRO_ID  & 240)>>4;  //faz uma operação AND do ID com 11110000. Pega o resultado da operação AND e desloca 4 bits para direita para obter como resultado o nó emissor da mensagem
  // int No_destino = OUTRO_ID & 15;        // Obtém o nó destino do id de outro nó

  if(No_Emissor>9 & No_Emissor<16){
    dezena = (char)49;
    unidade = (char)(48+(No_Emissor - 10));
  }
  else if(No_Emissor<=9){ 
    dezena = (char)48;
    unidade = (char)(No_Emissor+48);
    
  int MEU_ID2 = (4<<4)+No_Emissor; // Id caso uma solicitação de informação sobre a carga ou sensor


    if(cont>=39){
      if(receber[0]=='%'){
        if(receber[1]=='1'){
          digitalWrite(7, HIGH);                         
          estadoLed[0]='a';
          estadoLed[1]='t'; 
          estadoLed[2]='i'; 
          estadoLed[3]='v'; 
          estadoLed[4]='a';   // liga o led
          estadoLed[5]='d'; 
          estadoLed[6]='o'; 
          estadoLed[7]=(char)0;
          estadoLed1[0]=(char)0;
          estadoLed1[1]=(char)0;
        }     
        else if(receber[1]=='0'){
          digitalWrite(7, LOW);
          estadoLed[0]='d';
          estadoLed[1]='e';
          estadoLed[2]='s';                        // desliga o led
          estadoLed[3]='a';
          estadoLed[4]='t'; 
          estadoLed[5]='i'; 
          estadoLed[6]='v'; 
          estadoLed[7]='a'; 
          estadoLed1[0]='d';
          estadoLed1[1]='o';
        }
        else if(receber[1]=='?') {
          Serial.println("\n------------------------------------------------------------------"); 
            CAN.sendMsgBuf(MEU_ID2, 0, 8, estadoLed);
          delay(100);                       // send data per 100ms
          CAN.sendMsgBuf(MEU_ID2, 0, 8, estadoLed1);
          delay(100);  
          CAN.sendMsgBuf(MEU_ID2, 0, 8, enviar1);     // Envia o estado do led
          delay(100);  
          CAN.sendMsgBuf(MEU_ID2, 0, 8, enviar2);
          delay(100);  
          CAN.sendMsgBuf(MEU_ID2, 0, 8, enviar3);
          delay(100);       
        }
      }
      if(receber[0]==(char)0){
        int X = int(Termistor(analogRead(0)));  // le o valor do sensor
        char sensor_dezena = ((int)(X/10))+48;
        char sensor_unidade = ((int)(X%10))+48;
        Serial.print("#");
        Serial.print(dezena);
        Serial.print(unidade);              // Mostra a mensagem
        Serial.print("#");                 // que será enviada
        Serial.print(X);
        Serial.println(" graus");
        Serial.println("\n------------------------------------------------------------------");
        sensor[0]=sensor_dezena;
        sensor[1]=sensor_unidade;
        sensor[2]=' ';
        sensor[3]='g';
        sensor[4]='r';
        sensor[5]='a';
        sensor[6]='u';
        sensor[7]='s';     
        CAN.sendMsgBuf(MEU_ID2,0,8,sensor);       
        delay(100);
        CAN.sendMsgBuf(MEU_ID2,0,8,enviar0);    // Envia o valor do  lido pelo sensor
        delay(100);
        CAN.sendMsgBuf(MEU_ID2,0,8,enviar1);
        delay(100);
        CAN.sendMsgBuf(MEU_ID2,0,8,enviar2);
        delay(100);
        CAN.sendMsgBuf(MEU_ID2,0,8,enviar3);
        delay(100);
      }
         Serial.println(receber);
      cont=0;  
    } 
    delay(20);
  }

/*********************************************************************************************************
  Fim - receber mensagem
*********************************************************************************************************/

}
 


double Termistor (int RawADC)
{
  double Temp;
  Temp = log(((10240000/RawADC) - 10000)); // Considerando resistência de 10K
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );    // Função que converte o valor lido pelo sensor para graus celsius
  // Equação de Steinhart-Hart para o termistor - temperatura em Kelvin

  Temp = Temp - 273.15; // Converte Kelvin para Celsius 
  return Temp; 
}



/********************************************************************************************************
  END FILE
*********************************************************************************************************/




