/*
 * TramaDatos.h
 *
 *  Created on: 3/3/2019
 *      Autor: David Trujillo Torres
 *		Autor: Alberto Diaz Martin
 */

#ifndef TRAMADATOS_H_
#define TRAMADATOS_H_

#include <iostream>
#include <fstream>
#include "PuertoSerie.h"
#include "TramaControl.h"
using namespace std;

#define STX 02

struct TramaDatos {
	unsigned char S; //Sincronismo = SYN =22;
	unsigned char D; //Direccion=’T’;
	unsigned char C; //Control = STX = 02;
	unsigned char N; //NumTrama = (En principio fijo a ‘0’);
	unsigned char L; //Long (Longitud del campo de datos);
	char Datos[255]; //Datos[255];
	unsigned char BCE; //(Entre 1 y 254);
};

void enviarTramaDatos(HANDLE PuertoCOM, char vector[], int i, bool msj);
int calcularNumeroTramasDatos(int i);
unsigned char calcularBCE(char datos[], int l);
void mostrarTramaDatos(TramaDatos td);
void enviarFichero(HANDLE PuertoCOM);
void recepcion(HANDLE PuertoCOM, int &numCampo, int &numTrama, TramaControl &t,
		TramaDatos &td, bool &esTramaControl, bool &esFichero,
		ofstream &flujoFichero);

#endif /* TRAMADATOS_H_ */
