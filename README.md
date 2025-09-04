# Fundamentos y aplicación del módulo Bluetooth AT-09

## 1. Objetivo General
Elaborar un README que sirva como guía exhaustiva y profesional para la práctica del módulo Bluetooth AT-09, facilitando a cualquier lector la comprensión del propósito, la instalación, el uso y el contexto ético de un sistema Bluetooth en entornos inteligentes.

## 2. Objetivos Específicos
- Explicar el principio de funcionamiento de la tecnología Bluetooth.  
- Identificar los parámetros eléctricos y de comunicación del AT-09.  
- Configurar el hardware y software de manera reproducible.  
- Implementar un sistema de envío y recepción de mensajes JSON usando BLE y MQTT.  
- Evaluar los resultados de la práctica a través de pruebas con LEDs y mensajes en broker MQTT.  

## 3. Competencias
- Manejo de módulos de comunicación inalámbrica.  
- Integración de hardware y software IoT.  
- Uso de protocolos de comunicación (UART, BLE, MQTT).  
- Análisis crítico de seguridad y privacidad en comunicaciones.  
- Documentación técnica clara y profesional.  

## 4. Tabla de Contenidos
1. Objetivo General  
2. Objetivos Específicos  
3. Competencias  
4. Tabla de Contenidos  
5. Descripción  
6. Requisitos  
7. Instalación y Configuración  
8. Conexiones de Hardware  
9. Parámetros Técnicos del AT-09  
10. Uso y ejemplos de Código  
11. Resultados de Prueba  
12. Consideraciones Éticas y de Seguridad  
13. Formato de Salida (JSON)  
14. Solución de Problemas  
15. Contribuciones  
16. Referencias  

## 5. Descripción
Esta práctica introduce el uso del módulo Bluetooth AT-09 (clon del HM-10) para establecer comunicación inalámbrica de baja energía (BLE). Su integración con un ESP32 y un broker MQTT permite demostrar cómo un sistema embebido puede interactuar con aplicaciones móviles y servicios en la nube, clave para entornos inteligentes e IoT.

## 6. Requisitos
### Hardware
- ESP32 DevKit v1  
- Módulo Bluetooth AT-09  
- Resistencias de 220 Ω  
- LEDs de colores  
- Protoboard y cables Dupont  

### Software y Bibliotecas
- Arduino IDE  
- Librerías: `WiFi.h`, `PubSubClient.h`, `ArduinoJson.h`  
- App móvil: nRF Connect (iOS/Android)  
- Broker MQTT: test.mosquitto.org o Mosquitto local  

### Conocimientos previos
- Programación en Arduino C++  
- Conceptos básicos de UART y BLE  
- Manejo de JSON y publicación MQTT  

## 7. Instalación y Configuración
1. Instalar Arduino IDE y librerías requeridas.  
2. Conectar ESP32 al PC y seleccionar la tarjeta correcta.  
3. Configurar credenciales WiFi en el código (`WIFI_SSID` y `WIFI_PASS`).  
4. Cablear el módulo AT-09 a UART2 del ESP32 (GPIO16 RX2, GPIO17 TX2).  
5. Subir el código proporcionado y abrir el monitor serie a 115200 baudios.  

## 8. Conexiones de Hardware
| Señal AT-09 | Pin ESP32 | Función |
|-------------|-----------|---------|
| VCC (3.3V)  | 3.3V      | Alimentación |
| GND         | GND       | Tierra común |
| TXD         | GPIO16 RX2| Comunicación UART |
| RXD         | GPIO17 TX2| Comunicación UART |
| STATE       | GPIO4     | Estado de conexión BLE |

## 9. Parámetros Técnicos del AT-09
| Parámetro | Valor típico | Unidad |
|-----------|--------------|--------|
| Voltaje de operación | 3.3 | V |
| Corriente en operación | 8–15 | mA |
| Interfaz | UART | - |
| Frecuencia | 2.4 | GHz |
| Protocolo | BLE 4.0 | - |

## 10. Uso y ejemplos de Código
El programa inicializa WiFi, MQTT y BLE. Los LEDs indican estado:  
- Blanco → WiFi.  
- Azul/Rojo → BLE conectado/desconectado.  
- Verde → mensaje enviado.  
- Amarillo → mensaje válido recibido.  
- Naranja → mensaje con ID inválido.  
- Violeta → actuador ON/OFF.  

Ejemplo de JSON generado:  
{"id":"JFMDKAVG","origen":"Consola serial","accion":"ON","fecha":"03-09-2025","hora":"21:00:00"}


## 11. Resultados de Prueba
- Desde consola serial → envío de ON/OFF → LED verde parpadea, LED violeta responde.  
- Desde nRF Connect → envío de JSON válido → LED amarillo parpadea, mensaje publicado en MQTT.  
- Desde MQTT Explorer → publicación en `JFMDKAVG_RX` → ESP32 recibe y controla LEDs.  

## 12. Consideraciones Éticas y de Seguridad
- Cifrar comunicaciones BLE en aplicaciones reales.  
- Proteger credenciales WiFi/MQTT.  
- Evitar la transmisión de datos personales sin consentimiento.  
- Implementar autenticación en producción.  

## 13. Formato de Salida (JSON)
| Campo | Tipo | Descripción |
|-------|------|-------------|
| id | String | Identificador del equipo |
| origen | String | Fuente del mensaje (MQTT, BLE, Serial) |
| accion | String | Acción ON/OFF |
| fecha | String | Fecha en DD-MM-AAAA |
| hora | String | Hora en HH:MM:SS |

## 14. Solución de Problemas
| Problema | Posible causa | Solución |
|----------|---------------|----------|
| No enciende módulo | Voltaje incorrecto | Usar 3.3V, no 5V |
| No conecta BLE | STATE no cableado | Usar detección por actividad o conectar STATE |
| MQTT no recibe | Tópico incorrecto | Revisar `TEAM_ID_RX` |
| JSON inválido | Formato mal escrito | Usar comillas dobles y llaves `{}` |

## 15. Contribuciones
- Hacer fork del repositorio.  
- Crear rama con nombre descriptivo.  
- Documentar cambios en README.  
- Abrir Pull Request para revisión.  

## 16. Referencias
- Banzi, M., & Shiloh, M. (2014). *Getting Started with Bluetooth Low Energy: Tools and Techniques for Low-Power Networking*. O’Reilly Media.  
- Texas Instruments. (2010). *Bluetooth Low Energy Technology Overview*. Texas Instruments White Paper. Recuperado de https://www.ti.com/lit/an/swra292  
- NXP Semiconductors. (2016). *HM-10/AT-09 Bluetooth Low Energy module datasheet*. NXP.  
- Collins, J. (2011). *Designing Connected Products with Bluetooth Low Energy*. O’Reilly Media.  
- Arduino. (2023). *Arduino ESP32 Documentation*. Recuperado de https://docs.arduino.cc/  

