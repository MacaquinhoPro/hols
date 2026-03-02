#include <Arduino.h>

/* --- DEFINICIÓN DE HARDWARE (ACORDADO POR EL GRUPO) --- */
const int PIN_CONTROL_FREQ = 34; // Entrada analógica para el potenciómetro
const int PIN_INTERRUPTOR  = 12; // Pulsador para activar/desactivar sistema
const int PIN_GENERADOR    = 25; // Salida del DAC (Canal 1)
const int PIN_OSCILOSCOPIO = 35; // Entrada del ADC para visualizar señales

/* --- VARIABLES DE CONTROL Y ESTADO --- */
bool sistemaHabilitado = true;
float faseAcumulada = 0;
int ultimoEstadoBoton = HIGH;

void setup() {
  // Inicializamos la comunicación serial para la visualización de datos
  Serial.begin(115200);
  
  // Configuración de entradas y salidas según el esquema
  pinMode(PIN_INTERRUPTOR, INPUT_PULLUP);
  
  // Establecemos la resolución del ADC a 12 bits para máxima precisión (0-4095)
  analogReadResolution(12);
  
  Serial.println(">> Sistema de Osciloscopio y Generador Iniciado <<");
}

void loop() {
  // 1. LÓGICA DEL PULSADOR (CONTROL DE ENCENDIDO/APAGADO)
  int lecturaBoton = digitalRead(PIN_INTERRUPTOR);
  if (lecturaBoton == LOW && ultimoEstadoBoton == HIGH) {
    sistemaHabilitado = !sistemaHabilitado;
    delay(100); // Filtro antirrebote simple definido en el diseño
  }
  ultimoEstadoBoton = lecturaBoton;

  if (sistemaHabilitado) {
    // 2. CONTROL DE PARÁMETROS (POTENCIÓMETRO)
    // Mapeamos la lectura para controlar la velocidad de incremento de fase
    int lecturaPot = analogRead(PIN_CONTROL_FREQ);
    float incrementoFase = map(lecturaPot, 0, 4095, 1, 100) / 100.0;

    // 3. GENERADOR DE SEÑAL ANALÓGICA (DAC)
    // Implementamos una onda senoidal estándar: 127 + 127 * sin(x)
    // El valor final se envía al DAC (8 bits, rango 0-255)
    int valorDAC = 127 + (127 * sin(faseAcumulada));
    dacWrite(PIN_GENERADOR, valorDAC);
    
    // Actualizamos la fase para el siguiente ciclo
    faseAcumulada += incrementoFase;
    if (faseAcumulada > 2 * PI) faseAcumulada = 0;

    // 4. FUNCIÓN DE OSCILOSCOPIO (ADC)
    // Capturamos la señal externa (o interna en caso de loopback)
    int señalEntrada = analogRead(PIN_OSCILOSCOPIO);

    // 5. SALIDA DE DATOS PARA DOCUMENTACIÓN
    // Escalamos el DAC (x16) para que sea comparable gráficamente con el ADC de 12 bits
    Serial.print("DAC_Out_Escalado:");
    Serial.print(valorDAC * 16); 
    Serial.print(",");
    Serial.print("ADC_In_Lectura:");
    Serial.println(señalEntrada);

  } else {
    // Estado de reposo: Salida DAC a nivel bajo
    dacWrite(PIN_GENERADOR, 0);
  }

  // Definimos una frecuencia de muestreo estable para las pruebas
  delay(2); 
}