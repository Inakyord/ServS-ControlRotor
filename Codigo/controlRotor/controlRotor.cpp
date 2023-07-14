// **************************************************************************************************************
// ****  Código main(): flujo de ejecución del programa en dos núcleos.                                      ****
// ****  Última modificación: 2 de enero del 2022.                                                        ****
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
//
// COORDENADAS ICAT UNAM.
//   Longitud: -99.19
//   Latitud:   19.32
//
/*
=================================================================================================================
||                                                                                                             ||
||                          PROGRAMA DE CONTROL PARA ROTORES DE ANTENA YAESU G-5400B                           ||
||                                                                                                             ||
||  El programa está elaborado para el microcontrolador Pi Pico. Implementa el funcionamiento de comandos      ||
||  necesario para controlar los rotores de azimuth y elevación de la marca YAESU.                             ||
||  Se utilizan ambos núcleos del microcontrolador, uno para la lectura de comandos de entrada y el otro para  ||
||  el análisi de flujo y salidas de los pines lógicos del microcontrolador.                                   ||
||                                                                                                             ||
||                                                                                                             ||
=================================================================================================================
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Bibliotecas utilizadas
#include <stdio.h>
#include <string.h>
#include "pico/multicore.h"
// Biblioteca propia con funciones auxiliares (incluye otras bibliotecas...)
#include "funciones.h"


// Declaración de variables globales
int muestra=0;
uint16_t lecturas[2][MUESTRAS]; //para promedio móvil de az. y el.
uint16_t offsets[MUESTRAS];
uint16_t posActual[2]={0,0};
uint16_t posDeseada[2]={0,0};
// Inicialización del objeto display lcd
LiquidCrystal lcd(DRS_PIN, DE_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);


// Código para ejecutarse en el segundo núcleo
//   este es el encargado de procesar las entradas (del rotor, computadora,
//   switches y botones manuales) para dar las señales correspondientes al
//   display y al rotor para lograr el posicionamiento deseado.
void second_core_code(){

	uint32_t datoComando = 0;
	int mov_AZ_flag = 0;
	int mov_todo_flag = 0;
	int modo_manual=0;
	int anterior=0;

	while(true){

		// obtiene la posición del rotor (azimút y elevación)
		get_Pos(posActual,lecturas[0],&muestra, offsets);

		// checa si se está en modo manual o automático
		modo_manual = (gpio_get(MAN_PIN)==0);
		
		// checa si se cambió a modo manual y apaga las salidas
		if(!modo_manual && anterior==1){
			gpio_put(R_PIN, OFF);
			gpio_put(L_PIN, OFF);
			gpio_put(U_PIN, OFF);
			gpio_put(D_PIN, OFF);
			anterior=0;
		}

		// checa si se cambió a modo manual y elimina instrucción
		//   del movimiento de las antenas
		if(modo_manual && anterior==0){
			mov_todo_flag=0;
			mov_AZ_flag=0;
			anterior=1;
		}

		// checa si no está conectado el usb y deshabilita instrucción
		//   del movimiento de las antenas
		if((mov_AZ_flag || mov_todo_flag) && gpio_get(USB_PIN)==0){
			mov_todo_flag=0;
			mov_AZ_flag=0;
		}

		// actualiza el display lcd según el modo de operación y la instrucción recibida
		dibujarDisplay(lcd, posActual, posDeseada, modo_manual, mov_todo_flag, mov_AZ_flag);
		
		// activa/desactiva las señales del movimiento de los rotores según corresponda
		movimientoRotor(posActual, posDeseada, mov_todo_flag, mov_AZ_flag);

		// checa si está disponible algún comando leído en el primer núcleo y lo recibe
		if (multicore_fifo_rvalid()){
			datoComando=multicore_fifo_pop_blocking();
		}

		// lógica de los botones cuando se está en modo manual.
		if (gpio_get(MAN_PIN)==0){

			if (gpio_get(MR_PIN)==0){
				gpio_put(R_PIN, ON);
				gpio_put(L_PIN, OFF);
			}else if(gpio_get(ML_PIN)==0){
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, ON);
			}else{
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, OFF);
			}

			if (gpio_get(MU_PIN)==0){
				gpio_put(U_PIN, ON);
				gpio_put(D_PIN, OFF);
			}else if (gpio_get(MD_PIN)==0){
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, ON);
			}else{
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, OFF);
			}

			// no responde a comandos que no sean lecturas
			if (datoComando < 1012 || datoComando > 1014){
				datoComando=0;
				continue;
			}
		}

		// continúa el ciclo si se envió algún comando que no sea lectura
		//   estando en modo manual.
		if (datoComando==0){
			continue;
		}

		// lógica de los comandos recibidos desde el núcleo uno.
		switch(datoComando) {
			case 1001:			// offset calibration intern AZ
				break;
			
			case 1002:			// offset calibration intern EL
				break;
			
			case 1003:			// full scale calibration AZ
				break;
			
			case 1004:			// full scale calibration EL
				break;
			
			case 1005:			// Right
				gpio_put(L_PIN, OFF);
				gpio_put(R_PIN, ON);
				break;
			
			case 1006:			// Up
				gpio_put(D_PIN, OFF);
				gpio_put(U_PIN, ON);
				break;			
			
			case 1007:			// Left
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, ON);
				break;			
			
			case 1008:			// Down
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, ON);
				break;			
			
			case 1009:			// stop AZ rotation
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, OFF);
				mov_AZ_flag=0;
				mov_todo_flag=0;
				break;			
			
			case 1010:			// stop EL rotation
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, OFF);
				mov_todo_flag=0;
				break;			
			
			case 1011:			// stop/cancel current command
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, OFF);
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, OFF);
				mov_AZ_flag=0;
				mov_todo_flag=0;
				break;			
			
			case 1012:			// return AZ
				printf("+0%03d\r", posActual[0]);
				break;			
			
			case 1013:			// return EL
				printf("+0%03d\r", posActual[1]);
				break;			
			
			case 1014:			// return AZ and EL
				printf("+0%03d+0%03d\r", posActual[0], posActual[1]);
				break;
			
			case 1015:			// set AZ rotation speed
				break;
			
			case 1016:			// turn rotator AZ
				mov_todo_flag=0;
				mov_AZ_flag=1;
				break;
			
			case 1017:			// turn rotator AZ and EL
				mov_AZ_flag=0;
				mov_todo_flag=1;
				break;
			
			default:
				break;
		}
		datoComando=0; // después de hacer el comando lo borra
	}
}


// código principal del programa que se ejecuta en el núcleo principal
int main() {

	// Variables
	uint16_t inAZ; 			// unsigned short = 16 bits
	uint16_t inEL;
	uint16_t velAZ;
	uint32_t datoComando;

	// Lectura de comandos
	char cadena[MAX_CADENA];
	char *ptr;
	char espacio[] = " ";
	char comando[5];
	int difAZ;
	int difEL;
	int i;
	
	// Inicialización de entradas y salidas
	inicializacion(lcd, posActual, posDeseada, lecturas[0], &muestra, offsets);

	// inicia el segundo núcleo donde se procesa la lógica
	multicore_launch_core1(second_core_code);

	// prende dos veces el led para indicar el inicio.
	error(2);

	// ciclo infinito para estar a la espera de la lectura de comandos
	//   que reciba a través del USB conectado a la computadora.
	while(true){

		// almacena los caracteres hasta el salto de línea o retorno
		for(i=0; i < (MAX_CADENA-1); i++){
			cadena[i]=getchar();
			if (cadena[i]==10 || cadena[i]==13){
				cadena[i]=0;
				break;
			}
		}
		// elimina el salto de línea o retorno y lo cambia por el char 0
		if (i==(MAX_CADENA-1)){
			cadena[(MAX_CADENA-1)]=0;
		}

		i=strlen(cadena);
		
		// manda error si no hay caracteres válidos leídos.
		if (i==0){
			error(1);
			continue;
		}

		// separa la cadena leída por espacios
		ptr = strtok(cadena, espacio);
		if (ptr==NULL){
			//No leyó nada.
			error(1);
			continue;
		}
		// copia el primer token de la cadena
		strcpy(comando, ptr);

		
		// Lógica de flujo de instrucciones.
		//   determina el comando ingresado y sus parámetros correspondientes
		//   si algún dato es erróneo o inválido manda un error que se ve en
		//   el led. Si todo es correcto envía al segundo núcelo el valor del
		//   comando recibido para que este lo administre y procese.

		if(strcmp(comando, "O")==0){		// datoComando 1001
			// Offset calibration for internal AZ trimmer potentiometer
			datoComando=1001;
			multicore_fifo_push_blocking(datoComando);


		}else if(strcmp(comando, "O2")==0){	// datoComando 1002
			// Offset calibration for internal EL trimmer potentiometer
			datoComando=1002;
			multicore_fifo_push_blocking(datoComando);

		}else if(strcmp(comando, "F")==0){	// datoComando 1003
			// Full Scale calibration AZ
			datoComando=1003;
			multicore_fifo_push_blocking(datoComando);

		}else if(strcmp(comando, "F2")==0){	// datoComando 1004
			// Full Scale calibration EL
			datoComando=1004;
			multicore_fifo_push_blocking(datoComando);

		}else if(strcmp(comando, "R")==0){	// datoComando 1005
			// Right
			datoComando=1005;
			multicore_fifo_push_blocking(datoComando);

		}else if(strcmp(comando, "U")==0){	// datoComando 1006
			// Up 
			datoComando=1006;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "L")==0){	// datoComando 1007
			// Left 
			datoComando=1007;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "D")==0){	// datoComando 1008
			// Down 
			datoComando=1008;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "A")==0){	// datoComando 1009
			// Stop Azimuth rotation
			datoComando=1009;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "E")==0){	// datoComando 1010
			// Stop Elevation rotation
			datoComando=1010;
			multicore_fifo_push_blocking(datoComando);
			 
		}else if(strcmp(comando, "S")==0){	// datoComando 1011
			// Stop/cancel current command
			datoComando=1011;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "C")==0){	// datoComando 1012
			// Return current Azimuth angle in the form "+0nnn" degrees
			datoComando=1012;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "B")==0){	// datoComando 1013
			// Return current Elevation angle in the form "+0nnn" degrees
			datoComando=1013;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(strcmp(comando, "C2")==0){	// datoComando 1014
			// Return azimuth and elevation: "+0aaa+0eee"
			datoComando=1014;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(comando[0]=='X' && strlen(comando)==2){	// datoComando 1015
			// Select azimuth rotator turning speed, where n=1 (slowest) to n=4 (fastest)
			if(comando[1]<49 || comando[1]>52){
				// ERROR
				error(2);
				continue;
			}
			velAZ = comando[1]-48;
			datoComando=1015;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(comando[0]=='M' && strlen(comando)==4){	// datoComando 1016
			// Turn rotator to aaa degrees azimuth
			if(comando[1]<48 || comando[1]>57){
				// ERROR centenas AZ
				error(3);
				continue;
			}
			if(comando[2]<48 || comando[2]>57){
				// ERROR decenas AZ
				error(4);
				continue;
			}
			if(comando[3]<48 || comando[3]>57){
				// ERROR unidades AZ
				error(5);
				continue;
			}
			inAZ = (comando[1]-48)*100 + (comando[2]-48)*10 + (comando[3]-48);

			if(inAZ < 0 || inAZ > 360){
				// ERROR AZ fuera de rango
				error(6);
				continue;
			}
			posDeseada[0]=inAZ;
			
			datoComando=1016;
			multicore_fifo_push_blocking(datoComando);
			
		}else if(comando[0]=='W' && strlen(comando)==4){	// datoComando 1017
			// Turns to aaa degrees azimuth and eee elevation.
			if(comando[1]<48 || comando[1]>57){
				// ERROR centenas AZ
				error(7);
				continue;
			}
			if(comando[2]<48 || comando[2]>57){
				// ERROR decenas AZ
				error(8);
				continue;
			}
			if(comando[3]<48 || comando[3]>57){
				// ERROR unidades AZ
				error(9);
				continue;
			}
			inAZ = (comando[1]-48)*100 + (comando[2]-48)*10 + (comando[3]-48);

			ptr = strtok(NULL, espacio);
			strcpy(comando, ptr);

			if(strlen(comando)!=3){
				// ERROR longitud elevación
				error(10);
				continue;
			}
			if(comando[0]<48 || comando[0]>57){
				// ERROR centenas EL
				error(11);
				continue;
			}
			if(comando[1]<48 || comando[1]>57){
				// ERROR decenas EL
				error(12);
				continue;
			}
			if(comando[2]<48 || comando[2]>57){
				// ERROR unidades EL
				error(13);
				continue;
			}
			inEL = (comando[0]-48)*100 + (comando[1]-48)*10 + (comando[2]-48);
			
			if(inAZ < 0 || inAZ > 360){
				// ERROR fuera rango AZ
				error(14);
				continue;
			}
			if(inEL < 0 || inEL > 180){
				// ERROR fuera rango EL
				error(15);
				continue;
			}

			posDeseada[0]=inAZ;
			posDeseada[1]=inEL;

			datoComando=1017;
			multicore_fifo_push_blocking(datoComando);

		}else{
			//ERROR comando leído inválido.
			error(1);
			continue;
		}

	}

}