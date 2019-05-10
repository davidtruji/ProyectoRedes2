/*
 *  Creado el: 20/4/2019
 *      Curso: 2º
 *      Autor: David Trujillo Torres
 *		Autor: Alberto Diaz Martin
 */

#ifndef LIBFRC_H_
#define LIBFRC_H_
#include <iostream>
#include <conio.h>
#include <fstream>
#include "PuertoSerie.h"
#include "TramaControl.h"
#include "TramaDatos.h"


/**
 * Metodo que envia un mensaje mediante tramas de datos
 */
void enviarMensajePorTramas(HANDLE PuertoCOM, char vector[], int i);

/**
 * Metodo para el Envio de ficheros
 */
void enviarFichero(HANDLE PuertoCOM);

/**
 * Reenvia la trama de datos pasada por parametro a traves del puerto elegido
 */
void reenviarTramaDatos(HANDLE PuertoCOM, TramaDatos td);

/**
 * Metodo que envia el fichero Fenvio.txt
 */
void enviarFichero(HANDLE PuertoCOM);

/**
 * Metodo encargado de esperar la recepcion
 */
void recepcion(HANDLE PuertoCOM, int &numCampo, int &numTrama, TramaControl &t,
		TramaDatos &td, bool &esTramaControl, bool &esFichero, bool &finFichero,
		ofstream &flujoFichero);

/**
 * Menu para la eleccion del protocolo maestro/esclavo
 */
void seleccionMaestroEsclavo(HANDLE PuertoCOM);

/**
 * Menu del maestro para elegir Seleccion o Sondeo
 */
void seleccionMaestro(HANDLE PuertoCOM);

/**
 * Metodo de Seleccion para el protocolo Maestro/Esclavo
 */
void seleccion(HANDLE PuertoCOM);

/**
 * Metodo de Sondeo para el protocolo Maestro/Esclavo
 */
void sondeo(HANDLE PuertoCOM);

/**
 * Metodo que devuelve true cuando recibe ACKs y las muestra (Usado para Seleccion)
 */
bool recibirConfirmacionSeleccion(HANDLE PuertoCOM, int &campo,
		unsigned char num, TramaControl &t);

/**
 * Metodo que devuelve true cuando recibe ACKs y las muestra (Usado para Sondeo)
 */
bool recibirConfirmacionSondeo(HANDLE PuertoCOM, int &campo, unsigned char num,
		TramaControl &t);

/**
 * Metodo que se usa para esperar una ACK o NACK y reenvia tramas incorrectas (Seleccion y Sondeo)
 */
void esperarConfirmacion(HANDLE PuertoCOM, char dir, int trama,
		TramaControl& t,TramaDatos& td);

/**
 * Metodo que realiza el envio de ficheros del protocolo maestro/esclavo
 */
void enviarFicheroME(HANDLE PuertoCOM, unsigned char direccion);

/**
 * Metodo que se usa para la liberacion en el protocolo maestro/esclavo cuando usamos seleccion
 */
void liberacionSeleccion(HANDLE PuertoCOM);

/**
 * Metodo que se usa para la liberacion en el protocolo maestro/esclavo cuando usamos sondeo
 */
void solicitarCierreSondeo(HANDLE PuertoCOM);

/**
 * Metodo que se usa para recibir la respuesta (ACK o NACK) cuando se solicita el cierre en sondeo
 */
bool recibirCierreSondeo(HANDLE PuertoCOM, int& campo, unsigned char num,
		TramaControl &t);

/**
 * Menu para confirmar o denegar la solicitud de cierre en sondeo
 */
void responderSolicitudCierre(HANDLE PuertoCOM, unsigned char num);

/**
 * Metodo que devuelve true cuando recibe una trama ACK(Correcto) o NACK(Errores)
 */
bool recibirConfirmacionError(HANDLE PuertoCOM, int& campo, unsigned char dir,
		unsigned char num, TramaControl &t);

#endif /* LIBFRC_H_ */
