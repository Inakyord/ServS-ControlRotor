// **************************************************************************************************************
// ****  Código funciones.cpp: código fuente de las funciones a utilizar.                                    ****
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

#include "funciones.h"

int Edo = -1;		// Estado del display (para evitar reimprimir - parpadeo)
int anteUSB = 0;	// estado anterior de la conexion usb (evita reimprimir)


// muestra a través del flasheo de un usb algún error
// (también se usa al iniciar)
void error(int n){
	int i;
	for (i=0; i<n; i++){
		gpio_put(ERROR_PIN, ON);
		sleep_ms(T_ERROR);
		gpio_put(ERROR_PIN, OFF);
		sleep_ms(T_ERROR);
	}
}


// inicializa todas las entradas y salidas del microcontrolador,
//   realiza las primeras lecturas de posicionamiento,
//   inicializa el display y espera por conexión usb si se escoge
//   el modo automático
void inicializacion(LiquidCrystal &lcd, uint16_t* posActual, uint16_t* posDeseada, uint16_t* lecturas, int* muestra, uint16_t* offsets){
	int anterior=0;
	int i;

	stdio_init_all();
	sleep_ms(500);
	adc_init();
	sleep_ms(500);
	// pines adc
	adc_gpio_init(AZ_PIN);
	adc_gpio_init(EL_PIN);
	adc_gpio_init(OFFSET_PIN);
	// pines movimiento
	gpio_init(R_PIN);
	gpio_init(U_PIN);
	gpio_init(L_PIN);
	gpio_init(D_PIN);
	gpio_set_dir(R_PIN, GPIO_OUT);
	gpio_set_dir(U_PIN, GPIO_OUT);
	gpio_set_dir(L_PIN, GPIO_OUT);
	gpio_set_dir(D_PIN, GPIO_OUT);
	// pin led error
	gpio_init(ERROR_PIN);
	gpio_set_dir(ERROR_PIN, GPIO_OUT);
	// pines display
	gpio_init(DRS_PIN);
	gpio_init(DE_PIN);
	gpio_init(D4_PIN);
	gpio_init(D5_PIN);
	gpio_init(D6_PIN);
	gpio_init(D7_PIN);
	gpio_set_dir(DRS_PIN, GPIO_OUT);
	gpio_set_dir(DE_PIN, GPIO_OUT);
	gpio_set_dir(D4_PIN, GPIO_OUT);
	gpio_set_dir(D5_PIN, GPIO_OUT);
	gpio_set_dir(D6_PIN, GPIO_OUT);
	gpio_set_dir(D7_PIN, GPIO_OUT);
	// pines debug
	gpio_init(0);
	gpio_init(1);
	gpio_set_dir(0,GPIO_OUT);
	gpio_set_dir(1,GPIO_OUT);
	// pines control manual
	gpio_init(MD_PIN);
	gpio_init(MR_PIN);
	gpio_init(ML_PIN);
	gpio_init(MU_PIN);
	gpio_init(MAN_PIN);
	gpio_init(USB_PIN);
	gpio_set_dir(MD_PIN, GPIO_IN);
	gpio_set_dir(MR_PIN, GPIO_IN);
	gpio_set_dir(ML_PIN, GPIO_IN);
	gpio_set_dir(MU_PIN, GPIO_IN);
	gpio_set_dir(MAN_PIN, GPIO_IN);
	gpio_set_dir(USB_PIN, GPIO_IN);
	gpio_pull_up(MD_PIN);
	gpio_pull_up(MR_PIN);
	gpio_pull_up(ML_PIN);
	gpio_pull_up(MU_PIN);
	gpio_pull_up(MAN_PIN);
	gpio_pull_down(USB_PIN);
	
	// estado inicial de los pines:
	gpio_put(R_PIN, OFF);
	gpio_put(U_PIN, OFF);
	gpio_put(L_PIN, OFF);
	gpio_put(D_PIN, OFF);
	gpio_put(ERROR_PIN, OFF);
	gpio_put(DRS_PIN, OFF);
	gpio_put(DE_PIN, OFF);
	gpio_put(D4_PIN, OFF);
	gpio_put(D5_PIN, OFF);
	gpio_put(D6_PIN, OFF);
	gpio_put(D7_PIN, OFF);

	stdio_flush();
	sleep_ms(500);

	
	// primeras lecturas promedio móvil de posición
	for (i=0; i<MUESTRAS; i++){
		adc_select_input(0);
		lecturas[i]=adc_read();
		adc_select_input(1);
		lecturas[MUESTRAS+i]=adc_read();
		sleep_ms(200);
	}

	// inicialización display
	lcd.begin(16, 2);
	lcd.print("Sin conexion USB");


	// espera por conexión usb en modo automático
	//   y maneja los movimientos del modo manual
	while(!stdio_usb_connected() || (gpio_get(USB_PIN)==0)){
		get_Pos(posActual,lecturas,muestra, offsets);
		if(gpio_get(MAN_PIN)==0){
			anterior=1;
			dibujarDisplay(lcd, posActual, posDeseada, 1, 0, 0);
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
		}else{
			gpio_put(R_PIN, OFF);
			gpio_put(L_PIN, OFF);
			gpio_put(U_PIN, OFF);
			gpio_put(D_PIN, OFF);
			if(anterior){
				anterior=0;
				Edo = -1;
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("Sin conexion USB");
			}
		}
		sleep_ms(100);
	}

	// una vez se conecta el usb en modo automático continúa
	gpio_put(R_PIN, OFF);
	gpio_put(L_PIN, OFF);
	gpio_put(U_PIN, OFF);
	gpio_put(D_PIN, OFF);
	sleep_ms(500);
	stdio_usb_init();
	lcd.clear();
	lcd.setCursor(0,1);
	lcd.print("Conectando...");
	sleep_ms(1000);

}


// convierte las lecturas digitales a grados de posición
int mapeo(long x, long in_min, long in_max, long out_min, long out_max) {
	int resultado = (int)round((float)((x-in_min) * (out_max - out_min)) / (in_max - in_min) + out_min);
	if (resultado<0){
		return 0;
	}
	return resultado;
}


// Realiza una lectura de la posición actual y actualiza el promedio móvil.
void get_Pos(uint16_t* posActual, uint16_t* lecturas, int* muestra, uint16_t* offsets){

	uint32_t sumaAZ=0;
	uint32_t sumaEL=0;
	uint32_t sumaOff=0;
	uint16_t resAZ;
	uint16_t resEL;
	int gradosAZ;
	int gradosEL;
	int i, offset;

	adc_select_input(0);
	lecturas[*muestra]=adc_read();
	adc_select_input(1);
	lecturas[MUESTRAS+(*muestra)]=adc_read();
	adc_select_input(2);
	offsets[*muestra]=adc_read();

	for(i=0; i<MUESTRAS; i++){
		sumaEL += lecturas[i];
		sumaAZ += lecturas[MUESTRAS+i];
		sumaOff += offsets[i];
	}

	resAZ = round((float)sumaAZ / MUESTRAS);
	resEL = round((float)sumaEL / MUESTRAS);
	offset = round((float)sumaOff / MUESTRAS);
	gradosAZ = mapeo(resAZ, offset, 4095, 0, 360);
	gradosEL = mapeo(resEL, offset, 4095, 0, 180);

	posActual[0]=(gradosAZ+180)%360;
	posActual[1]=gradosEL;

	(*muestra)++;
	(*muestra)%=MUESTRAS;

	sleep_ms(100);

}


// controla el movimiento automático del rotor según lo que el procesamiento de instrucciones indique
void movimientoRotor(uint16_t* posActual, uint16_t* posDeseada, int mov_todo_flag, int mov_AZ_flag){
	int difAZ;
	int difEL;
	int umbral=3;	// valor aceptado de diferencia entre pos actual y pos deseada

	// posiciona sólo azimuth
	if (mov_AZ_flag) {

		difAZ = fabs(posActual[0] - posDeseada[0]);
		if (difAZ > (360-umbral)){ // corrección por distancia de 360° a 1°.
			difAZ = 360 - difAZ;
		}

		if(difAZ < umbral ){
			mov_AZ_flag=0;
			gpio_put(R_PIN, OFF);
			gpio_put(L_PIN, OFF);
		}else{

			if (posDeseada[0] <= 180){
				if (posActual[0] > 180 ){
					gpio_put(L_PIN, OFF);
					gpio_put(R_PIN, ON);
				} else if (posActual[0] < posDeseada[0]){
					gpio_put(L_PIN, OFF);
					gpio_put(R_PIN, ON);
				} else{
					gpio_put(R_PIN, OFF);
					gpio_put(L_PIN, ON);
				}
			}else{
				if(posActual[0] <= 180){
					gpio_put(R_PIN, OFF);
					gpio_put(L_PIN, ON);
				}else if (posActual[0] > posDeseada[0]){
					gpio_put(R_PIN, OFF);
					gpio_put(L_PIN, ON);
				}else{
					gpio_put(L_PIN, OFF);
					gpio_put(R_PIN, ON);
				}
			}
		}
	}

	// posiciona azimuth y elevación
	if (mov_todo_flag) {

		difAZ = fabs(posActual[0] - posDeseada[0]);
		if (difAZ > (360-umbral)){ // corrección por distancia de 360° a 1°.
			difAZ = 360 - difAZ;
		}
		difEL = fabs(posActual[1] - posDeseada[1]);

		if(difAZ < umbral && difEL < umbral){
			mov_todo_flag=0;
			gpio_put(R_PIN, OFF);
			gpio_put(L_PIN, OFF);
			gpio_put(U_PIN, OFF);
			gpio_put(D_PIN, OFF);
		}else{
			// mover azimuth
			if (difAZ >= umbral){
				if (posDeseada[0] <= 180){
					if (posActual[0] > 180 ){
						gpio_put(L_PIN, OFF);
						gpio_put(R_PIN, ON);
					} else if (posActual[0] < posDeseada[0]){
						gpio_put(L_PIN, OFF);
						gpio_put(R_PIN, ON);
					} else{
						gpio_put(R_PIN, OFF);
						gpio_put(L_PIN, ON);
					}
				}else{ // posDeseada[0] > 180
					if(posActual[0] <= 180){
						gpio_put(R_PIN, OFF);
						gpio_put(L_PIN, ON);
					}else if (posActual[0] > posDeseada[0]){
						gpio_put(R_PIN, OFF);
						gpio_put(L_PIN, ON);
					}else{
						gpio_put(L_PIN, OFF);
						gpio_put(R_PIN, ON);
					}
				}
			}else{
				gpio_put(R_PIN, OFF);
				gpio_put(L_PIN, OFF);
			}
			// mover elevación
			if(difEL >= umbral){
				if(posActual[1] > posDeseada[1]){
					gpio_put(U_PIN, OFF);
					gpio_put(D_PIN, ON);
				}else{
					gpio_put(D_PIN, OFF);
					gpio_put(U_PIN, ON);
				}
			}else{
				gpio_put(U_PIN, OFF);
				gpio_put(D_PIN, OFF);
			}
		}
	}
}


// Imprime en el display, tratando de evitar reimpresiones idénticas, para que no parpadé mucho.
void dibujarDisplay(LiquidCrystal &lcd, uint16_t* posActual, uint16_t* posDeseada, int manual, int todo, int azimuth){
	
	if(manual==1){
		if(Edo!=0){
			lcd.setCursor(0,0);
			lcd.print("M  AZ:     (---)");
			lcd.setCursor(0,1);
			lcd.print("   EL:     (---)");
			Edo = 0;
			anteUSB=0;
		}
		lcd.setCursor(7,0);
		lcd.print("   ");
		lcd.setCursor(7,0);
		lcd.print(String(posActual[0]));
		lcd.setCursor(7,1);
		lcd.print("   ");
		lcd.setCursor(7,1);
		lcd.print(String(posActual[1]));
	}else if(!stdio_usb_connected() || (gpio_get(USB_PIN)==0)){
		if(Edo!=1){
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Sin conexion USB");	
			Edo = 1;
			anteUSB=0;
		}
		return;
	}else if(azimuth==1){
		if(Edo!=2){
			lcd.setCursor(0,0);
			lcd.print("A  AZ:     (   )");
			lcd.setCursor(0,1);
			lcd.print("   EL:     (---)");
			Edo = 2;
			anteUSB=0;
		}
		lcd.setCursor(7,0);
		lcd.print("   ");
		lcd.setCursor(7,0);
		lcd.print(String(posActual[0]));
		lcd.setCursor(12,0);
		lcd.print("   ");
		lcd.setCursor(12,0);
		lcd.print(String(posDeseada[0]));
		lcd.setCursor(7,1);
		lcd.print("   ");
		lcd.setCursor(7,1);
		lcd.print(String(posActual[1]));
	}else if(todo==1){
		if(Edo!=3){
			lcd.setCursor(0,0);
			lcd.print("A  AZ:     (   )");
			lcd.setCursor(0,1);
			lcd.print("   EL:     (   )");
			Edo = 3;
			anteUSB=0;
		}
		lcd.setCursor(7,0);
		lcd.print("   ");
		lcd.setCursor(7,0);
		lcd.print(String(posActual[0]));
		lcd.setCursor(12,0);
		lcd.print("   ");
		lcd.setCursor(12,0);
		lcd.print(String(posDeseada[0]));
		lcd.setCursor(7,1);
		lcd.print("   ");
		lcd.setCursor(7,1);
		lcd.print(String(posActual[1]));
		lcd.setCursor(12,1);
		lcd.print("   ");
		lcd.setCursor(12,1);
		lcd.print(String(posDeseada[1]));
	}else{
		if(Edo!=4){
			lcd.setCursor(0,0);
			lcd.print("A  AZ:     (---)");
			lcd.setCursor(0,1);
			lcd.print("   EL:     (---)");
			Edo=4;
			anteUSB=0;
		}
		lcd.setCursor(7,0);
		lcd.print("   ");
		lcd.setCursor(7,0);
		lcd.print(String(posActual[0]));
		lcd.setCursor(7,1);
		lcd.print("   ");
		lcd.setCursor(7,1);
		lcd.print(String(posActual[1]));
	}

	if (gpio_get(USB_PIN)!=0){
		if(anteUSB!=1){
			lcd.setCursor(0,1);
			lcd.print("U");
			anteUSB=1;
		}
	}else{
		if(anteUSB!=0){
			lcd.setCursor(0,1);
			lcd.print(" ");
			anteUSB=0;
		}
	}

	return;

}