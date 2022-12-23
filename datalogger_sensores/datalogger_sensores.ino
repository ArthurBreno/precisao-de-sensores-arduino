/*
Codigo desenvolvido por: Arhur Breno
Data: 20/12/2022

projeto desenvolvido utilizando datalogger shield V1.0
LER README.md
*/

#include <SPI.h>  
#include <SD.h>   
#include <DHT.h> 
#include <Wire.h>
#include <TimeLib.h>   
#include <DS1307RTC.h>  
#include <OneWire.h>   
#include <DallasTemperature.h>

//------------------------------------------------------
//#define DHT22pino 3
#define DHT211pino 3
#define ONE_WIRE_BUS 4

//DHT dhtP11(DHT11pino, DHT22)
DHT dhtP22(DHT22pino, DHT22);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  

uint8_t ntcBulboSeco = A1;
uint8_t ntcBulboUmido = A0;

//------------------------------------------------------
float vetorDados[50];			// inicialização do vetor que irá armazenar os valores que serão analisados pela função analisarDados()
float var = 0;				// variavel que irá armazenar os valores dos sensores    
File dataFile;				// arquivo utilizado para gravar os valores no cartão SD
uint8_t contadorErros = 0;		// variavel utilizada para contar o número de leituras fora do intervalo escolhido
float mediaFinal = 0;			// variavel modificada somente na função analisar dados para armazenar valor da média
float amplitudadeFinal = 0;		// variavel modificada somente na função analisar dados para armazenar valor da amplitude dos valores no vetor
float desvioPadraoFinal = 0;		// variavel modificada somente na função analisar dados para armazenar valor do desvio padrão
float tempBulboSeco = 0;
float tempBulboUmido = 0;

//-----------------------------------------------------

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(ntcBulboUmido, INPUT);
  pinMode(ntcBulboSeco, INPUT);
  Serial.println(1);
  //------------------------------------------------------
  dhtP22.begin();
  sensors.begin();
  delay(100);
  Serial.println(2);
  SD.begin(10);
  delay(100);
	
  dataFile = SD.open("test.txt", FILE_WRITE);	// verificar inicialização do cartão SD, e caso haja reinicialização do microcontrolador
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
  dataFile = SD.open("test.txt", FILE_WRITE);	//armazenar data e hora em variavel e 
  tmElements_t tm;
  if (dataFile) {
    if (RTC.read(tm)) {
      Serial.println("v hora");
      String dataHora = String(tm.Day) + "/" + String(tm.Month) + "/" + String(tmYearToCalendar(tm.Year)) + " " + String(tm.Hour) + ":" + String(tm.Minute) + ",";
      dataFile.print(dataHora);
      dataFile.close();
      delay(500);
    }
  } else {
    Serial.print("F hora");
  }
/*
o workflow do codigo está construido de forma a ler e gravar SEPARADAMENTE cada sensor, assim para remover algum sensor basta remover ou comentar o espaço
entre a linha de traços
*/
  //FIM GRVAR HORA NO SD
  Serial.println(3);
  //-----------------------------------------------------------------------------
  contadorErros = 20;
  var = 0;
  Serial.println(4);
  // definir loop para dhtTemp
  for (uint8_t i = 0; i < 30; i++) {
    var = dhtP22.readTemperature();
    if (isnan(var) || var <= 1) { 	// condição para decidir se o valor lido é valido ou não
      contadorErros = contadorErros + 1; // caso o valor lido seja invalido, será adicionado 1 ao contador de erros
    } else {
      vetorDados[i] = var;		// caso o valor lido seja valido, será 
    }
    delay(2000);			// conforme recomenda a documentacao do DHT22, o intervalo entre leituras nao deve ser inferior a 2 segundos	
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

  //------------------------------------------------------------------------------
  
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

  //-------------------------------------------------------------------------------
  
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
  
  /// //-------------------------------------------------------------------------------

  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para NTC BulboSeco
  delay(1000);
  for (uint8_t i = 0; i < 50; i++) {
    var = (analogRead(ntcBulboSeco) * 5.0) / (1023.0); // conversão do ADC (analog to digital converte) para tensão em volts
    var = (10000.0 / ((5.10 / var) - 1.0)); // conversão de volts para ohm
    var = (-0.00000000000958485951 * pow(var, 3)) + (0.0000004754241223026 * pow(var, 2)) + (-0.008998819476625 * var) + (77.7144937226049);
	  // equacao para converter resistencia em temperatura °C(varia de NTC para NTC)
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
    Serial.println("v NTC_bulboSeco");
  } else {
    Serial.println("f NTC_bulboSeco");
  }
  //---------------------------------------------------------------------------------------------
  contadorErros = 0;
  var = 0;
  Serial.println(4);
  // definir loop para NTC BulboUmido
  digitalWrite(8, HIGH);
  for (uint8_t i = 0; i < 50; i++) {
    var = (analogRead(ntcBulboUmido) * 5.0) / (1023.0);
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
    Serial.println("v NTC_bulboUmido");
  } else {
    Serial.println("f NTC_bulboUmido");
  }
  //--------------------------------------------------------------------------------------------------------------------------

  contadorErros = 25;
  var = 0;
  Serial.println(4);
  //loop para calculo da umidade relativa
  uint8_t contadorRepeticaoErros;
  float bulboSecoTempLoop;
  float bulboUmidoTempLoop;

	
  for (uint8_t i = 0; i < 50; i++) { // loop para atribuir o valor da umidade relativa no vetor, ou adicionar 1 a variavel erro
    contadorRepeticaoErros = 0;
    bulboSecoTempLoop = 0;
    bulboUmidoTempLoop = 0;
    tempBulboSeco = 0;
    tempBulboUmido = 0;
    for (uint8_t i = 0; i < 25; i++) { // loop para coletar as temperaturas  individualmente e fazer a media, caso o numero de erros seja infeior a 10
      //coleta do valor da temperatura do bulbo umido
      var = (analogRead(ntcBulboUmido) * 5.0) / (1023.0);
      var = (10000.0 / ((5.10 / var) - 1.0));
      tempBulboUmido = (-0.00000000001033040468 * pow(var, 3)) + (0.000000494671652376 * pow(var, 2)) + (-0.0091476893610536 * var) + (77.6493119075527);
      // coleta do valor da temperatura do bulbo seco
      var = (analogRead(ntcBulboSeco) * 5.0) / (1023.0);
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

  //FINALIZAÇÃO DO LOOP-----------------------------------------------------------------------------------
  SD.end();
  delay(300000);  //valor padrao = 1090000
}

void analisarDados(float vetor[], uint8_t tam) {
  // fazer as 3 estatitica na msm função e atribuir ao valor da variavel dentro da propria função
  float media;
  tam = 50 - tam;
  if (tam > 20) {
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


