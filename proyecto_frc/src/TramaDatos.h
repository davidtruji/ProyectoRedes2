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

/**
 * Envia una trama de datos
 */
void enviarTramaDatos(HANDLE PuertoCOM, unsigned char dir, unsigned char num,
		char datos[], int numDatos);
void enviarTramaDatos(HANDLE PuertoCOM, TramaDatos td);

/**
 * Calcula la cantidad de tramas necesarias
 */
int calcularNumeroTramasDatos(int i);

/**
 * Calcula el valor de BCE
 */
unsigned char calcularBCE(char datos[], int l);

/**
 * Muestra la trama de datos
 */
void mostrarTramaDatos(TramaDatos td, bool enviada);
void mostrarTramaDatos(bool enviada, unsigned char dir, unsigned char num,
		int L, char Datos[], unsigned char BCE);
/**
 * Muestra el campo datos de la trama de datos
 */
void mostrarDatos(TramaDatos td);

#endif /* TRAMADATOS_H_ */
