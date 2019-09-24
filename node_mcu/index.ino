#include <ESP8266WiFi.h>  // IMPORTA A BIBLIOTECA ESP8266WiFi
#include <PubSubClient.h> // IMPORTA A BIBLIOTECA PubSubClient

// ID MQQT, TOPICO PARA PUBLICACAO E SUBSCRIBE
#define TOPICO_SUBSCRIBE  "ph3i-subscribe" 
#define TOPICO_PUBLISH    "ph3i-publish"
#define ID_MQTT           "ph3i-id-mqtt"
                               
// MAPEAMENTO DE PINOS DO NodeMCU
#define D0    16 //GPIO16 -> LED
#define D1    5  //GPIO5  -> RELE
#define D5    14 //GPIO14 -> Sensor1 -> Azul
#define D6    12 //GPIO12 -> Sensor2 -> Verde
#define D7    13 //GPIO13 -> Sensor3 -> Laranja

// WIFI Home
//const char* SSID = "Home 2.5";
//const char* PASSWORD = "S1F7528240";

// WIFI Work
//const char* SSID = "Work 5.0";
//const char* PASSWORD = "S1F7528240";

// WIFI Celphone
const char* SSID = "Celphone 4G";
const char* PASSWORD = "S1F7528240";
 
// MQTT
const char* BROKER_MQTT = "iot.eclipse.org";
int BROKER_PORT = 1883;

// VARIAVEIS GLOBAIS
WiFiClient espClient;         // CRIANDO O OBJETO: espClient
PubSubClient MQTT(espClient); // INSTANCIANDO O CLIENTE MQTT PASSANDO O OBJETO: espClient
char EstadoSaida = '0';       // VARIAVEL QUE ARMAZENA O ESTADO ATUAL DA SAIDA
int valor_s1 = 0;             // INICIA OS SENSORES COM O VALOR 0 = CHEIO
int valor_s2 = 0;             // INICIA OS SENSORES COM O VALOR 0 = CHEIO
int valor_s3 = 0;             // INICIA OS SENSORES COM O VALOR 0 = CHEIO
int nivel_anterior = 0;       // ZERA O NIVEL DA CAIXA DA AGUA
 
// PROTOTYPES
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

/* 
 *  IMPLEMENTAÇÃO DAS FUNÇÕES
 */
void setup() 
{
    // INICIALIZACOES
    InitOutput();
    initSerial();
    initMQTT();
    initWiFi();

    // DEFINE O PINO DO RELE COMO SAIDA
    pinMode(D1, OUTPUT);

    // INICIA COM O RELE DESLIGADO
    digitalWrite(D1 ,LOW);

    // DEFINE OS PINOS DOS SENSORES COMO ENTRADA
    pinMode(D5, INPUT); //Sensor1 -> Azul
    pinMode(D6, INPUT); //Sensor2 -> Verde
    pinMode(D7, INPUT); //Sensor3 -> Laranja
}

/*
 * FUNCAO: INCIALIZA COMuNICACAO SERIAL COM BAUDRATE 115200 (PARA FINS DE MONITORAR NO TERMINAL SERIAL O QUE ESTA ACONTECENDO)
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void initSerial() 
{
    Serial.begin(9600);
}

/*
 * FUNCAO: INICIALIZA E CONECTA-SE NA REDE WIFI DESEJADA
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void initWiFi() 
{
    delay(10);
    Serial.println("CONEXAO WIFI");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde...");
    
    reconectWiFi();
}

/*
 * FUNCAO: INICIALIZA PARAMETROS DE CONEXAO MQTT (ENDERECO DO BOROKER, PORTA E SETA FUNCAO DE CALLBACK)
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   // INFORMA QUAL BROKER E PORTA DEVE SER CONECTADO
    MQTT.setCallback(mqtt_callback);            // ATRIBUI FUNCAO DE CALLBACK (FUNCAO CHAMADA QUANDO QUALQUER INFORMACAO DE UM DOS TOPICOS SUBESCRITOS CHEGA)
}

/*
 * FUNCAO: CALLBACK (ESTA FUNCAO EH CHAMADA TODA VEZ QUE UMA INFORMACAO DE UM DOS TOPICOS SUBESCRITOS CHEGA)
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;

    // OBTEM A STRING DO PAYLOAD RECEBIDO
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    /*
     * TOMA ACAO DEPENDENDO DA STRING RECEBIDA
     * VERIFICA SE DEVE COLOCAR NIVEL ALTO DE TENSAO NA SAIDA D0 (LED)
     * 
     * IMPORTANTE: O LED CONTIDO NA PLACA EH ACIONADO COM LOGICA INVERTIDA (ENVIA HIGH PARA O OUTPUT FAZ O LED APAGAR / ENVIAR LOW FAZ O LED ACENDER)
     */
    if (msg.equals("L"))
    {
        digitalWrite(D0, LOW);
        EstadoSaida = '1';
    }

    // VERIFICA SE DEVE COLOCAR NVIEL ALTO DE TENSAO NA SAIDA D0
    if (msg.equals("D"))
    {
        digitalWrite(D0, HIGH);
        EstadoSaida = '0';
    }
    
}

/*
 * FUNCAO: RECONECTA-SE AO BROKER MQTT (CASO AINDA NAO ESTEJA CONECTADO OU EM CASO DE A CONEXAO CAIR
 * 
 * PARAMETRO: NENHUM
 * RETORNO: NENHUM
 */
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else 
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

/*
 * FUNCAO: RECONECTA-SE AO WIFI
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void reconectWiFi() 
{
    // SE JA ESTA CONECTADO A REDE WIFI, NADA EH FEITO
    // CASO CONTRARIO SÃO EFETUADAS TENTATIVAS DE CONEXAO
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // CONECTA NA REDE WIFI
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso na rede: ");
    Serial.println(SSID);
    Serial.print("IP obtido: ");
    Serial.println(WiFi.localIP());
}

/*
 * FUNCAO: VERIFICA O ESTADO DAS CONEXOES WIFI E AO BROKER MQTT (EM CASO DE DESCONEXAO, A MESMA EH REFEITA PARA QUALQUER UMA DAS DUAS)
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
      reconnectMQTT();  // SE NAO HA CONEXAO COM O BROKER, A CONEXAO EH REFEITA
      reconectWiFi();   // SE NAO HA CONEXAO COM O WIFI, A CONEXAO EH REFEITA
}

/*
 * FUNCAO: ENVIA AO BROKER O ESTADO ATUAL DO OUTPUT
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void EnviaEstadoOutputMQTT(void)
{
    if (EstadoSaida == '0')
      MQTT.publish(TOPICO_PUBLISH, "D");

    if (EstadoSaida == '1')
      MQTT.publish(TOPICO_PUBLISH, "L");

    Serial.println("- Estado da saida D0 enviado ao broker!");
    delay(1000);
}

/*
 * FUNCAO: INICIALIZA O OUTPUT EM NIVEL LOGICO BAIXO
 * 
 * PARAMETROS: NENHUM
 * RETORNO: NENHUM
 */
void InitOutput(void)
{
    // IMPORTANTE: O LED CONTIDO NA PLACA EH ACIONADO COM LOGICA INVERTIDA (ENVIA HIGH PARA O OUTPUT FAZ O LED APAGAR / ENVIAR LOW FAZ O LED ACENDER)
    pinMode(D0, OUTPUT);
    digitalWrite(D0, HIGH);          
}

/*
 * PROGRAMA PRINCIPAL
 */
void loop() 
{   
    // GARANTE O FUNCIONAMENTO DAS CONEXOES WIFI E AO BROKER MQTT
    VerificaConexoesWiFIEMQTT();

    // LEITURA DOS SENSORES
    int valor_s1 = digitalRead(D5);
    int valor_s2 = digitalRead(D6);
    int valor_s3 = digitalRead(D7);

    // MOSTRA OS VALORES DOS SENSORES NO SERIAL MONITOR
    Serial.print("S1: ");
    Serial.print(valor_s1);
    Serial.print(" S2: ");
    Serial.print(valor_s2);
    Serial.print(" S3: ");
    Serial.println(valor_s3);

    // ESTA CHEIO
    if ((valor_s1 == 0) && valor_s2 == 0 && valor_s3 == 0)
    {
      // ATUALIZA O VALOR DO DISPLAY
      //mostra_nivel(0);

      // ENVIA PARA O MQTT O NIVEL
      MQTT.publish(TOPICO_PUBLISH, "C"); //C -> Cheio
      
      // DESLIGA O RELE
      digitalWrite(D1, LOW);
    }

    if ((valor_s1 == 1) && valor_s2 == 0 && valor_s3 == 0)
    {
      // ATUALIZA O VALOR DO DISPLAY
      //mostra_nivel(11);

      // ENVIA PARA O MQTT O NIVEL
      MQTT.publish(TOPICO_PUBLISH, "M"); //M -> Médio
    }
  
    if ((valor_s1 == 1) && valor_s2 == 1 && valor_s3 == 0)
    {
      // ATUALIZA O VALOR DO DISPLAY
      //mostra_nivel(4);

      // ENVIA PARA O MQTT O NIVEL
      MQTT.publish(TOPICO_PUBLISH, "B"); //B -> Baixo
    }

    // ESTA VAZIO
    if ((valor_s1 == 1) && valor_s2 == 1 && valor_s3 == 1)
    {
      // MOSTRA O NIVEL NO DISPLAY
      //mostra_nivel(15);

      // ENVIA PARA O MQTT O NIVEL
      MQTT.publish(TOPICO_PUBLISH, "V"); //V -> Vazio
      
      // LIGA O RELE
      digitalWrite(D1, HIGH);
    }
  
    delay(100);

    // ENVIA O STATUS DE TODOS OS OUTPUTS PARA O BROKER NO PROTOCOLO ESPERADO
    //EnviaEstadoOutputMQTT();

    // KEEP-ALIVE DA COMUNICACAO COM BROKER MQTT
    MQTT.loop();
}

