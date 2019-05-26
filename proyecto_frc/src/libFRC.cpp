/*
 *  Creado el: 20/4/2019
 *      Curso: 2º
 *      Autor: David Trujillo Torres
 *		Autor: Alberto Diaz Martin
 */

#include "libFRC.h"
#define F5 63
#define FN '\0'
bool maestro = false, esclavo = false;

void enviarMensajePorTramas(HANDLE PuertoCOM, char vector[], int i) {
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

void recepcion(HANDLE PuertoCOM, int &numCampo, int &numTrama, TramaControl &t,
		TramaDatos &td, bool &esTramaControl, bool &esFichero, bool &finFichero,
		ofstream &flujoFichero) {
	char car = 0;
	car = RecibirCaracter(PuertoCOM);

	if (car != 0) {

		switch (numCampo) {
		case 1: // SYN & control de ficheros
			if (car == SYN) {
				t.S = car;
				td.S = car;
				numCampo++;

			} else if (car == '#') {
				esFichero = true;
				numTrama = 0;

			} else if (car == '@') {
				finFichero = true;
				flujoFichero.close();
				if (!maestro && !esclavo)
					printf("Fichero recibido\n");
			}

			break;

		case 2: // Direccion
			t.D = car;
			td.D = car;
			numCampo++;
			break;

		case 3: // Control
			t.C = car;
			td.C = car;
			if (car != STX)
				esTramaControl = true;
			else {
				numTrama++;
				esTramaControl = false;
			}
			numCampo++;
			break;

		case 4: // Numero de trama
			if (maestro || esclavo) {
				td.N = car;
				t.NT = car;

				if (esTramaControl) {
					numCampo = 1;
					mostrarTramaControl(t, false);

					if (t.D == 'R') {
						enviarTramaControl(PuertoCOM, 'R', ACK, '0');
					} else if (t.D == 'T' && esclavo) {
						enviarTramaControl(PuertoCOM, 'T', ACK, '0');
						//Confirmacion de sondeo
						enviarFicheroME(PuertoCOM, 'T');
						solicitarCierreSondeo(PuertoCOM);
						system("title FRC - Práctica de laboratorio 2018/19");
					}

					if (t.C == EOT) {

						if (esclavo) {
							esclavo = false;
							system(
									"title FRC - Práctica de laboratorio 2018/19");
						} else {
							responderSolicitudCierre(PuertoCOM, t.NT);
							system(
									"title FRC - Práctica de laboratorio 2018/19");

						}

					}

				} else {
					numCampo++;
				}
			} else {
				t.NT = car;
				td.N = car;
				if (esTramaControl) {
					numCampo = 1;
					mostrarTramaControl(t, false);
				} else
					numCampo++;

			}

			break;

		case 5: // Campo Longitud & Datos
			td.L = car;
			RecibirCadena(PuertoCOM, td.Datos, td.L);
			td.Datos[td.L + 1] = '\0';
			numCampo++;
			break;

		case 6: //Calculo de BCE & Mostrar/Escribir trama
			td.BCE = (unsigned char) car;
			numCampo = 1;
			if (esFichero) {

				if (maestro || esclavo) {
					if (td.BCE == calcularBCE(td.Datos, td.L)) { //Si el BCE va bien...

						if (numTrama == 1) {
							string s(td.Datos);
							flujoFichero.open(s);

						} else if (numTrama == 2) {
							//	printf("\nRecibiendo fichero por %s\n", td.Datos);
						} else if (finFichero) {
							//printf(
							//"El fichero recibido tiene un tamano de %s bytes\n", td.Datos
							//);
							finFichero = false;
							esFichero = false;
						} else
							flujoFichero.write(td.Datos, td.L);

						if (esclavo) {
							//Mostramos la trama de datos recibida
							mostrarTramaDatos(td, false);
							enviarTramaControl(PuertoCOM, 'R', ACK, td.N);

						} else if (maestro) {
							//Mostramos la trama de datos recibida
							mostrarTramaDatos(td, false);
							enviarTramaControl(PuertoCOM, 'T', ACK, td.N);
						}
						//printf("\n");

					} else {
						printf("*** ATENCION BCE INCORRECTO ***\n");
						// Mostrar trama datos aun siendo incorrecta...
						//Mostramos la trama de datos recibida
						mostrarTramaDatos(td, false);
						if (esclavo) {
							enviarTramaControl(PuertoCOM, 'R', NACK, td.N);
						} else if (maestro) {
							enviarTramaControl(PuertoCOM, 'T', NACK, td.N);
						}
						//printf("\n");
					}

				} else {
					if (td.BCE == calcularBCE(td.Datos, td.L)) {

						if (numTrama == 1) {
							string s(td.Datos);
							flujoFichero.open(s);

						} else if (numTrama == 2) {
							printf("\nRecibiendo fichero por %s\n", td.Datos);
						} else if (finFichero) {
							printf(
									"El fichero recibido tiene un tamano de %s bytes\n",
									td.Datos);
							finFichero = false;
							esFichero = false;
						} else
							flujoFichero.write(td.Datos, td.L);

					} else
						printf(
								"Error en la recepcion de la trama del fichero\n");

				}

			} else
				mostrarDatos(td);
			break;

		default:
			break;
		}

	}

}

void enviarFichero(HANDLE PuertoCOM) {
	ifstream flujoFicheroLectura;
	char fichero[255];
	string linea;
	flujoFicheroLectura.open("Fenvio.txt");
	int numCampo = 1, numDato = 0, numCar = 0, longitudFichero = 0;
	TramaControl t;
	TramaDatos td;
	bool esTramaControl = false, esFichero = false, finFichero = false;
	ofstream flujoFicheroEscritura;

	if (flujoFicheroLectura.is_open()) {

		//Enviamos caracter fichero '#' antes de la primera trama de datos
		EnviarCaracter(PuertoCOM, '#');

		//Lectura y envio de la cabecera del fichero
		getline(flujoFicheroLectura, linea, '\n'); //Primera linea nombre del fichero
		char nombreFichero[linea.length()];
		strcpy(nombreFichero, linea.c_str());
		nombreFichero[linea.length()] = '\0';
		enviarTramaDatos(PuertoCOM, 'T', '0', nombreFichero, linea.length());

		getline(flujoFicheroLectura, linea, '\n'); //Segunda linea autor
		char autor[linea.length()];
		strcpy(autor, linea.c_str());
		autor[linea.length()] = '\0';
		enviarTramaDatos(PuertoCOM, 'T', '0', autor, linea.length());
		printf("\nEnviando fichero por %s\n", autor);

		//Envio del resto del fichero
		while (!flujoFicheroLectura.eof()) {

			flujoFicheroLectura.read(fichero, 254);
			longitudFichero = flujoFicheroLectura.gcount();
			fichero[longitudFichero] = '\0';

			if (longitudFichero >= 1) { //Este if controla que no se envien tramas vacias
				numCar = numCar + longitudFichero; // Contar caracteres del fichero enviado

				enviarTramaDatos(PuertoCOM, 'T', '0', fichero, longitudFichero);

				recepcion(PuertoCOM, numCampo, numDato, t, td, esTramaControl,
						esFichero, finFichero, flujoFicheroEscritura);
			}
		}

		//Enviamos caracter fichero '@' despues de la ultima trama
		EnviarCaracter(PuertoCOM, '@');

		//Envio del numero caracteres del fichero enviado
		linea = to_string(numCar);
		char caracteres[linea.length()];
		strcpy(caracteres, linea.c_str());
		caracteres[linea.length()] = '\0';
		enviarTramaDatos(PuertoCOM, 'T', '0', caracteres, linea.length());

		flujoFicheroLectura.close();
		printf("Fichero enviado\n");

	} else
		printf("\nError al intentar abrir el fichero...\n");

}

void seleccionMaestroEsclavo(HANDLE PuertoCOM) {
	bool flag = false;
	char tecla;

	while (!flag) {
		printf("\n===== PROTOCOLO Maestro-Esclavo =====\n");
		printf("(1) - Maestro\n");
		printf("(2) - Esclavo\n");
		while (!kbhit()) {
		}

		flag = true;
		tecla = getch();

		switch (tecla) {
		case '1':
			printf("MODO MAESTRO\n");
			system(
					"title [Modo Maestro] FRC - Práctica de laboratorio 2018/19");
			maestro = true;
			esclavo = false;
			seleccionMaestro(PuertoCOM);
			break;
		case '2':
			printf("MODO ESCLAVO\n\n");
			system(
					"title [Modo Esclavo] FRC - Práctica de laboratorio 2018/19");
			esclavo = true;
			maestro = false;
			break;
		default:
			flag = false;
			printf("OPCION NO VALIDA\n");
			break;
		}

	}
}

void seleccionMaestro(HANDLE PuertoCOM) {
	bool flag = false;
	char tecla;
	while (!flag) {
		printf("\n===== MAESTRO: Elija una opcion; =====\n");
		printf("(1) - Seleccion\n");
		printf("(2) - Sondeo\n");

		while (!kbhit()) {
		}

		flag = true;
		tecla = getch();

		switch (tecla) {
		case '1':
			printf("SELECCION\n\n");
			seleccion(PuertoCOM); //Establecimiento
			enviarFicheroME(PuertoCOM, 'R'); //Tranferencia
			liberacionSeleccion(PuertoCOM); //Liberacion
			maestro = false;
			system("title FRC - Práctica de laboratorio 2018/19");
			break;
		case '2':
			printf("SONDEO\n\n");
			sondeo(PuertoCOM); //Establecimiento
			break;
		default:
			flag = false;
			printf("OPCION NO VALIDA\n");
			break;
		}

	}
}

void seleccion(HANDLE PuertoCOM) {
	bool seleccion = false;
	int campo = 1;
	TramaControl t;
	//Enviar llamada de seleccion
	enviarTramaControl(PuertoCOM, 'R', ENQ, '0');
	//Espera de confirmacion
	while (!seleccion)
		seleccion = recibirConfirmacionSeleccion(PuertoCOM, campo, '0', t);

	//printf("\n");
}

bool recibirConfirmacionSeleccion(HANDLE PuertoCOM, int &campo,
		unsigned char num, TramaControl &t) {
	char car = 0;
	car = RecibirCaracter(PuertoCOM);
	bool ack = false;

	if (car) {
		switch (campo) {
		case 1:
			if (car == SYN) {
				campo++;
				t.S = car;
			}
			break;
		case 2:
			if (car == 'R') {
				campo++;
				t.D = car;
			}
			break;
		case 3:
			if (car == ACK) {
				campo++;
				t.C = car;
			}
			break;
		case 4:
			if (car == num) {
				ack = true;
				t.NT = car;
				mostrarTramaControl(t, false);
			}
			campo = 1;

			break;
		}
	}
	return ack;

}

void enviarFicheroME(HANDLE PuertoCOM, unsigned char direccion) {
	ifstream flujoFicheroLectura;
	char fichero[255], tecla;
	string linea;
	flujoFicheroLectura.open("EProtoc.txt");
	int longitudFichero = 0, trama = 0, numCar = 0;
	TramaControl t;
	TramaDatos td;
	bool salir = false, error = false;
	ofstream flujoFicheroEscritura;

	if (flujoFicheroLectura.is_open()) {

		//Enviamos caracter fichero '#' antes de la primera trama de datos
		EnviarCaracter(PuertoCOM, '#');

		//Lectura y envio de la cabecera del fichero

		td.S = SYN; //Sincronismo = SYN =22
		td.D = direccion; //Direccion=(En principio fijo a ’T’)
		td.C = STX; //Control = STX = 02;
		td.N = '0';
		getline(flujoFicheroLectura, linea, '\n'); //Primera linea nombre del fichero
		td.L = linea.length();
		char nombreFichero[linea.length()];
		strcpy(nombreFichero, linea.c_str());
		nombreFichero[linea.length()] = '\0';
		strcpy(td.Datos, nombreFichero);
		td.BCE = calcularBCE(td.Datos, linea.length());
		enviarTramaDatos(PuertoCOM, direccion, '0', nombreFichero,
				linea.length());
		mostrarTramaDatos(true, direccion, '0', linea.length(), nombreFichero,
				calcularBCE(nombreFichero, linea.length()));
		esperarConfirmacion(PuertoCOM, direccion, trama, t, td);
		trama++;

		td.N = '1';
		getline(flujoFicheroLectura, linea, '\n'); //Segunda linea autor
		char autor[linea.length()];
		strcpy(autor, linea.c_str());
		autor[linea.length()] = '\0';
		strcpy(td.Datos, autor);
		td.BCE = calcularBCE(td.Datos, linea.length());
		enviarTramaDatos(PuertoCOM, direccion, '1', autor, linea.length());
		mostrarTramaDatos(true, direccion, '1', linea.length(), autor,
				calcularBCE(autor, linea.length()));
		esperarConfirmacion(PuertoCOM, direccion, trama, t, td);
		trama++;

		//Envio del resto del fichero
		while (!flujoFicheroLectura.eof() && !salir) {

			flujoFicheroLectura.read(fichero, 254);
			longitudFichero = flujoFicheroLectura.gcount();
			fichero[longitudFichero] = '\0';

			if (longitudFichero >= 1) {
				td.S = SYN; //Sincronismo = SYN =22
				td.D = direccion; //Direccion=(En principio fijo a ’T’)
				td.C = STX; //Control = STX = 02;
				if (trama % 2 == 0)
					td.N = '0'; //NumTrama = (0 en pares);
				else
					td.N = '1'; //NumTrama = (1 en impares);
				td.L = longitudFichero; //Enviamos el campo LONG

				//Campo de datos
				for (int i = 0; i < longitudFichero; i++) {
					if (error && i == 0) {
						td.Datos[i] = 'ç';
						error = false;
					} else
						td.Datos[i] = fichero[i];
				}

				td.BCE = calcularBCE(fichero, td.L); //Calculo del BCE
				numCar = numCar + longitudFichero; // Contar caracteres del fichero enviado

				enviarTramaDatos(PuertoCOM, td);
				mostrarTramaDatos(td, true);

				td.Datos[0] = fichero[0]; //Esto restablece el valor original si se metió el 'ç' del error
				td.BCE = calcularBCE(fichero, td.L);
				esperarConfirmacion(PuertoCOM, direccion, trama, t, td);

				trama++;

				//SIMULACION DE ERROR EN TRAMA Y TECLA ESC
				if (kbhit()) {
					tecla = getch();
					if (tecla == 27) {
						salir = true;
						printf("\nESC PRESIONADO SALIENDO...\n");
					}
					if (tecla == FN) { //Comprobamos si es una tecla de Función
						tecla = getch();
						if (tecla == F5) {
							printf("*** INTRODUCIDO ERROR EN TRAMA ***\n");
							error = true;
						}
					}

				}
			}
		}

		//Enviamos caracter fichero '@' despues de la ultima trama
		EnviarCaracter(PuertoCOM, '@');

		//Envio del numero caracteres del fichero enviado
		linea = to_string(numCar);
		char caracteres[linea.length()];
		strcpy(caracteres, linea.c_str());
		caracteres[linea.length()] = '\0';
		if (trama % 2 == 0) {
			enviarTramaDatos(PuertoCOM, direccion, '0', caracteres,
					linea.length());
			mostrarTramaDatos(true, direccion, '0', linea.length(), caracteres,
					calcularBCE(caracteres, linea.length()));
		} else {
			enviarTramaDatos(PuertoCOM, direccion, '1', caracteres,
					linea.length());
			mostrarTramaDatos(true, direccion, '1', linea.length(), caracteres,
					calcularBCE(caracteres, linea.length()));
		}
		esperarConfirmacion(PuertoCOM, direccion, trama, t, td);

		flujoFicheroLectura.close();
		//printf("Fichero enviado\n");

	} else
		printf("\nError al intentar abrir el fichero...\n");

}

void sondeo(HANDLE PuertoCOM) {
	bool sondeo = false;
	int campo = 1;
	TramaControl t;
	//Enviar llamada de sondeo
	enviarTramaControl(PuertoCOM, 'T', ENQ, '0');
	//Espera de confirmacion
	while (!sondeo)
		sondeo = recibirConfirmacionSondeo(PuertoCOM, campo, '0', t);

	//printf("\n");
}

bool recibirConfirmacionSondeo(HANDLE PuertoCOM, int& campo, unsigned char num,
		TramaControl &t) {
	char car = 0;
	car = RecibirCaracter(PuertoCOM);
	bool ack = false;

	if (car) {
		switch (campo) {
		case 1:
			if (car == SYN) {
				campo++;
				t.S = car;
			}
			break;
		case 2:
			if (car == 'T') {
				campo++;
				t.D = car;
			}
			break;
		case 3:
			if (car == ACK) {
				campo++;
				t.C = car;
			}
			break;
		case 4:
			if (car == num) {
				ack = true;
				t.NT = car;
				mostrarTramaControl(t, false);
			}
			campo = 1;

			break;
		}
	}
	return ack;

}

void liberacionSeleccion(HANDLE PuertoCOM) {
	bool liberacion = false;
	int campo = 1;
	TramaControl t;
	enviarTramaControl(PuertoCOM, 'R', EOT, '0');
	while (!liberacion)
		liberacion = recibirConfirmacionSeleccion(PuertoCOM, campo, '0', t);

	//printf("\n Se ha confirmado liberacion\n");
}

void solicitarCierreSondeo(HANDLE PuertoCOM) {
	bool cierre = false;
	bool recibido = false;
	int intento = 0, campo = 1;
	TramaControl t;

	while (!cierre) {
		if (intento % 2 == 0)
			enviarTramaControl(PuertoCOM, 'T', EOT, '0');
		else
			enviarTramaControl(PuertoCOM, 'T', EOT, '1');

		while (!recibido) {
			if (intento % 2 == 0) {
				recibido = recibirCierreSondeo(PuertoCOM, campo, '0', t);
			} else {
				recibido = recibirCierreSondeo(PuertoCOM, campo, '1', t);
			}

		}

		recibido = false;
		if (t.C == NACK)
			intento++;
		else
			cierre = true;

	}

}

bool recibirCierreSondeo(HANDLE PuertoCOM, int& campo, unsigned char num,
		TramaControl &t) {

	char car = 0;

	car = RecibirCaracter(PuertoCOM);

	bool recibido = false;

	if (car) {
		switch (campo) {
		case 1:
			if (car == SYN) {
				campo++;

				t.S = car;
			}

			break;
		case 2:
			if (car == 'T') {
				t.D = car;
				campo++;
			}
			break;
		case 3:
			if (car == ACK || car == NACK) {
				campo++;
				t.C = car;

			}

			break;
		case 4:
			if (car == num) {
				t.NT = car;

				mostrarTramaControl(t, false);

				recibido = true;
			}
			campo = 1;

			break;
		}
	}
	return recibido;

}

void responderSolicitudCierre(HANDLE PuertoCOM, unsigned char num) {
	bool opcion = false;
	char tecla;

	while (!opcion) {
		printf("\n===== CONFIRMACION DE CIERRE: Elija una opcion; =====\n");
		printf("(1) - ACEPTAR\n");
		printf("(2) - RECHAZAR\n");
		while (!kbhit()) {
		}

		opcion = true;
		tecla = getch();

		switch (tecla) {
		case '1':
			printf("ACEPTADO\n");
			enviarTramaControl(PuertoCOM, 'T', ACK, num);
			break;
		case '2':
			printf("RECHAZADO\n");
			enviarTramaControl(PuertoCOM, 'T', NACK, num);
			break;
		default:
			opcion = false;
			printf("OPCION NO VALIDA\n");
			break;
		}
	}
}

void esperarConfirmacion(HANDLE PuertoCOM, char dir, int trama, TramaControl& t,
		TramaDatos& td) {
	int numCampo = 1;
	bool ack = false, tramaRecibida = false;
	if (dir == 'R') {

		while (!ack) {
			while (!tramaRecibida) {
				if (trama % 2 == 0)
					tramaRecibida = recibirConfirmacionError(PuertoCOM,
							numCampo, 'R', '0', t);
				else
					tramaRecibida = recibirConfirmacionError(PuertoCOM,
							numCampo, 'R', '1', t);

			}
			tramaRecibida = false;

			if (t.C == NACK) {
				enviarTramaDatos(PuertoCOM, td);
				mostrarTramaDatos(td, true);
			} else {
				ack = true;
			}

		}
		ack = false;
	} else {
		while (!ack) {
			while (!tramaRecibida) {
				if (trama % 2 == 0)
					tramaRecibida = recibirConfirmacionError(PuertoCOM,
							numCampo, 'T', '0', t);
				else
					tramaRecibida = recibirConfirmacionError(PuertoCOM,
							numCampo, 'T', '1', t);

			}
			tramaRecibida = false;

			if (t.C == NACK) {
				enviarTramaDatos(PuertoCOM, td);
				mostrarTramaDatos(td, true);
			} else {
				ack = true;
			}

		}
		ack = false;
	}

}

bool recibirConfirmacionError(HANDLE PuertoCOM, int& campo, unsigned char dir,
		unsigned char num, TramaControl& t) {
	char car = 0;

	car = RecibirCaracter(PuertoCOM);

	bool recibido = false;

	if (car) {
		switch (campo) {
		case 1:
			if (car == SYN) {
				campo++;

				t.S = car;
			}

			break;
		case 2:
			if (car == dir) {
				t.D = car;
				campo++;
			}
			break;
		case 3:
			if (car == ACK || car == NACK) {
				campo++;
				t.C = car;

			}

			break;
		case 4:
			if (car == num) {
				t.NT = car;

				mostrarTramaControl(t, false);

				recibido = true;
			}
			campo = 1;

			break;
		}
	}
	return recibido;
}
