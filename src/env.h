// Função para ler variáveis do arquivo .env
// Uso: String valor = getEnv("NOME_VARIAVEL");
#include <Arduino.h>
#include <FS.h>

String getEnv(const String& key) {
  File envFile = SPIFFS.open("/.env", "r");
  if (!envFile) return String("");
  while (envFile.available()) {
    String line = envFile.readStringUntil('\n');
    line.trim();
    if (line.startsWith("#") || line.length() == 0) continue;
    int sep = line.indexOf('=');
    if (sep == -1) continue;
    String k = line.substring(0, sep);
    String v = line.substring(sep + 1);
    k.trim(); v.trim();
    if (k == key) {
      envFile.close();
      return v;
    }
  }
  envFile.close();
  return String("");
}
