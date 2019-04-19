/*
 * TramaDatos.cpp
 *
 *  Created on: 3/3/2019
 *      Autor: David Trujillo Torres
 *		Autor: Alberto Diaz Martin
 */

#include "TramaDatos.h"

void enviarTramaDatos(HANDLE PuertoCOM, char vector[], int i) {
	int numTramasDatos;
	int c;
	int indice = 0;
	char datos[255];
	vector[i] = '\n'; //Anadimos \n en el final del mensaje e incrementamos el tamano de este
	i++;

	numTramasDatos = calcularNumeroTramasDatos(i);

	for (int j = 0; j < numTramasDatos; j++) {
		EnviarCaracter(PuertoCOM, SYN); //Sincronismo = SYN =22
		EnviarCaracter(PuertoCOM, 'T'); //Direccion=(En principio fijo a ’T’)
		EnviarCaracter(PuertoCOM, STX); //Control = STX = 02;
		EnviarCaracter(PuertoCOM, '0'); //NumTrama = (En principio fijo a ‘0’);

		if (i >= 254)
			EnviarCaracter(PuertoCOM, 254); //Si la trama es de mas de 254 caracteres, se enviaran 254 que es el maximo
		else
			EnviarCaracter(PuertoCOM, i); //Si la trama es de menos de 254 caracteres, se enviaran el valor de i que contiene lo que queda por enviar

		for (c = 0; c < i && c < 254; c++) {
			datos[c] = vector[indice];
			indice++;
		}
		EnviarCadena(PuertoCOM, datos, c);

		i = i - c;

		EnviarCaracter(PuertoCOM, calcularBCE(datos, c)); //Calculo y envio del BCE

	}

}

int calcularNumeroTramasDatos(int i) {
	int numTramasDatos = 0;

	if (i < 254 || i == 254)
		numTramasDatos = 1;
	else {
		numTramasDatos = i / 254;
		if (i % 254 != 0)
			numTramasDatos++;
	}
	return numTramasDatos;
}

void recepcion(HANDLE PuertoCOM, int &numCampo, int &numDato, TramaControl &t,
		TramaDatos &td, bool &esTramaControl, bool &esFichero,
		ofstream &flujoFichero) {
	char car = 0;
	car = RecibirCaracter(PuertoCOM);

	if (car != 0) {

		switch (numCampo) {
		case 1:
//			if (car == SYN) {
//				t.S = car;
//				td.S = car;
//				numCampo++;
//			}

			if (car == SYN) {
				t.S = car;
				td.S = car;
				numCampo++;
			} else if (car == '$') {
				esFichero = true;
				flujoFichero.open("FRC-R.txt");
				printf("\nRecibiendo fichero\n");

			} else if (car == '#') {
				esFichero = false;
				flujoFichero.close();
				printf("\nFichero recibido\n");
			}

			break;

		case 2:
			t.D = car;
			td.D = car;
			numCampo++;
			break;

		case 3:
			t.C = car;
			td.C = car;
			if (car != STX)
				esTramaControl = true;
			else
				esTramaControl = false;

			numCampo++;
			break;

		case 4:
			t.NT = car;
			td.N = car;
			if (esTramaControl) {
				numCampo = 1;
				mostrarTramaControl(t, true);
			} else
				numCampo++;
			break;

		case 5:
			td.L = car;
			RecibirCadena(PuertoCOM, td.Datos, td.L);
			td.Datos[td.L + 1] = '\0';
			numCampo++; //TODO: Procesar cadena entera en vez de caracter a caracter.
			break;

		case 6:
			td.BCE = car;
			numCampo = 1;
			if (esFichero) {
				if (flujoFichero.is_open()) {
					if (td.BCE == calcularBCE(td.Datos, td.L))
						flujoFichero.write(td.Datos, td.L);
					else
						printf("\nError al recibir trama BCE incorrecto...\n");

				} else
					printf("\nError al intentar escribir en fichero...\n");
			} else
				mostrarTramaDatos(td);
			break;

		default:
			break;
		}

	}

}

void enviarFichero(HANDLE PuertoCOM) {
	printf("\nEnviando fichero\n");
	int c;
	char datos[255];
	ifstream flujoFicheroLectura;
	int longitudFichero = 0;
	char fichero[255];
	flujoFicheroLectura.open("Fenvio.txt");

	int numCampo = 1, numDato = 0;
	TramaControl t;
	TramaDatos td;
	bool esTramaControl = false, esFichero = false;
	ofstream flujoFicheroEscritura;

	if (flujoFicheroLectura.is_open()) {

		//Enviamos caracter fichero '$' antes de la primera trama de datos
		EnviarCaracter(PuertoCOM, '$');

		while (!flujoFicheroLectura.eof()) {

			flujoFicheroLectura.read(fichero, 254);
			longitudFichero = flujoFicheroLectura.gcount();
			fichero[longitudFichero] = '\0';

			EnviarCaracter(PuertoCOM, SYN); //Sincronismo = SYN =22
			EnviarCaracter(PuertoCOM, 'T'); //Direccion=(En principio fijo a ’T’)
			EnviarCaracter(PuertoCOM, STX); //Control = STX = 02;
			EnviarCaracter(PuertoCOM, '0'); //NumTrama = (En principio fijo a ‘0’);

			//Enviamos el campo LONG
			EnviarCaracter(PuertoCOM, longitudFichero);

			//Enviamos Campo de datos
			for (c = 0; c < longitudFichero && c < 254; c++) {

				EnviarCaracter(PuertoCOM, fichero[c]);
				datos[c] = fichero[c];
			}

			EnviarCaracter(PuertoCOM, calcularBCE(datos, c)); //Calculo y envio del BCE

			recepcion(PuertoCOM, numCampo, numDato, t, td, esTramaControl,
					esFichero, flujoFicheroEscritura);

		}
		//Enviamos caracter fichero '#' despues de la ultima trama
		EnviarCaracter(PuertoCOM, '#');

		flujoFicheroLectura.close();
		printf("\nFichero enviado\n");
	} else
		printf("\nError al intentar abrir el fichero...\n");

}

//void recepcion2(HANDLE PuertoCOM, int &numCampo, int &numDato, TramaControl &t,
//		TramaDatos &td, bool &esTramaControl, bool &esFichero,
//		ofstream &flujoFichero) {
//	char car = 0;
//	car = RecibirCaracter(PuertoCOM);
//
//	if (car != 0) {
//
//		switch (numCampo) {
//		case 1:
//			if (car == SYN) {
//				t.S = car;
//				td.S = car;
//				numCampo++;
//			} else if (car == '$') {
//				esFichero = true;
//				flujoFichero.open("FRC-R.txt");
//				printf("\nRecibiendo fichero\n");
//
//			} else if (car == '#') {
//				esFichero = false;
//				flujoFichero.close();
//				printf("\nFichero recibido\n");
//			}
//
//			break;
//		case 2:
//			t.D = car;
//			td.D = car;
//			numCampo++;
//
//			break;
//		case 3:
//			t.C = car;
//			td.C = car;
//			if (car != STX)
//				esTramaControl = true;
//			else
//				esTramaControl = false;
//
//			numCampo++;
//
//			break;
//		case 4:
//			t.NT = car;
//			td.N = car;
//			if (esTramaControl) {
//				numCampo = 1;
//				mostrarTramaControl(t);
//			} else
//				numCampo++;
//
//			break;
//		case 5:
//			td.L = car;
//			numCampo++;
//
//			break;
//
//		case 6:
//
//			td.Datos[numDato] = car;
//			if (numDato < td.L - 1)
//				numDato++;
//			else {
//				td.Datos[numDato + 1] = '\0';
//				numCampo++;
//				numDato = 0;
//
//			}
//
//			break;
//		case 7:
//			td.BCE = car;
//			numCampo = 1;
//			if (esFichero) {
//				if (flujoFichero.is_open()) {
//					if (td.BCE == calcularBCE(td.Datos, td.L))
//						flujoFichero.write(td.Datos, td.L);
//					else
//						printf("\nError al recibir trama BCE incorrecto...\n");
//
//				} else
//					printf("\nError al intentar escribir en fichero...\n");
//			} else
//				mostrarTramaDatos(td);
//
//			break;
//		default:
//			break;
//		}
//
//	}
//
//}

void mostrarTramaDatos(TramaDatos td) {
	if (td.BCE == calcularBCE(td.Datos, td.L)) {
		for (int i = 0; i < td.L; i++)
			printf("%c", td.Datos[i]);

	} else {
		printf("\nSe ha recibido una trama de datos defectuosa\n");

	}

}

unsigned char calcularBCE(char datos[], int l) {

	unsigned char bce = datos[0];

	for (int i = 1; i < l; i++) {
		bce = bce ^ datos[i];
	}

	if (bce == 255 || bce == 0)
		bce = 1;

	return bce;
}
