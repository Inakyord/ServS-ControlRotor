// **************************************************************************************************************
// ****  Cabecera funciones.h: declaración de bibliotecas, constantes y funciones a utilizar.                ****
// ****  Última modificación: 5 de diciembre del 2022.                                                        ****
// ****                                                                                                      ****
// ****                                                                                                      ****
// ****  Programa elaborado por: IÑAKY ORDIALES CABALLERO                                                    ****
// ****  dentro de la Universidad Nacional Autónoma de México.                                               ****
// ****                                                                                                      ****
// ****  Como parte del servicio social realizado en el: Instituto de Ciencas Aplicadas y Tecnología (ICAT)  ****
// ****  en el laboratiorio de "Modelado y Simulación de Procesos" del área de instrumentación,              ****
// ****  durante el periodo abarcado de junio del 2022 a enero del 2023.                                     ****
// ****                                                                                                      ****
// ****  Tutor encargado:  M. I. RAFAEL PRIETO MELÉNDEZ                                                      ****
// ****                                                                                                      ****
// **************************************************************************************************************

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <LiquidCrystal.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Salida de control
#define ON  1
#define OFF 0

// PINES ENTRADAS HW
#define AZ_PIN 27    		// lectura azimuth 
#define EL_PIN 26    		// lectura elevation
#define OFFSET_PIN 28		// lectura del offset (conexion a tierra)
// PINES SALIDAS HW
#define R_PIN 14    		// para aumentar azimuth (derecha)
#define U_PIN 13    		// para aumentar elevación (arriba)
#define L_PIN 12    		// para disminuir azimuth (izquierda)
#define D_PIN 11    		// para disminuir elevación (abajo)
#define MR_PIN 6 			// derecha manual
#define MU_PIN 7 			// arriba manual
#define ML_PIN 8 			// izquierda manual
#define MD_PIN 9			// abajo manual
#define MAN_PIN 10 			// habilitar modo manual
#define USB_PIN 24
// PINES CONTROL DISPLAY
#define DRS_PIN 16
#define DE_PIN 17
#define D4_PIN 18
#define D5_PIN 19
#define D6_PIN 20
#define D7_PIN 21
// Muestreo de posición
#define MUESTRAS 8 		// número de muestras para el promedio
#define FREQ 66			// cuántas veces calcula el promedio por segundo
#define MAX_CADENA 30	// número máximo de caracteres de la cadena que lee

#define T_ERROR 300 	// tiempo prendido foco error
#define ERROR_PIN 15 	// mostrar error de lectura de comando


// DECLARACIÓN DE FUNCIONES
void inicializacion(LiquidCrystal&, uint16_t*, uint16_t*, uint16_t*, int*, uint16_t*);
int mapeo(long, long, long, long, long);
void get_Pos(uint16_t*, uint16_t*, int*, uint16_t*);
void error(int);
void movimientoRotor(uint16_t*, uint16_t*, int, int);
void dibujarDisplay(LiquidCrystal&, uint16_t*, uint16_t*, int, int, int);
void second_core_code();