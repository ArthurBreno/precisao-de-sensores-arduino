/*
  codigo para mini datalogger
  portas utilizadas pelos borns:
  born 1 - digital 3 - Ship
  born 2 - Analogico 0 - RTC
  born 3 - Analogico 2 - "antes"
  born 4 - Analogico 1 - "depois"
 IMPORTANTE - Todos os borns seguem o mesmo padrão de pinagem: da esquerda para a direita olhando para o buraco que bota o fio:
  conecção 1 - Vcc (todos com 5 volts)
  conecção 2 - Data (borns 1,3 e 4 concetados a porta de dados Digital, born 2 conectado a porta de dados analogica)
  conecção 3 - Gnd (todos os GNDs da placa estão conectados entre si então tanto faz)
*/

/*
CABEÇALHO SOBRE CALCULO DA UMIDADE RELATIVA
definicao 
ts - temperatura do bulbo seco
tu - temperatura do bulbo umido
ea - pressao atual de vapor
esTU - pressao de vapor saturado do bulbo umido
esTS - pressao de vapor saturado do bulbo seco
ur - umidade relativa

y - 8.0 * 10 ^-4
pATM - 0.94 * 10 ^5
ea = esTU - (y x pATM x (Ts - Tu))
esTS = a x 10 ^ ((7.5 * ts) / (237.3 + ts)
esTU = a x 10 ^ ((7.5 * tu) / (237.3 + tu)
ur = ea / esTS

referencia: http://www.leb.esalq.usp.br/leb/aulas/lce200/Cap6.pdf

ordem de calculo	- calcular esTU
					        - calcular esTS
      			    	- calcular ea
                  - calcular ur
*/

#include <SPI.h>  // Biblioteca de comunicação SPI Nativa
#include <SD.h>   // Biblioteca de comunicação com cartão SD Nativa
#include <DHT.h>  //Incluindo a biblioteca DHT
#include <Wire.h>
#include <TimeLib.h>    // biblioteca auxiliar para o RTC
#include <DS1307RTC.h>  // bilbioteca correta para para o RTC
#include <OneWire.h>    // BIBLIOTECA AUXILIAR DO DALLAS
#include <DallasTemperature.h>

//------------------------------------------------------
//#define DHT22pino 3
//#define ONE_WIRE_BUS 3

//------------------------------------------------------
//DHT dhtP22(DHT22pino, DHT22);

//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

//------------------------------------------------------
float vetorDados[50];
float var = 0;
File dataFile;
uint8_t contadorErros = 0;
float mediaFinal = 0;
float amplitudadeFinal = 0;
float desvioPadraoFinal = 0;
float tempBulboSeco = 0;
float tempBulboUmido = 0;

//-----------------------------------------------------

void setup() {
  // pinod digitais - positivos - A1/5 - A2/6 ---- negativos - A1/7 - A2/8
  pinMode(3, OUTPUT);
  //
  digitalWrite(3, HIGH);
  //
  Serial.begin(9600);
  delay(1000);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  Serial.println(1);
  //------------------------------------------------------
  //dhtP22.begin();
  //sensors.begin();
  delay(100);
  Serial.println(2);
  SD.begin(10);
  delay(100);
  dataFile = SD.open("test.txt", FILE_WRITE);
  delay(100);
  if (dataFile) {
    dataFile.println("init");
    dataFile.close();
  }
  delay(100);
  //------------------------------------------------------
}
void loop() {
  SD.begin(10);
  //gravar hora no SD
  dataFile = SD.open("test.txt", FILE_WRITE);
  tmElements_t tm;
  if (dataFile) {
    Serial.println("v hora");
    if (RTC.read(tm)) {
      String a = String(tm.Day) + "/" + String(tm.Month) + "/" + String(tmYearToCalendar(tm.Year)) + " " + String(tm.Hour) + ":" + String(tm.Minute) + ",";
      dataFile.print(a);
      dataFile.close();
      delay(500);
    }
  } else {
    Serial.print("F hora");
  }
  //FIM GRVAR HORA NO SD
  Serial.println(3);
  //-----------------------------------------------------------------------------
  /* contadorErros = 20;
  var = 0;
  Serial.println(4);
  // definir loop para dhtTemp
  for (uint8_t i = 0; i < 30; i++) {
    var = dhtP22.readTemperature();
    if (isnan(var) || var <= 1) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
    delay(2000);
  }
  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    Serial.println("v dhtTemp");
  } else {
    Serial.println("F dhtTemp");
  }

*/
  //------------------------------------------------------------------------------
  /*
  contadorErros = 20;
  var = 0;
  Serial.println(4);
  // definir loop para dhtUmi
  for (uint8_t i = 0; i < 30; i++) {
    var = dhtP22.readHumidity();
    if ((isnan(var)) || (var <= 1) || (var > 100)) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
    delay(2000);
  }
  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    delay(500);
    Serial.println("v dhtHumi");
  } else {
    Serial.println("F dhtHumi");
  }
*/
  //-------------------------------------------------------------------------------
  /*
  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para dallas
  for (uint8_t i = 0; i < 50; i++) {
    sensors.requestTemperatures();
    var = sensors.getTempCByIndex(0);
    if ((var == -127.00) || (var == 85)) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
    delay(100);
  }
  Serial.println(contadorErros);
  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    delay(500);
    Serial.println("v dallas");
  } else {
    Serial.println("f dallas");
  }
  */
  /// //-------------------------------------------------------------------------------

  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para NTC novo
  delay(1000);
  for (uint8_t i = 0; i < 50; i++) {
    var = (analogRead(A1) * 5.0) / (1023.0);
    var = (10000.0 / ((5.10 / var) - 1.0));
    var = (-0.00000000000958485951 * pow(var, 3)) + (0.0000004754241223026 * pow(var, 2)) + (-0.008998819476625 * var) + (77.7144937226049);
    if (var > 100 || var < 0) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
  }

  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    delay(500);
    Serial.println("v NTC_NOVO");
  } else {
    Serial.println("f NTC_NOVO");
  }
  //---------------------------------------------------------------------------------------------
  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para NTC VELHO
  digitalWrite(8, HIGH);
  for (uint8_t i = 0; i < 50; i++) {
    var = (analogRead(A0) * 5.0) / (1023.0);
    var = (10000.0 / ((5.10 / var) - 1.0));
    var = (-0.00000000001033040468 * pow(var, 3)) + (0.000000494671652376 * pow(var, 2)) + (-0.0091476893610536 * var) + (77.6493119075527);
    if (var > 100 || var < 0) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
  }

  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    delay(500);
    Serial.println("v NTC_VELHO");
  } else {
    Serial.println("f NTC_VELHO");
  }
  //--------------------------------------------------------------------------------------------------------------------------

  contadorErros = 25;
  var = 0;
  Serial.println(4);
  uint8_t contadorRepeticaoErros;
  float bulboSecoTempLoop;
  float bulboUmidoTempLoop;

  for (uint8_t i = 0; i < 50; i++) {
    contadorRepeticaoErros = 0;
    bulboSecoTempLoop = 0;
    bulboUmidoTempLoop = 0;
    tempBulboSeco = 0;
    tempBulboUmido = 0;
    for (uint8_t i = 0; i < 25; i++) {
      //coleta do valor da temperatura do bulbo umido --  NTC VELHO
      var = (analogRead(A0) * 5.0) / (1023.0);
      var = (10000.0 / ((5.10 / var) - 1.0));
      tempBulboUmido = (-0.00000000001033040468 * pow(var, 3)) + (0.000000494671652376 * pow(var, 2)) + (-0.0091476893610536 * var) + (77.6493119075527);
      // coleta do valor da temperatura do bulbo seco -- NTC NOVO
      var = (analogRead(A1) * 5.0) / (1023.0);
      var = (10000.0 / ((5.10 / var) - 1.0));
      tempBulboSeco = (-0.00000000000958485951 * pow(var, 3)) + (0.0000004754241223026 * pow(var, 2)) + (-0.008998819476625 * var) + (77.7144937226049);
      if ((tempBulboSeco > 100 || tempBulboSeco < 0) || (tempBulboUmido > 100 || tempBulboUmido < 0)) {
        contadorRepeticaoErros = contadorRepeticaoErros + 1;
      } else {
        bulboSecoTempLoop = bulboSecoTempLoop + tempBulboSeco;
        bulboUmidoTempLoop = bulboUmidoTempLoop + tempBulboUmido;
      }
    }
    if (contadorRepeticaoErros > 15) {
      contadorErros = contadorErros + 1;
    } else {
      bulboSecoTempLoop = bulboSecoTempLoop / (25 - contadorRepeticaoErros);
      bulboUmidoTempLoop = bulboUmidoTempLoop / (25 - contadorRepeticaoErros);
      // calculo da UR
      float potenciaTemp = (7.5 * bulboUmidoTempLoop) / (237.3 + bulboUmidoTempLoop);
      float esTU = 610.8 * pow(10, potenciaTemp);
      // fim calculo esTU
      // calcular esTS
      potenciaTemp = (7.5 * bulboSecoTempLoop) / (237.3 + bulboSecoTempLoop);
      float esTS = 610.8 * pow(10, potenciaTemp);
      // fim calculo esTS
      // calcular ea
      float ea = esTU - (0.0008 * 94000 * (bulboSecoTempLoop - bulboUmidoTempLoop));
      // fim calcular ea
      vetorDados[i] = (ea / esTS) * 100;
    }
  }
  digitalWrite(4, LOW);
  digitalWrite(8, LOW);

  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.println(",");
    dataFile.close();
    delay(500);
    Serial.println("v UR");
  } else {
    Serial.println("f UR");
  }

  /// //-------------------------------------------------------------------------------
  /*
  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para LDR
  for (uint8_t i = 0; i < 50; i++) {
    var = (analogRead(A2) * 5.10) / (1024);
    var = (10000 / ((5.10 / var) - 1));
    var = 75400000 * pow(var, -1.68);
    if (var < 0) {
      contadorErros = contadorErros + 1;
    } else {
      vetorDados[i] = var;
    }
    delay(500);
  }

  analisarDados(vetorDados, contadorErros);

  dataFile = SD.open("test.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print(mediaFinal);
    dataFile.print(",");
    dataFile.print(amplitudadeFinal);
    dataFile.print(",");
    dataFile.print(desvioPadraoFinal);
    dataFile.print(",");
    dataFile.close();
    delay(500);
    Serial.println("v LDR");
  } else {
    Serial.println("f LDR");
    }
*/
  //FINALIZAÇÃO DO LOOP-----------------------------------------------------------------------------------
  SD.end();
  delay(300000);  //valor padrao = 1090000
}

void analisarDados(float vetor[], uint8_t tam) {
  // fazer as 3 estatitica na msm função e atribuir ao valor da variavel dentro da propria função
  float media;
  tam = 50 - tam;
  if (tam > 20) {
    Serial.println("foi");
    //calc media
    for (uint8_t i = 0; i < tam; i++) {
      media = media + vetor[i];
    }
    media = media / float(tam);
    // inicio da remocao dos valores mais distantes da media...
    float maiorDistancia;
    uint8_t indexMaiorValor;
    float tempTeste;
    float tempValorInicial;
    float tempValor0;
    uint8_t j;
    for (uint8_t i = 0; i < 10; i++) {
      maiorDistancia = 0;
      indexMaiorValor = 0;
      tempTeste = 0;
      tempValorInicial = 0;
      tempValor0 = 0;
      j = i;
      for (j = i + 1; j < tam; j++) {
        tempTeste = media - vetor[j];
        if (tempTeste < 0) {
          tempTeste = tempTeste * (-1);
        }
        if (tempTeste > maiorDistancia) {
          maiorDistancia = tempTeste;
          indexMaiorValor = j;
        }
      }
      tempValorInicial = vetor[indexMaiorValor];
      tempValor0 = vetor[i];
      vetor[i] = tempValorInicial;
      vetor[indexMaiorValor] = tempValor0;
    }

    tam = tam - 10;  // modificado tamanho do tam com constante
    //recalc media.....
    media = 0;
    for (uint8_t i = 0; i < tam; i++) {
      media = media + vetor[i];
    }
    mediaFinal = media / float(tam);
    // calc amplitude
    float maiorValor = 0;
    float menorValor = 200;
    for (uint8_t i = 0; i < tam; i++) {
      if (vetor[i] > maiorValor) {
        maiorValor = vetor[i];
      }
      if (vetor[i] < menorValor) {
        menorValor = vetor[i];
      }
    }
    amplitudadeFinal = maiorValor - menorValor;
    //calc variancia
    float somatoria = 0;
    float tempi = 0;
    float resultado = 0;
    for (uint8_t i = 0; i < tam; i++) {
      tempi = vetor[i] - mediaFinal;
      if (tempi < 0) {
        tempi = tempi * (-1);
      }
      resultado = pow(tempi, 2);
      somatoria = somatoria + resultado;
    }
    somatoria = somatoria / float(tam);
    //fim calc variancia
    //incio calc desvio padrao
    somatoria = sqrtf(somatoria);
    //fim calc desvio padrao
    desvioPadraoFinal = somatoria;
  } else {
    mediaFinal = -100;
    amplitudadeFinal = -100;
    desvioPadraoFinal = -100;
  }
}


