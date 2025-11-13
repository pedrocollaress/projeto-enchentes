#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h> // A biblioteca que acabamos de adicionar

// --- Configurações de Rede ---
const char* WIFI_SSID = "SuaRedeWiFi";
const char* WIFI_PASS = "SuaSenha";

// --- Configurações do Servidor ---
// Mude para o IP ou domínio do seu servidor
const char* SERVER_HOST = "192.168.1.100"; 
const uint16_t SERVER_PORT = 8080;
const char* SERVER_PATH = "/"; // O "endpoint" do seu WebSocket (ex: "/ws")

// --- Configurações de Hardware ---
// O pino onde você ligou o divisor de tensão
#define SENSOR_PIN D5 

// --- Variáveis Globais ---
WebSocketsClient webSocket;
bool ultimoEstadoSensor = LOW; // Para guardar o estado anterior e detectar a borda (o pulso)

/**
 * @brief Função chamada em eventos do WebSocket (conexão, desconexão, dados)
 */
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Desconectado! Tentando reconectar...");
      break;

    case WStype_CONNECTED:
      Serial.printf("[WSc] Conectado ao servidor: %s\n", payload);
      // Opcional: Envia uma mensagem "Oi, estou aqui"
      webSocket.sendTXT("NodeMCU_Enchente_1 Conectado"); 
      break;

    case WStype_TEXT:
      Serial.printf("[WSc] Mensagem recebida do servidor: %s\n", payload);
      // Aqui você pode tratar comandos vindos do servidor
      // Ex: if(strcmp((char*)payload, "PING") == 0) { webSocket.sendTXT("PONG"); }
      break;

    case WStype_ERROR:
    case WStype_BIN:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      // Eventos não utilizados neste exemplo
      break;
  }
}

/**
 * @brief Configura a conexão Wi-Fi
 */
void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Função de setup principal (executa uma vez)
 */
void setup() {
  Serial.begin(115200); // Inicia o monitor serial (para debug)
  Serial.println("\n[SETUP] Iniciando sistema...");

  // Configura o pino do sensor como ENTRADA
  pinMode(SENSOR_PIN, INPUT); 
  ultimoEstadoSensor = digitalRead(SENSOR_PIN); // Lê o estado inicial

  // Conecta ao Wi-Fi
  setupWifi();

  // Inicia o cliente WebSocket
  webSocket.begin(SERVER_HOST, SERVER_PORT, SERVER_PATH);
  
  // Define a função que vai cuidar dos eventos
  webSocket.onEvent(webSocketEvent);

  // Tenta reconectar a cada 5 segundos se a conexão cair
  webSocket.setReconnectInterval(5000); 

  Serial.println("[SETUP] Setup completo. Aguardando eventos...");
}

/**
 * @brief Verifica o estado do sensor e envia o "pulso" se houver mudança
 */
void checarSensor() {
  bool estadoAtualSensor = digitalRead(SENSOR_PIN);

  // Verifica se o estado MUDOU (detecção de borda/pulso)
  if (estadoAtualSensor != ultimoEstadoSensor) {
    
    // Pequeno delay para evitar "ruído" (debounce)
    delay(50); 
    // Lê de novo para confirmar
    estadoAtualSensor = digitalRead(SENSOR_PIN);
    if(estadoAtualSensor == ultimoEstadoSensor) return; // Foi só ruído, ignora.

    // Atualiza o estado
    ultimoEstadoSensor = estadoAtualSensor;

    // Se o estado atual for ALTO (HIGH), significa que detectou
    if (estadoAtualSensor == HIGH) {
      Serial.println("[SENSOR] PULSO DETECTADO! (Água atingiu o nível)");
      // Envia a mensagem para o servidor "ouvir"
      webSocket.sendTXT("PULSO:DETECTADO");
    } 
    // Se o estado atual for BAIXO (LOW), significa que normalizou
    else {
      Serial.println("[SENSOR] PULSO NORMALIZADO! (Água baixou do nível)");
      // Envia a mensagem para o servidor "ouvir"
      webSocket.sendTXT("PULSO:NORMALIZADO");
    }
  }
}

/**
 * @brief Loop principal (executa continuamente)
 */
void loop() {
  // Esta função é OBRIGATÓRIA. 
  // Ela mantém a conexão WebSocket viva e escuta por mensagens.
  webSocket.loop();
  
  // Nossa função que verifica o sensor
  checarSensor();

  // Um pequeno delay para não sobrecarregar o loop
  delay(100); 
}
