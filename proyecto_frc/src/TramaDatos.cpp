/*
 * TramaDatos.cpp
 *
 *  Created on: 3/3/2019
 *      Autor: David Trujillo Torres
 *		Autor: Alberto Diaz Martin
 */

#include "TramaDatos.h"

void enviarTramaDatos(HANDLE PuertoCOM, unsigned char dir, unsigned char num,
		char datos[], int numDatos) {

	EnviarCaracter(PuertoCOM, SYN); //Sincronismo = SYN =22
	EnviarCaracter(PuertoCOM, dir); //Direccion=(En principio fijo a ’T’)
	EnviarCaracter(PuertoCOM, STX); //Control = STX = 02;
	EnviarCaracter(PuertoCOM, num); //NumTrama = (En principio fijo a ‘0’);

	if (numDatos >= 254)
		EnviarCaracter(PuertoCOM, 254); //Si la trama es de mas de 254 caracteres, se enviaran 254 que es el maximo
	else
		EnviarCaracter(PuertoCOM, numDatos); //Si la trama es de menos de 254 caracteres, se enviaran el valor de i que contiene lo que queda por enviar

	//Envio de datos
	EnviarCadena(PuertoCOM, datos, numDatos);

	EnviarCaracter(PuertoCOM, calcularBCE(datos, numDatos)); //Calculo y envio del BCE

}

void enviarTramaDatos(HANDLE PuertoCOM, TramaDatos td) {
	int L;
	EnviarCaracter(PuertoCOM, td.S); //Sincronismo = SYN =22
	EnviarCaracter(PuertoCOM, td.D); //Direccion=(En principio fijo a ’T’)
	EnviarCaracter(PuertoCOM, td.C); //Control = STX = 02;
	EnviarCaracter(PuertoCOM, td.N); //NumTrama = (En principio fijo a ‘0’);
	EnviarCaracter(PuertoCOM, td.L); //NumTrama = (En principio fijo a ‘0’);
	L = (int) td.L;
	EnviarCadena(PuertoCOM, td.Datos, L); //Envio del campo long
	EnviarCaracter(PuertoCOM, td.BCE); // BCE
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

void mostrarTramaDatos(TramaDatos td, bool enviada) {
	char envioRecivo;
//R R STX 0 122 122
//E R STX 0 122

	if (enviada) {
		envioRecivo = 'E';
		printf("[%c] [%c] [ STX ] [%c] [%d]\n", envioRecivo, td.D, td.N,
				td.BCE);
	} else {
		envioRecivo = 'R';
		printf("[%c] [%c] [ STX ] [%c] [%d] [%d]\n", envioRecivo, td.D, td.N,
				td.BCE, calcularBCE(td.Datos, td.L));
	}

//		for (int i = 0; i < td.L; i++)
//			printf("%c", td.Datos[i]);

}

void mostrarTramaDatos(bool enviada, unsigned char dir, unsigned char num,
		int L, char Datos[], unsigned char BCE) {

	char envioRecivo;

	if (enviada) {
		envioRecivo = 'E';
		printf("[%c] [%c] [ STX ] [%c] [%d]\n", envioRecivo, dir, num, BCE);
	} else {
		envioRecivo = 'R';
		printf("[%c] [%c] [ STX ] [%c] [%d] [%d]\n", envioRecivo, dir, num, BCE,
				calcularBCE(Datos, L));
	}

}

void mostrarDatos(TramaDatos td) {
	if (td.BCE == calcularBCE(td.Datos, td.L)) {
		for (int i = 0; i < td.L; i++)
			printf("%c", td.Datos[i]);

	} else {
		printf("\nSe ha recibido una trama de datos defectuosa\n");

	}

}

//void mostrarTramaDatos(TramaDatos td) {
//	if (td.BCE == calcularBCE(td.Datos, td.L)) {
//		for (int i = 0; i < td.L; i++)
//			printf("%c", td.Datos[i]);
//
//	} else {
//		printf("\nSe ha recibido una trama de datos defectuosa\n");
//
//	}
//
//}

unsigned char calcularBCE(char datos[], int l) {

	unsigned char bce = datos[0];

	for (int i = 1; i < l; i++) {
		bce = bce ^ datos[i];
	}

	if (bce == 255 || bce == 0)
		bce = 1;

	return bce;
}
