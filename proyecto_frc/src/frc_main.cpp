//============================================================================
// ----------- PRÁCTICAS DE FUNDAMENTOS DE REDES DE COMUNICACIONES -----------
// ---------------------------- CURSO 2018/19 --------------------------------
// ----------------------------- MAIN.CPP ------------------------------------
//============================================================================
//    Autor: David Trujillo Torres
// 	  Autor: Alberto Diaz Martin
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <iostream>
#include "PuertoSerie.h"
#include "TramaControl.h"
#include "TramaDatos.h"

using namespace std;

const int MAX = 700; //Numero maximo de caracters que se permitiran leer
HANDLE PuertoCOM;

#define ESC 27
#define BACK 8
#define INTRO 13
#define F1 59
#define F2 60
#define F3 61
#define F4 62
#define F5 63
#define FN '\0'

/**
 * Muestra el menu para elegir el puerto en serie y abre el elegido
 */
void elegirPuerto() {

	// Parámetros necesarios al llamar a AbrirPuerto:
	// - Nombre del puerto a abrir ("COM1", "COM2", "COM3", ...).
	// - Velocidad (1200, 1400, 4800, 9600, 19200, 38400, 57600, 115200).
	// - Número de bits en cada byte enviado o recibido (4, 5, 6, 7, 8).
	// - Paridad (0=sin paridad, 1=impar, 2=par, 3=marca, 4=espacio).
	// - Bits de stop (0=1 bit, 1=1.5 bits, 2=2 bits)
	bool opcion = false;
	char tecla;

	while (!opcion) {
		printf(
				"\nUse la teclas: 1, 2, 3, o 4 para abrir el Puerto COM correspondiente.\n");

		while (!kbhit()) {
		}

		opcion = true;
		tecla = getch();

		switch (tecla) {
		case '1':
			PuertoCOM = AbrirPuerto("COM1", 9600, 8, 0, 0);
			break;
		case '2':
			PuertoCOM = AbrirPuerto("COM2", 9600, 8, 0, 0);
			break;
		case '3':
			PuertoCOM = AbrirPuerto("COM3", 9600, 8, 0, 0);
			break;
		case '4':
			PuertoCOM = AbrirPuerto("COM4", 9600, 8, 0, 0);
			break;
		default:
			printf("Tecla no valida, vuelva a intentarlo...\n");
			opcion = false;
			break;
		}

		if (PuertoCOM != NULL) {
			printf("Puerto COM %c abierto correctamente\n\n", tecla);
		} else
			opcion = false;

	}

}

/**
 * Metodo que ejecuta las funciones del programa dependiendo de la tecla presionada
 * F1(Envia una trama de datos), F2(Envia una trama de control), F3(Envia un fichero), F4(Inicia el Protocolo Maesto/Esclavo)
 */
void envio(char vector[], int &i) {
	char car;

	if (kbhit()) {
		car = getch();

		switch (car) {

		case F1:
			enviarMensajePorTramas(PuertoCOM, vector, i);
			printf("%c", '\n');
			i = 0;
			break;
		case F2:
			seleccionarTramaControl(PuertoCOM);
			break;
		case F3:
			enviarFichero(PuertoCOM);
			break;

		default:
			break;
		}

	}

}

/**
 * Metodo que ejecuta el retroceso
 */
void retroceso(int &i) {
	if (i > 0) {
		i--;
		printf("\b");
		printf(" ");
		printf("\b");
	}

}

/**
 * Metodo que ejecuta el salto de linea
 */
void salto(int &i, char vector[]) {
	if (i < MAX) {
		vector[i] = '\n';
		printf("%c", vector[i]);
		i++;
	}
}

int main() {
	system("title FRC - Práctica de laboratorio 2018/19");
	char vector[MAX + 2];
	int i = 0, numTrama = 0, campoTrama = 1;
	TramaControl t;
	TramaDatos td;
	char car = 0;
	ofstream flujoFichero;
	bool esTramaControl = false, esFichero = false, finFichero = false;

	elegirPuerto();
	printf(
			"\n F1 - ENVIO DE TRAMA DE DATOS.\n F2 - ENVIO DE TRAMA DE CONTROL.\n F3 - ENVIO DE FICHERO.\n F4 - PROTOCOLO MAESTRO-ESCLAVO.\n  *F5 - SIMULA ERROR EN PROTOCOLO MAESTRO-ESCLAVO.\n");

	// Lectura y escritura simultánea de caracteres:
	while (car != ESC) {

		recepcion(PuertoCOM, campoTrama, numTrama, t, td, esTramaControl,
				esFichero, finFichero, flujoFichero);

		if (kbhit()) {
			car = getch();

			switch (car) {
			case BACK:       //Retroceso.
				retroceso(i);
				break;
			case INTRO:       //Salto de línea.
				salto(i, vector);
				break;
			case FN:       //Envio.
				envio(vector, i);
				break;
			case ESC:
				break;
			default:       //salida por pantalla.
				if (i < MAX) {
					vector[i] = car;
					printf("%c", car);
					i++;
				}
				break;
			}

		}

	}

	// Para cerrar el puerto:
	CerrarPuerto(PuertoCOM);

	return 0;
}
