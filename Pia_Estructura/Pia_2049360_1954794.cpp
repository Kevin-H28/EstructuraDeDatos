#include<iostream>
#include<Windows.h>
#include<cstring>
#include "resource.h"
#include<Commctrl.h> //Libreria para usar los controles comunes de DateTimePicker
#pragma comment(lib, "Comctl32.lib") //Para poder usar la liberria de Common Controls
#include<ctime> //Libreria para usar la fecha y hora
#include<cstdio> //Libreria para guardar y cargar archivos binarios
#include<vector> //Libreria para usar vectores (para busqueda binaria)

using namespace std;

//Declaraciones globales usadas para el inicio de sesion
char usuarioActivo[50] = "";
HINSTANCE hInst;

//Funcion global para validar que las cedulas y los id sean numeros
bool esNumero(const char* cadena) {
	for (int i = 0; cadena[i] != '\0'; i++) {
		if (!isdigit(cadena[i]))
			return false;
	}
	return true;
}

//Estructura para almacenar los datos de un m�dico
struct Medico
{
	int cedula;
	char nombre[50]; //Nombre(s)
	char apellidoPaterno[50]; //Apellido paterno
	char apellidoMaterno[50]; //Apellido materno
	char correo[50];
	char telefono[20];
	char especialidad[50];
	Medico* ante;
	Medico* sig;

	//Constructor para inicializar los miembros de la estructura
	Medico(int c, const char* n, const char* ap, const char* am, const char* co, const char* t, const char* esp) {
		cedula = c;
		strcpy_s(nombre, n);
		strcpy_s(apellidoPaterno, ap);
		strcpy_s(apellidoMaterno, am);
		strcpy_s(correo, co);
		strcpy_s(telefono, t);
		strcpy_s(especialidad, esp);
		ante = sig = nullptr;
	}

	//Constructor para usar al leer el archivo
	Medico() {
		cedula = 0;
		nombre[0] = '\0';
		apellidoPaterno[0] = '\0';
		apellidoMaterno[0] = '\0';
		correo[0] = '\0';
		telefono[0] = '\0';
		especialidad[0] = '\0';
		ante = sig = nullptr;
	}
};

//Lista doblemente ligada para m�dicos
class ListaMedicos
{
public:
	Medico* cabeza;
	Medico* cola;

	ListaMedicos() : cabeza(nullptr), cola(nullptr) {}

	//M�todo para agregar un m�dico a la lista
	void agregarMedico(int cedula, const char* nombre, const char* ap, const char* am, const char* correo, const char* telefono, const char* especialidad) {
		Medico* nuevo = new Medico(cedula, nombre, ap, am, correo, telefono, especialidad);
		if (!cabeza) {
			cabeza = cola = nuevo;
		}
		else {
			cola->sig = nuevo;
			nuevo->ante = cola;
			cola = nuevo;
		}
	}

	//M�todo para buscar un m�dico por su n�mero de c�dula
	Medico* buscarMedico(int cedula) {
		Medico* temp = cabeza;
		while (temp) {
			if (temp->cedula == cedula)
				return temp;
			temp = temp->sig;
		}
		return nullptr;
	}

	//M�todo para verificar si una c�dula ya existe en la lista
	bool existeCedula(int cedula) {
		Medico* temp = cabeza;
		while (temp) {
			if (temp->cedula == cedula)
				return true;
			temp = temp->sig;
		}
		return false;
	}

	//M�todo para obtener el nombre completo de un m�dico a partir de su c�dula
	string buscarNombrePorCedula(int cedula) {
		Medico* temp = cabeza;
		while (temp) {
			if (temp->cedula == cedula) {
				return string(temp->apellidoPaterno) + " " + temp->apellidoMaterno + " " + temp->nombre;
			}
			temp = temp->sig;
		}
		return "Desconocido";
	}

	//Metodo para saber si la especialidad que se quiere eliminar esta en uso
	bool especialidadEstaEnUso(const char* nombreEspecialidad) {
		Medico* temp = cabeza;
		while (temp) {
			if (strcmp(temp->especialidad, nombreEspecialidad) == 0)
				return true;
			temp = temp->sig;
		}
		return false;
	}

	//M�todo para ordenar la lista de m�dicos por apellido paterno utilizando HeapSort
	void heapSortPorApellido() {
		int n = 0;
		Medico* temp = cabeza;
		while (temp) {
			n++;
			temp = temp->sig;
		}
		if (n <= 1) return;

		Medico** arr = new Medico * [n];
		temp = cabeza;
		for (int i = 0; i < n; i++) {
			arr[i] = temp;
			temp = temp->sig;
		}

		for (int i = n / 2 - 1; i >= 0; i--) {
			heapify(arr, n, i);
		}
		for (int i = n - 1; i > 0; i--) {
			std::swap(arr[0], arr[i]);
			heapify(arr, i, 0);
		}

		cabeza = arr[0];
		Medico* actual = cabeza;
		for (int i = 1; i < n; i++) {
			arr[i - 1]->sig = arr[i];
			arr[i]->ante = arr[i - 1];
		}
		cola = arr[n - 1];
		cola->sig = nullptr;
		delete[] arr;
	}

	//M�todo para guardar los m�dicos en archivo binario
	void guardarEnArchivo(const char* archivoMedicos) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoMedicos, "wb") != 0) {
			cerr << "Error al abrir el archivo para guardar m�dicos." << endl;
			return;
		}
		Medico* actual = cabeza;
		while (actual) {
			if (fwrite(actual, sizeof(Medico), 1, archivo) != 1) {
				cerr << "Error al escribir en el archivo de m�dicos." << endl;
				fclose(archivo);
				return;
			}
			actual = actual->sig;
		}
		fclose(archivo);
	}

	//M�todo para cargar los m�dicos desde archivo binario
	void cargarDesdeArchivo(const char* archivoMedicos) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoMedicos, "rb") != 0) {
			cerr << "Error al abrir el archivo para cargar m�dicos." << endl;
			return;
		}
		Medico temp;
		while (fread(&temp, sizeof(Medico), 1, archivo)) {
			agregarMedico(temp.cedula, temp.nombre, temp.apellidoPaterno, temp.apellidoMaterno, temp.correo, temp.telefono, temp.especialidad);
		}
		if (ferror(archivo)) {
			cerr << "Error al leer el archivo de m�dicos." << endl;
		}
		fclose(archivo);
	}

private:
	//M�todo auxiliar para Heapify
	void heapify(Medico** arr, int n, int i) {
		int mayor = i;
		int izq = 2 * i + 1;
		int der = 2 * i + 2;
		if (izq < n && strcmp(arr[izq]->apellidoPaterno, arr[mayor]->apellidoPaterno) > 0)
			mayor = izq;
		if (der < n && strcmp(arr[der]->apellidoPaterno, arr[mayor]->apellidoPaterno) > 0)
			mayor = der;
		if (mayor != i) {
			std::swap(arr[i], arr[mayor]);
			heapify(arr, n, mayor);
		}
	}
};
ListaMedicos listaMedicos;

//Estructura para almacenar los datos de un paciente
struct Paciente
{
	int id;
	char nombre[50]; //Nombre(s)
	char apellidoPaterno[50]; //Apellido paterno
	char apellidoMaterno[50]; //Apellido materno
	char correo[50];
	char telefono[11];
	char genero[10];
	char edad[10];
	Paciente* ante;
	Paciente* sig;

	//Constructor para inicializar los miembros de la estructura
	Paciente(int i, const char* n, const char* ap, const char* am, const char* co, const char* t, const char* g, int e) {
		id = i;
		strcpy_s(nombre, n);
		strcpy_s(apellidoPaterno, ap);
		strcpy_s(apellidoMaterno, am);
		strcpy_s(correo, co);
		strcpy_s(telefono, t);
		strcpy_s(genero, g);
		sprintf_s(edad, sizeof(edad), "%d", e); // Convertir edad a cadena
		ante = sig = nullptr;
	}

	//Constructor para usar al leer el archivo
	Paciente() {
		id = 0;
		nombre[0] = '\0';
		apellidoPaterno[0] = '\0';
		apellidoMaterno[0] = '\0';
		correo[0] = '\0';
		telefono[0] = '\0';
		genero[0] = '\0';
		edad[0] = '\0';
		ante = sig = nullptr;
	}
};
//Lista doblemente ligada para pacientes
class ListaPacientes
{
public:
	Paciente* cabeza;
	Paciente* cola;

	ListaPacientes() :cabeza(nullptr), cola(nullptr) {}

	void agregarPaciente(int id, const char* nombre, const char* ap, const char* am, const char* correo, const char* telefono, const char* genero, int edad) {
		Paciente* nuevo = new Paciente(id, nombre, ap, am, correo, telefono, genero, edad);
		if (!cabeza) //Si la lista esta vacia
		{
			cabeza = cola = nuevo;
		}
		else {
			cola->sig = nuevo;
			nuevo->ante = cola;
			cola = nuevo;
		}
	}

	void mostrarPacientes() {
		Paciente* temp = cabeza; //Puntero temporal para recorrer la lista
		while (temp != nullptr) {
			cout << "ID: " << temp->id << ", Nombre: " << temp->apellidoPaterno << " " << temp->apellidoMaterno << " " << temp->nombre << ", Edad: " << temp->edad << endl;
			temp = temp->sig;
		}
	}

	bool existeID(int id) {
		Paciente* temp = cabeza;
		while (temp != nullptr) {
			if (temp->id == id)
				return true;
			temp = temp->sig;
		}
		return false;
	}

	string buscarNombrePorId(int id) {
		Paciente* actual = cabeza;
		while (actual) {
			if (actual->id == id) {
				string nombreCompleto = string(actual->apellidoPaterno) + " " + actual->apellidoMaterno + " " + actual->nombre;
				return nombreCompleto;
			}
			actual = actual->sig;
		}
		return "Desconocido";
	}

	Paciente* buscarPacientePorID(int id) {
		Paciente* temp = cabeza;
		while (temp != nullptr) {
			if (temp->id == id) {
				return temp;
			}
			temp = temp->sig;
		}
		return nullptr;
	}

	//Funcion para ordenar la lista de pacientes por apellido paterno (HEAPSORT)
	void heapSortPorApellido() {
		//Contar el numero de nodos
		int n = 0;
		Paciente* temp = cabeza;
		while (temp) {
			n++;
			temp = temp->sig;
		}
		if (n <= 1) //Si ya esta ordenada
		{
			return;
		}
		//Crear un arreglo para almacenar los nodos (copia la lista a un arreglo para trabajar con heapsort)
		Paciente** arr = new Paciente * [n];
		temp = cabeza;
		for (int i = 0; i < n; i++) {
			arr[i] = temp;
			temp = temp->sig;
		}
		//Construir el heap maximo
		for (int i = n / 2 - 1; i >= 0; i--) {
			heapify(arr, n, i);
		}
		//Extraer elementos del heap uno por uno
		for (int i = n - 1; i > 0; i--) {
			swap(arr[0], arr[i]); //Intercambiar el primer elemento con el ultimo
			heapify(arr, i, 0); //Llamar a heapify en el heap reducido
		}
		cabeza = arr[0]; //Actualizar la cabeza de la lista
		Paciente* actual = cabeza;
		for (int i = 1; i < n; i++) {
			arr[i - 1]->sig = arr[i]; //Actualizar el siguiente del nodo anterior
			arr[i]->ante = arr[i - 1]; //Actualizar el anterior del nodo actual
		}
		cola = arr[n - 1]; //Actualizar la cola de la lista
		cola->sig = nullptr; //Actualizar el siguiente del ultimo nodo a null
		delete[] arr; //Liberar memoria del arreglo
	}
	//Funcion auxiliar para heapify
	void heapify(Paciente** arr, int n, int i) { //Recursivo
		int mayor = i; // Inicializar el mayor como raiz
		int izq = 2 * i + 1; // Izquierda = 2*i + 1
		int der = 2 * i + 2; // Derecha = 2*i + 2
		if (izq < n && strcmp(arr[izq]->apellidoPaterno, arr[mayor]->apellidoPaterno) > 0) //Si el hijo izquierdo es mayor que la raiz
			mayor = izq;
		if (der < n && strcmp(arr[der]->apellidoPaterno, arr[mayor]->apellidoPaterno) > 0) //Si el hijo derecho es mayor que la raiz
			mayor = der;
		if (mayor != i) { //Si el mayor no es la raiz
			swap(arr[i], arr[mayor]); //Intercambiar la raiz con el mayor
			heapify(arr, n, mayor); //Recursivamente heapify el subarbol afectado
		}
	}

	//Funcion para guardar archivo de pacientes en pacientes.dat
	void guardarEnArchivo(const char* archivoPacientes) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoPacientes, "wb") != 0) {
			cerr << "Error al abrir el archivo para guardar pacientes." << endl;
			return;
		}
		Paciente* actual = cabeza;
		while (actual) { //Recorre la lista
			if (fwrite(actual, sizeof(Paciente), 1, archivo) != 1) { //Verifica si se escribio correctamente
				cerr << "Error al escribir en el archivo de pacientes." << endl;
				fclose(archivo);
				return;
			}
			actual = actual->sig;
		}
		fclose(archivo);
	}

	//Funcion para cargar archivo de pacientes desde pacientes.dat
	void cargarDesdeArchivo(const char* archivoPacientes) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoPacientes, "rb") != 0) {
			cerr << "Error al abrir el archivo para cargar pacientes." << endl;
			return;
		}
		Paciente temp;
		while (fread(&temp, sizeof(Paciente), 1, archivo)) { //Lee el archivo
			agregarPaciente(temp.id, temp.nombre, temp.apellidoPaterno, temp.apellidoMaterno, temp.correo, temp.telefono, temp.genero, atoi(temp.edad));
		}
		if (ferror(archivo)) //Verifica si hubo un error al leer el archivo
		{
			cerr << "Error al leer el archivo de pacientes." << endl;
		}
		fclose(archivo);
	}
};
ListaPacientes listaPacientes;
//Funcion global para hacer busqueda binaria de pacientes por apellido Paterno
vector<Paciente*> buscarPacientesPorApellido(const char* apellidoBuscado) {
	vector<Paciente*> resultados; //Vector para almacenar los resultados

	//Copiar la lista a un arreglo de punteros para aplicar busqueda binaria
	vector<Paciente*> arreglo;
	Paciente* actual = listaPacientes.cabeza;
	while (actual) {
		arreglo.push_back(actual); 
		actual = actual->sig;
	}

	int izquierda = 0;
	int derecha = arreglo.size() - 1;
	int encontrado = -1;

	//Hacer busqueda binaria
	while (izquierda <= derecha) {
		int medio = (izquierda + derecha) / 2;
		int cmp = strcmp(arreglo[medio]->apellidoPaterno, apellidoBuscado); //Comparar el apellido buscado con el del medio

		if (cmp == 0) {
			encontrado = medio;
			break;
		}
		else if (cmp < 0) {
			izquierda = medio + 1;
		}
		else {
			derecha = medio - 1;
		}
	}

	//Si no se encontro ninguno, retornar vacio
	if (encontrado == -1) return resultados;

	//Recorrer hacia la izquierda desde el indice encontrado
	int i = encontrado;
	while (i >= 0 && strcmp(arreglo[i]->apellidoPaterno, apellidoBuscado) == 0) {
		i--;
	}
	i++; //Avanzar uno porque se paso al anterior

	//Recorrer hacia la derecha y guardar todos los que coincidan
	while (i < arreglo.size() && strcmp(arreglo[i]->apellidoPaterno, apellidoBuscado) == 0) {
		resultados.push_back(arreglo[i]); //Agregar a la lista de resultados
		i++;
	}

	return resultados;
}

//Estructura para almacenar los datos de una especialidad
struct Especialidad
{
	char nombre[50];
	Especialidad* ante;
	Especialidad* sig;

	//Constructor para inicializar los miembros de la estructura
	Especialidad(const char* n) {
		strcpy_s(nombre, n);
		ante = sig = nullptr;
	}

	//Constructor para usar al leer el archivo
	Especialidad() {
		nombre[0] = '\0';
		ante = sig = nullptr;
	}
};
//Lista doblemente ligada para especialidades
class ListaEspecialidades
{
public:
	Especialidad* cabeza;
	Especialidad* cola;

	ListaEspecialidades() :cabeza(nullptr), cola(nullptr) {}

	void agregarEspecialidad(const char* nombre) {
		Especialidad* nueva = new Especialidad(nombre);
		if (!cabeza) //Si la lista esta vacia
		{
			cabeza = cola = nueva;
		}
		else {
			cola->sig = nueva;
			nueva->ante = cola;
			cola = nueva;
		}
	}

	void eliminarEspecialidad(Especialidad* e) {
		if (e->ante)
		{
			e->ante->sig = e->sig;
		}
		else
		{
			cabeza = e->sig; //Actualizar la cabeza de la lista
		}
		if (e->sig)
		{
			e->sig->ante = e->ante;
		}
		else
		{
			cola = e->ante; //Actualizar la cola de la lista
		}
		delete e; //Liberar memoria
	}

	bool existeEspecialidad(const char* nombre) {
		Especialidad* temp = cabeza;
		while (temp != nullptr) {
			if (strcmp(temp->nombre, nombre) == 0)
				return true;
			temp = temp->sig;
		}
		return false;
	}

	//Funcion para ordenar la lista de especialidades por nombre (QUICKSORT)
	void quickSortPorNombre() {
		//Contar el numero de nodos
		int n = 0;
		Especialidad* temp = cabeza;
		while (temp) {
			n++;
			temp = temp->sig;
		}
		if (n <= 1) return;

		//Crear un arreglo para almacenar los nodos (copia la lista a un arreglo para trabajar con quicksort)
		Especialidad** arr = new Especialidad * [n];
		temp = cabeza;
		for (int i = 0; i < n; i++) {
			arr[i] = temp;
			temp = temp->sig;
		}

		//Aplicar el algoritmo quicksort
		quickSort(arr, 0, n - 1);

		//Reconstruir lista ligada a partir del arreglo ordenado
		cabeza = arr[0];
		cabeza->ante = nullptr;
		for (int i = 1; i < n; i++) {
			arr[i - 1]->sig = arr[i];
			arr[i]->ante = arr[i - 1];
		}
		cola = arr[n - 1];
		cola->sig = nullptr;

		delete[] arr;
	}
	//Funcion auxiliar para quicksort
	void quickSort(Especialidad** arr, int menor, int mayor) { //Recursivo
		if (menor < mayor) {
			int pi = particion(arr, menor, mayor); //Particiona el arreglo
			quickSort(arr, menor, pi - 1); //Recursivamente ordena la parte izquierda
			quickSort(arr, pi + 1, mayor); //Recursivamente ordena la parte derecha
		}
	}

	int particion(Especialidad** arr, int menor, int mayor) { //Funcion para particionar el arreglo
		Especialidad* pivot = arr[mayor]; //El pivote es el ultimo elemento
		int i = menor - 1; //Indice del elemento menor

		for (int j = menor; j < mayor; j++) {
			if (strcmp(arr[j]->nombre, pivot->nombre) < 0) { //Si el elemento actual es menor que el pivote
				i++; //Incrementar el indice del elemento menor
				swap(arr[i], arr[j]); //Intercambiar arr[i] y arr[j]
			}
		}
		swap(arr[i + 1], arr[mayor]); //Intercambiar arr[i + 1] y arr[mayor] (pivote)
		return i + 1; //Retorna el indice del pivote
	}

	//Funcion para guardar archivo de especialidades en especialidades.dat
	void guardarEnArchivo(const char* archivoEspecialidades) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoEspecialidades, "wb") != 0) {
			cerr << "Error al abrir el archivo para guardar especialidades." << endl;
			return;
		}
		Especialidad* actual = cabeza;
		while (actual) { //Recorre la lista
			if (fwrite(actual, sizeof(Especialidad), 1, archivo) != 1) { //Verifica si se escribio correctamente
				cerr << "Error al escribir en el archivo de especialidades." << endl;
				fclose(archivo);
				return;
			}
			actual = actual->sig;
		}
		fclose(archivo);
	}

	//Funcion para cargar archivo de especialidades desde especialidades.dat
	void cargarDesdeArchivo(const char* archivoEspecialidades) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoEspecialidades, "rb") != 0) {
			cerr << "Error al abrir el archivo para cargar especialidades." << endl;
			return;
		}
		Especialidad temp;
		while (fread(&temp, sizeof(Especialidad), 1, archivo)) { //Lee el archivo
			agregarEspecialidad(temp.nombre);
		}
		if (ferror(archivo)) //Verifica si hubo un error al leer el archivo
		{
			cerr << "Error al leer el archivo de especialidades." << endl;
		}
		fclose(archivo);
	}
};
ListaEspecialidades listaEspecialidades;

//Estructura para almacenar los datos de un consultorio
struct DisponibilidadConsultorio
{
	int numConsultorio;
	char dia[15];
	char horaInicio[6];
	char horaFin[6];
	DisponibilidadConsultorio* ante;
	DisponibilidadConsultorio* sig;

	//Constructor para inicializar los miembros de la estructura
	DisponibilidadConsultorio(int nc, const char* d, const char* hi, const char* hf) {
		numConsultorio = nc;
		strcpy_s(dia, d);
		strcpy_s(horaInicio, hi);
		strcpy_s(horaFin, hf);
		ante = sig = nullptr;
	}

	//Constructor para usar al leer el archivo
	DisponibilidadConsultorio() {
		numConsultorio = 0;
		dia[0] = '\0';
		horaInicio[0] = '\0';
		horaFin[0] = '\0';
		ante = sig = nullptr;
	}
};
//Funcion auxiliar para convertir dia a numero (para ordenamiento)
int valorDia(const char* dia) {
	if (strcmp(dia, "Domingo") == 0) return 0;
	if (strcmp(dia, "Lunes") == 0) return 1;
	if (strcmp(dia, "Martes") == 0) return 2;
	if (strcmp(dia, "Mi�rcoles") == 0) return 3;
	if (strcmp(dia, "Jueves") == 0) return 4;
	if (strcmp(dia, "Viernes") == 0) return 5;
	if (strcmp(dia, "S�bado") == 0) return 6;
	return 7; // Valor alto por si no coincide
}
//Funcion auxiliar para comparar dia y hora (para ordenamiento)
bool esMenor(DisponibilidadConsultorio* a, DisponibilidadConsultorio* b) {
	int diaA = valorDia(a->dia);
	int diaB = valorDia(b->dia);

	if (diaA != diaB)
		return diaA < diaB;

	//Si es el mismo dia, comparar por hora
	return strcmp(a->horaInicio, b->horaInicio) < 0;
}
//Lista doblemente ligada para disponibilidad de consultorios
class ListaConsultorios
{
public:
	DisponibilidadConsultorio* cabeza;
	DisponibilidadConsultorio* cola;

	ListaConsultorios() :cabeza(nullptr), cola(nullptr) {}

	void agregarBloque(int Consultorio, const char* dia, const char* horaInicio, const char* horaFin) {
		DisponibilidadConsultorio* nuevo = new DisponibilidadConsultorio(Consultorio, dia, horaInicio, horaFin);
		if (!cabeza) //Si la lista esta vacia
		{
			cabeza = cola = nuevo;
		}
		else {
			cola->sig = nuevo;
			nuevo->ante = cola;
			cola = nuevo;
		}
	}

	void mostrarDisponibilidad() {
		DisponibilidadConsultorio* temp = cabeza; //Puntero temporal para recorrer la lista
		while (temp != nullptr) {
			cout << "Consultorio: " << temp->numConsultorio << ", Dia: " << temp->dia << ", Hora Inicio: " << temp->horaInicio << ", Hora Fin: " << temp->horaFin << endl;
			temp = temp->sig;
		}
	}

	void eliminarBloque(DisponibilidadConsultorio* e) {

		if (e->ante) //Si anterior no es nulo
		{
			e->ante->sig = e->sig; //Actualizar el puntero del anterior
		}
		else
		{
			cabeza = e->sig; //Actualizar la cabeza de la lista
		}
		if (e->sig) //Si siguiente no es nulo
		{
			e->sig->ante = e->ante; //Actualizar el puntero del siguiente
		}
		else
		{
			cola = e->ante; //Actualizar la cola de la lista
		}
		delete e; //Liberar memoria
	}

	bool existeBloque(int consultorio, const char* dia, const char* horaInicio) {
		DisponibilidadConsultorio* temp = cabeza;
		while (temp != nullptr) {
			if (temp->numConsultorio == consultorio &&
				strcmp(temp->dia, dia) == 0 &&
				strcmp(temp->horaInicio, horaInicio) == 0)
			{
				return true;
			}
			temp = temp->sig;
		}
		return false;
	}

	//Buscar  bloque para eliminar
	DisponibilidadConsultorio* buscarBloque(int numConsultorio, const char* dia, const char* horaInicio) {
		DisponibilidadConsultorio* temp = cabeza;
		while (temp) {
			if (temp->numConsultorio == numConsultorio &&
				strcmp(temp->dia, dia) == 0 &&
				strcmp(temp->horaInicio, horaInicio) == 0) {
				return temp;
			}
			temp = temp->sig;
		}
		return nullptr;
	}

	//Funcion para ordenar los consultorios por fecha y hora (QUICKSORT)
	void quickSortPorDiaYHora() {
		//Contar el numero de nodos
		int n = 0;
		DisponibilidadConsultorio* temp = cabeza;
		while (temp) {
			n++;
			temp = temp->sig;
		}
		if (n <= 1) return;

		//Crear un arreglo para almacenar los nodos (copia la lista a un arreglo para trabajar con quicksort)
		DisponibilidadConsultorio** arr = new DisponibilidadConsultorio * [n];
		temp = cabeza;
		for (int i = 0; i < n; i++) {
			arr[i] = temp;
			temp = temp->sig;
		}

		//Aplicar el algoritmo quickSort
		quickSort(arr, 0, n - 1);

		//Reconstruir lista ligada a partir del arreglo ordenado
		cabeza = arr[0];
		cabeza->ante = nullptr;
		for (int i = 1; i < n; i++) {
			arr[i - 1]->sig = arr[i];
			arr[i]->ante = arr[i - 1];
		}
		cola = arr[n - 1];
		cola->sig = nullptr;

		delete[] arr;
	}
	//Funcion auxiliar para quicksort
	void quickSort(DisponibilidadConsultorio** arr, int menor, int mayor) { //Recursivo
		if (menor < mayor) {
			int pi = particion(arr, menor, mayor); //Particiona el arreglo
			quickSort(arr, menor, pi - 1); //Recursivamente ordena la parte izquierda
			quickSort(arr, pi + 1, mayor); //Recursivamente ordena la parte derecha
		}
	}

	int particion(DisponibilidadConsultorio** arr, int menor, int mayor) { //Funcion para particionar el arreglo
		DisponibilidadConsultorio* pivot = arr[mayor]; //El pivote es el ultimo elemento
		int i = menor - 1; //Indice del elemento menor

		for (int j = menor; j < mayor; j++) { //Recorre el arreglo
			if (esMenor(arr[j], pivot)) { //Si el elemento actual es menor que el pivote
				i++; //Incrementar el indice del elemento menor
				swap(arr[i], arr[j]); //Intercambiar arr[i] y arr[j]
			}
		}
		swap(arr[i + 1], arr[mayor]); //Intercambiar arr[i + 1] y arr[mayor] (pivote)
		return i + 1; //Retorna el indice del pivote
	}

	//Funcion para guardar archivo de consultorios en consultorios.dat
	void guardarEnArchivo(const char* archivoConsultorios) {
		FILE* archivo;
		if (fopen_s(&archivo,archivoConsultorios, "wb") != 0) {
			cerr << "Error al abrir el archivo para guardar consultorios." << endl;
			return;
		}
		DisponibilidadConsultorio* actual = cabeza;
		while (actual) { //Recorre la lista
			if (fwrite(actual, sizeof(DisponibilidadConsultorio), 1, archivo) != 1) { //Verifica si se escribio correctamente
				cerr << "Error al escribir en el archivo de consultorios." << endl;
				fclose(archivo);
				return;
			}
			actual = actual->sig;
		}
		fclose(archivo);
	}

	//Funcion para cargar archivo de consultorios desde consultorios.dat
	void cargarDesdeArchivo(const char* archivoConsultorios) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoConsultorios, "rb") != 0) {
			cerr << "Error al abrir el archivo para cargar consultorios." << endl;
			return;
		}
		DisponibilidadConsultorio temp;
		while (fread(&temp, sizeof(DisponibilidadConsultorio), 1, archivo)) { //Lee el archivo
			agregarBloque(temp.numConsultorio, temp.dia, temp.horaInicio, temp.horaFin);
		}
		if (ferror(archivo)) //Verifica si hubo un error al leer el archivo
		{
			cerr << "Error al leer el archivo de consultorios." << endl;
		}
		fclose(archivo);
	}
};
ListaConsultorios listaConsultorios;

//Estructura auxiliar para guardar citas como datos planos (se utiliza para los archivos binarios)
struct CitaGuardada
{
	int idPaciente;
	int idMedico;
	char fecha[20];
	char hora[10];
	char especialidad[50];
	char estatus[20];
	char diagnostico[100];
};
//Estructura para almacenar los datos de una cita medica
struct CitaMedica
{
	int idPaciente;
	int idMedico;
	int numConsultorio;
	string fecha;
	string hora;
	string especialidad;
	string estatus;
	string diagnostico;

	CitaMedica* ante;
	CitaMedica* sig;

	//Constructor para inicializar los miembros de la estructura
	CitaMedica(int ip, int im, int nc, string f, string h, string ep, string et, string d)
		:idPaciente(ip), idMedico(im), numConsultorio(nc), fecha(f), hora(h), especialidad(ep), estatus(et), diagnostico(d), ante(nullptr), sig(nullptr) {
	}
};
//Funcion auxiliar para convertir el estatus a string (para ordenamiento)
int prioridadEstatus(const string& estatus) {
	if (estatus == "Cancelada") return 0;
	if (estatus == "Concretada") return 1;
	if (estatus == "Agendada") return 2;
	return 3; // Cualquier otro estatus desconocido va al final
}
//Lista doblemente ligada para citas medicas
class ListaCitas
{
public:
	CitaMedica* cabeza;
	CitaMedica* cola;

	ListaCitas() :cabeza(nullptr), cola(nullptr) {}

	void agregarCita(int idPaciente, int idMedico, int numConsultorio, string fecha, string hora, string especialidad, string estatus, string diagnostico) {
		CitaMedica* nueva = new CitaMedica(idPaciente, idMedico,numConsultorio, fecha, hora, especialidad, estatus, diagnostico);
		if (!cabeza) //Si la lista esta vacia
		{
			cabeza = cola = nueva;
		}
		else
		{
			cola->sig = nueva;
			nueva->ante = cola;
			cola = nueva;
		}
	}

	void mostrarCitas() {
		CitaMedica* temp = cabeza; //Puntero temporal para recorrer la lista
		while (temp != nullptr) {
			cout << "Paciente ID: " << temp->idPaciente << ", Medico ID : " << temp->idMedico << ", Fecha : " << temp->fecha << ", Hora : " << temp->hora << endl;
			temp = temp->sig;
		}
	}

	//Busca si el consultorio esta ocupado en la fecha y hora especificada, en caso de que si, no se puede eliminar el consultorio ocupado
	bool bloqueConsultorioEnUso(int numConsultorio, const char* dia, const char* horaInicio) {
		CitaMedica* temp = cabeza;
		while (temp) {
			Medico* m = listaMedicos.buscarMedico(temp->idMedico);
			if (m) {
				DisponibilidadConsultorio* bloque = listaConsultorios.buscarBloque(m->cedula, dia, horaInicio);
				if (bloque && bloque->numConsultorio == numConsultorio) {
					return true;
				}
			}
			temp = temp->sig;
		}
		return false;
	}

	//Funcion para ordenar la lista de citas medicas por estatus (HEAPSORT)
	void heapSortPorEstatus()
	{
		//Contar el numero de nodos
		int n = 0;
		CitaMedica* temp = cabeza;
		while (temp) {
			n++;
			temp = temp->sig;
		}
		if (n <= 1) //Si ya esta ordenada o vacia
		{
			return;
		}

		//Crear un arreglo para almacenar los nodos (copia la lista a un arreglo para trabajar con heapsort)
		CitaMedica** arr = new CitaMedica * [n];
		temp = cabeza;
		for (int i = 0; i < n; i++) {
			arr[i] = temp;
			temp = temp->sig;
		}

		//Construir el heap maximo
		for (int i = n / 2 - 1; i >= 0; i--) {
			heapify(arr, n, i);
		}

		//Extraer elementos del heap uno por uno
		for (int i = n - 1; i > 0; i--) {
			swap(arr[0], arr[i]); //Intercambiar el primer elemento con el ultimo
			heapify(arr, i, 0); //Llamar a heapify en el heap reducido
		}

		//Reconstruir la lista ligada a partir del arreglo ordenado
		cabeza = arr[0]; //Actualizar la cabeza de la lista
		cabeza->ante = nullptr;
		for (int i = 1; i < n; i++)
		{
			arr[i - 1]->sig = arr[i]; //Actualizar el siguiente del nodo anterior
			arr[i]->ante = arr[i - 1]; //Actualizar el anterior del nodo actual
		}
		cola = arr[n - 1]; //Actualizar la cola de la lista
		cola->sig = nullptr; //Actualizar el siguiente del ultimo nodo a null

		delete[] arr; //Liberar memoria del arreglo
	}
	//Funcion auxiliar para heapify
	void heapify(CitaMedica** arr, int n, int i) { //Recursivo
		int mayor = i; // Inicializar el mayor como raiz
		int izq = 2 * i + 1; // Izquierda = 2*i + 1
		int der = 2 * i + 2; // Derecha = 2*i + 2
		//Comparar el estatus de la raiz con los hijos
		if (izq < n && arr[izq] && arr[mayor] && prioridadEstatus(arr[izq]->estatus) < prioridadEstatus(arr[mayor]->estatus))
			mayor = izq;
		if (der < n && arr[der] && arr[mayor] && prioridadEstatus(arr[der]->estatus) < prioridadEstatus(arr[mayor]->estatus))
			mayor = der;
		if (mayor != i) { //Si el mayor no es la raiz
			swap(arr[i], arr[mayor]); //Intercambiar la raiz con el mayor
			heapify(arr, n, mayor); //Recursivamente heapify
		}
	}

	//Funcion para guardar archivo de citas medicas en citas.dat
	void guardarEnArchivo(const char* archivoCitas) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoCitas, "wb") != 0) { //Verifica si se abrio correctamente
			cerr << "Error al abrir el archivo para guardar citas." << endl;
			return;
		}
		CitaMedica* actual = cabeza;
		while (actual) { //Recorre la lista
			CitaGuardada cg;
			cg.idPaciente = actual->idPaciente;
			cg.idMedico = actual->idMedico;
			strcpy_s(cg.fecha, actual->fecha.c_str());
			strcpy_s(cg.hora, actual->hora.c_str());
			strcpy_s(cg.especialidad, actual->especialidad.c_str());
			strcpy_s(cg.estatus, actual->estatus.c_str());
			strcpy_s(cg.diagnostico, actual->diagnostico.c_str());
			if (fwrite(&cg, sizeof(CitaGuardada), 1, archivo) != 1) { //Verifica si se escribio correctamente
				cerr << "Error al escribir en el archivo de citas." << endl;
				fclose(archivo);
				return;
			}
			actual = actual->sig;
		}
		fclose(archivo);
	}

	//Funcion para cargar archivo de citas medicas desde citas.dat
	void cargarDesdeArchivo(const char* archivoCitas) {
		FILE* archivo;
		if (fopen_s(&archivo, archivoCitas, "rb") != 0) { //Verifica si se abrio correctamente
			cerr << "Error al abrir el archivo para cargar citas." << endl;
			return;
		}
		CitaGuardada cg;
		while (fread(&cg, sizeof(CitaGuardada), 1, archivo)) { //Lee el archivo
			agregarCita(cg.idPaciente, cg.idMedico, 0, cg.fecha, cg.hora, cg.especialidad, cg.estatus, cg.diagnostico);
		}
		if (ferror(archivo)) //Verifica si hubo un error al leer el archivo
		{
			cerr << "Error al leer el archivo de citas." << endl;
		}
		fclose(archivo);
	}
};
ListaCitas listaCitas;

//Funciones de carga y guardado de archivos por usuario
void cargarArchivosUsuario() {
	char archivoPacientes[100];
	char archivoMedicos[100];
	char archivoCitas[100];
	char archivoEspecialidades[100];
	char archivoConsultorios[100];

	sprintf_s(archivoPacientes, "pacientes_%s.dat", usuarioActivo);
	sprintf_s(archivoMedicos, "medicos_%s.dat", usuarioActivo);
	sprintf_s(archivoCitas, "citas_%s.dat", usuarioActivo);
	sprintf_s(archivoEspecialidades, "especialidades_%s.dat", usuarioActivo);
	sprintf_s(archivoConsultorios, "consultorios_%s.dat", usuarioActivo);

	listaPacientes.cargarDesdeArchivo(archivoPacientes);
	listaMedicos.cargarDesdeArchivo(archivoMedicos);
	listaCitas.cargarDesdeArchivo(archivoCitas);
	listaEspecialidades.cargarDesdeArchivo(archivoEspecialidades);
	listaConsultorios.cargarDesdeArchivo(archivoConsultorios);
}
void guardarArchivosUsuario() {
	char archivoPacientes[100];
	char archivoMedicos[100];
	char archivoCitas[100];
	char archivoEspecialidades[100];
	char archivoConsultorios[100];

	sprintf_s(archivoPacientes, "pacientes_%s.dat", usuarioActivo);
	sprintf_s(archivoMedicos, "medicos_%s.dat", usuarioActivo);
	sprintf_s(archivoCitas, "citas_%s.dat", usuarioActivo);
	sprintf_s(archivoEspecialidades, "especialidades_%s.dat", usuarioActivo);
	sprintf_s(archivoConsultorios, "consultorios_%s.dat", usuarioActivo);

	listaPacientes.guardarEnArchivo(archivoPacientes);
	listaMedicos.guardarEnArchivo(archivoMedicos);
	listaCitas.guardarEnArchivo(archivoCitas);
	listaEspecialidades.guardarEnArchivo(archivoEspecialidades);
	listaConsultorios.guardarEnArchivo(archivoConsultorios);
}

//Declaraciones de los procedimientos de dialogo
INT_PTR CALLBACK DialogLoginProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogRegistroProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogMenuProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogMedicosProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogPacientesProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogEspecialidadesProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogConsultoriosProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogCitasMedicasProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogReporteCitasPacienteProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogReporteConsultasMedicoProc(HWND, UINT, WPARAM, LPARAM);



//Funcion main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR LpCmdLine, int nCmdShow)
{
	hInst = hInstance;

	// Mostrar primero la ventana de login
	INT_PTR result = DialogBox(hInstance, MAKEINTRESOURCE(IDD_LOGIN), NULL, DialogLoginProc);

	if (result == IDOK) {
		// Login exitoso ? cargar archivos correspondientes al usuario
		cargarArchivosUsuario();

		// Mostrar men� principal
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_MENU), NULL, DialogMenuProc);
	}

	return (int)result;

	//Inicializar controles comunes (incluye DateTimePicker)
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_DATE_CLASSES;  // Necesario para usar DateTimePicker
	InitCommonControlsEx(&icex);

	//CARGA DE LISTAS EN ARCHIVOS BINARIOS
	cargarArchivosUsuario(); //Cargar los archivos de usuarios, medicos, especialidades, pacientes, consultorios y citas


	//Abrir la ventana de menu
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MENU), NULL, DialogMenuProc);
	return 0;
}

//Funcion auxiliar para limpiar campos
void limpiarCamposLogin(HWND hDlg) {
	SetDlgItemText(hDlg, EC_USUARIO_LOGIN, "");
	SetDlgItemText(hDlg, EC_PASSWORD_LOGIN, "");
}
//Procedimiento para la pantalla de login
INT_PTR CALLBACK DialogLoginProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_INGRESAR_LOGIN:
		{
			char usuario[50], contrasena[50];
			GetDlgItemText(hDlg, EC_USUARIO_LOGIN, usuario, sizeof(usuario));
			GetDlgItemText(hDlg, EC_PASSWORD_LOGIN, contrasena, sizeof(contrasena));

			if (strlen(usuario) == 0 || strlen(contrasena) == 0) {
				MessageBox(hDlg, "Completa todos los campos.", "Faltan datos", MB_OK | MB_ICONWARNING);
				limpiarCamposLogin(hDlg); // Limpiar campos tras intento fallido
				break;
			}

			FILE* file;
			if (fopen_s(&file, "usuarios.txt", "r") != 0) {
				MessageBox(hDlg, "No se pudo abrir el archivo de usuarios.", "Error", MB_OK | MB_ICONERROR);
				limpiarCamposLogin(hDlg); // Limpiar campos tras intento fallido
				break;
			}

			bool autenticado = false;
			char linea[100];
			while (fgets(linea, sizeof(linea), file)) {
				char userFile[50], passFile[50];
				if (sscanf_s(linea, "%[^,],%s", userFile, (unsigned)_countof(userFile), passFile, (unsigned)_countof(passFile)) == 2) {
					if (strcmp(usuario, userFile) == 0 && strcmp(contrasena, passFile) == 0) {
						autenticado = true;
						break;
					}
				}
			}
			fclose(file);

			if (autenticado) {
				strcpy_s(usuarioActivo, usuario); // Guardar el nombre de usuario globalmente
				EndDialog(hDlg, IDOK); // Cierra ventana y contin�a
			}
			else {
				MessageBox(hDlg, "Usuario o contrase�a incorrectos.", "Acceso denegado", MB_OK | MB_ICONERROR);
				limpiarCamposLogin(hDlg); // Limpiar campos tras intento fallido
			}
			break;
		}

		case BT_REGISTRARSE_LOGIN:
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_REGISTRO), hDlg, DialogRegistroProc);
			limpiarCamposLogin(hDlg); // Limpiar campos al volver
			break;
			break;
		}


		case BT_SALIR_LOGIN:
			EndDialog(hDlg, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

//Funcion auxiliar para limpiar campos
void limpiarCamposRegistro(HWND hDlg, bool soloConfirmar = false) {
	if (soloConfirmar) {
		SetDlgItemText(hDlg, EC_CONFIRMAR_PASSWORD_REGISTRO, "");  // Solo limpiar confirmar
	}
	else {
		SetDlgItemText(hDlg, EC_USUARIO_REGISTRO, "");
		SetDlgItemText(hDlg, EC_PASSWORD_REGISTRO, "");
		SetDlgItemText(hDlg, EC_CONFIRMAR_PASSWORD_REGISTRO, "");
	}
}
//Procedimiento para la pantalla de registro
INT_PTR CALLBACK DialogRegistroProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_CREAR_USUARIO:
		{
			char usuario[50], contrasena[50], confirmar[50];
			GetDlgItemText(hDlg, EC_USUARIO_REGISTRO, usuario, sizeof(usuario));
			GetDlgItemText(hDlg, EC_PASSWORD_REGISTRO, contrasena, sizeof(contrasena));
			GetDlgItemText(hDlg, EC_CONFIRMAR_PASSWORD_REGISTRO, confirmar, sizeof(confirmar));

			// Validar campos
			if (strlen(usuario) == 0 || strlen(contrasena) == 0 || strlen(confirmar) == 0)
			{
				MessageBox(hDlg, "Completa todos los campos.", "Faltan datos", MB_OK | MB_ICONWARNING);
				limpiarCamposRegistro(hDlg);  // Limpiar todos los campos
				return TRUE;
			}

			if (strcmp(contrasena, confirmar) != 0)
			{
				MessageBox(hDlg, "Las contrase�as no coinciden.", "Error", MB_OK | MB_ICONERROR);
				SetDlgItemText(hDlg, EC_CONFIRMAR_PASSWORD_REGISTRO, ""); // Solo borra la confirmaci�n
				return TRUE;
			}

			// Verificar si el usuario ya existe
			FILE* file;
			if (fopen_s(&file, "usuarios.txt", "r") == 0)
			{
				char linea[100];
				while (fgets(linea, sizeof(linea), file))
				{
					char userFile[50];
					sscanf_s(linea, "%[^,]", userFile, (unsigned)_countof(userFile));
					if (strcmp(userFile, usuario) == 0)
					{
						fclose(file);
						MessageBox(hDlg, "Ese nombre de usuario ya est� en uso.", "Error", MB_OK | MB_ICONERROR);
						limpiarCamposRegistro(hDlg);  // Limpiar todos los campos
						return TRUE;
					}
				}
				fclose(file);
			}

			// Registrar usuario
			if (fopen_s(&file, "usuarios.txt", "a") != 0)
			{
				MessageBox(hDlg, "No se pudo abrir el archivo de usuarios.", "Error", MB_OK | MB_ICONERROR);
				limpiarCamposRegistro(hDlg);  // Limpiar todos los campos
				return TRUE;
			}

			fprintf(file, "%s,%s\n", usuario, contrasena);
			fclose(file);

			MessageBox(hDlg, "Usuario registrado correctamente.", "Registro exitoso", MB_OK | MB_ICONINFORMATION);
			EndDialog(hDlg, IDOK);
			return TRUE;
		}

		case BT_CANCELAR_REGISTRO:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

//Procedimiento para la pantalla de menu principal
INT_PTR CALLBACK DialogMenuProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_MEDICOS_MENU:
			//Abrir la ventana de medicos
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MEDICOS), hDlg, DialogMedicosProc);
			break;

		case BT_PACIENTES_MENU:
			//Abrir la ventana de pacientes
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PACIENTES), hDlg, DialogPacientesProc);
			break;

		case BT_CONSULTORIOS_MENU:
			//Abrir la ventana de consultorios
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CONSULTORIOS), hDlg, DialogConsultoriosProc);
			break;

		case BT_ESPECIALIDADES_MENU:
			//Abrir la ventana de especialidades
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ESPECIALIDADES), hDlg, DialogEspecialidadesProc);
			break;

		case BT_CITAS_MEDICAS_MENU:
			//Abrir la ventana de citas medicas
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CITAS_MEDICAS), hDlg, DialogCitasMedicasProc);
			break;

		case BT_REPCITASPACIENTE_MENU:
			//Abrir la ventana de reporte de citas por paciente
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPORTE_CITAS_PACIENTE), hDlg, DialogReporteCitasPacienteProc);
			break;

		case BT_REPCONSULTAMEDICO_MENU:
			//Abrir la ventana de reporte de citas por medico
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPORTE_CONSULTA_MEDICO), hDlg, DialogReporteConsultasMedicoProc);
			break;

		case BT_SALIR_MENU:
			guardarArchivosUsuario(); //Guardar los archivos de usuarios, medicos, especialidades, pacientes, consultorios y citas

			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		guardarArchivosUsuario(); //Guardar los archivos de usuarios, medicos, especialidades, pacientes, consultorios y citas

		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

//Funciones para validar los datos de entrada
bool nombreMedicoEsValido(const char* nombre) { //Valida el nombre del medico
	for (int i = 0; nombre[i] != '\0'; i++) {
		if (!isalpha(nombre[i]) && nombre[i] != ' ' && nombre[i] != '�' && nombre[i] != '�') {
			return false;
		}
	}
	return true;
}
bool esNumeroMedicoValido(const char* str) { //Valida que la cedula del medico solo acepte numeros
	int len = (int)strlen(str);
	if (len == 0) return false;
	for (int i = 0; i < len; i++) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}
bool correoMedicoEsValido(const char* correo) {
	// Verificar que no haya espacios
	for (int i = 0; correo[i] != '\0'; ++i) {
		if (isspace(correo[i]))
			return false;
	}

	const char* at = strchr(correo, '@');
	if (!at) return false;

	const char* punto = strchr(at, '.');
	return (punto != nullptr);
}
bool telefonoMedicoEsValido(const char* tel) { //Valida que el telefono del medico tenga 10 digitos y acepta solo numeros
	if (strlen(tel) != 10) return false;
	for (int i = 0; i < 10; ++i) {
		if (!isdigit(tel[i]))
			return false;
	}
	return true;
}
//Procedimiento para la pantalla de medicos
INT_PTR CALLBACK  DialogMedicosProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hListBox;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		//Inicializar el ListBox
		hListBox = GetDlgItem(hDlg, LB_MEDICOS);

		//Llenar el ComboBox con las especialidades
		HWND hComboBox = GetDlgItem(hDlg, CB_ESPECIALIDAD_MEDICO);
		SendMessage(hComboBox, CB_RESETCONTENT, 0, 0); // Limpia el ComboBox antes de llenarlo

		Especialidad* temp = listaEspecialidades.cabeza;
		while (temp != nullptr)
		{
			SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)temp->nombre);
			temp = temp->sig;
		}

		// Llenar el ListBox con los m�dicos existentes
		Medico* m = listaMedicos.cabeza;
		while (m)
		{
			char texto[100];
			sprintf_s(texto, "%s %s %s - %d", m->apellidoPaterno, m->apellidoMaterno, m->nombre, m->cedula);
			int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)m);
			m = m->sig;
		}

		return TRUE;
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_AGREGAR_MEDICO:
		{
			char cedulaStr[10];
			char nombre[50];
			char apellidoP[50];
			char apellidoM[50];
			char correo[50];
			char telefono[20];
			char especialidad[50];

			//Obtener los datos del m�dico desde los edit controls
			GetDlgItemText(hDlg, EC_NUM_CEDULA_MEDICO, cedulaStr, sizeof(cedulaStr));
			GetDlgItemText(hDlg, EC_NOMBRE_MEDICO, nombre, sizeof(nombre));
			GetDlgItemText(hDlg, EC_APELLIDOP_MEDICO, apellidoP, sizeof(apellidoP));
			GetDlgItemText(hDlg, EC_APELLIDOM_MEDICO, apellidoM, sizeof(apellidoM));
			GetDlgItemText(hDlg, EC_CORREO_MEDICO, correo, sizeof(correo));
			GetDlgItemText(hDlg, EC_TELEFONO_MEDICO, telefono, sizeof(telefono));

			HWND hComboEspecialidad = GetDlgItem(hDlg, CB_ESPECIALIDAD_MEDICO);
			int indexEspecialidad = (int)SendMessage(hComboEspecialidad, CB_GETCURSEL, 0, 0);
			if (indexEspecialidad != CB_ERR) {
				SendMessage(hComboEspecialidad, CB_GETLBTEXT, indexEspecialidad, (LPARAM)especialidad);
			}
			else {
				MessageBox(hDlg, "Selecciona una especialidad.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Validaciones de campos vac�os
			if (strlen(cedulaStr) == 0 || strlen(nombre) == 0 || strlen(apellidoP) == 0 || strlen(apellidoM) == 0 ||
				strlen(correo) == 0 || strlen(telefono) == 0 || strlen(especialidad) == 0)
			{
				MessageBox(hDlg, "Por favor llena todos los campos.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Validar campos de nombre y apellidos
			if (!nombreMedicoEsValido(nombre) || !nombreMedicoEsValido(apellidoP) || !nombreMedicoEsValido(apellidoM)) {
				MessageBox(hDlg, "Nombre y apellidos solo deben contener letras y espacios.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Validar que la c�dula no exista
			int cedula = atoi(cedulaStr);
			if (listaMedicos.existeCedula(cedula)) {
				MessageBox(hDlg, "Ya existe un m�dico con esa c�dula.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Validar correo
			if (!correoMedicoEsValido(correo)) {
				MessageBox(hDlg, "Correo inv�lido. Aseg�rate de que tenga un '@' y un '.'", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Validar telefono
			if (!telefonoMedicoEsValido(telefono)) {
				MessageBox(hDlg, "Tel�fono inv�lido. Debe tener 10 d�gitos.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Agregar m�dico a la lista ligada
			listaMedicos.agregarMedico(cedula, nombre, apellidoP, apellidoM, correo, telefono, especialidad);
			Medico* nuevo = listaMedicos.cola;

			//Mostrar en el ListBox
			char textoMostrar[150];
			sprintf_s(textoMostrar, sizeof(textoMostrar), "%s %s %s - %d", apellidoP, apellidoM, nombre, cedula);

			HWND hListBox = GetDlgItem(hDlg, LB_MEDICOS);
			int index = (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)textoMostrar);
			SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)nuevo); //Guardar el puntero

			MessageBox(hDlg, "M�dico agregado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);

			//Limpiar campos
			SetDlgItemText(hDlg, EC_NUM_CEDULA_MEDICO, "");
			SetDlgItemText(hDlg, EC_NOMBRE_MEDICO, "");
			SetDlgItemText(hDlg, EC_APELLIDOP_MEDICO, "");
			SetDlgItemText(hDlg, EC_APELLIDOM_MEDICO, "");
			SetDlgItemText(hDlg, EC_CORREO_MEDICO, "");
			SetDlgItemText(hDlg, EC_TELEFONO_MEDICO, "");
			SendMessage(hComboEspecialidad, CB_SETCURSEL, -1, 0);
			
			break;
		}

		case LB_MEDICOS:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				HWND hListBox = GetDlgItem(hDlg, LB_MEDICOS);
				int index = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
				if (index != LB_ERR)
				{
					Medico* m = (Medico*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); // m es el m�dico seleccionado
					if (m != nullptr)
					{
						char cedulaStr[10];
						sprintf_s(cedulaStr, sizeof(cedulaStr), "%d", m->cedula);

						//Llenar los campos con los datos del m�dico
						SetDlgItemText(hDlg, EC_NUM_CEDULA_MEDICO, cedulaStr);
						SetDlgItemText(hDlg, EC_NOMBRE_MEDICO, m->nombre);
						SetDlgItemText(hDlg, EC_APELLIDOP_MEDICO, m->apellidoPaterno);
						SetDlgItemText(hDlg, EC_APELLIDOM_MEDICO, m->apellidoMaterno);
						SetDlgItemText(hDlg, EC_CORREO_MEDICO, m->correo);
						SetDlgItemText(hDlg, EC_TELEFONO_MEDICO, m->telefono);

						//Seleccionar la especialidad en el ComboBox
						HWND hComboEspecialidad = GetDlgItem(hDlg, CB_ESPECIALIDAD_MEDICO);
						int count = (int)SendMessage(hComboEspecialidad, CB_GETCOUNT, 0, 0);
						for (int i = 0; i < count; i++)
						{
							char esp[50];
							SendMessage(hComboEspecialidad, CB_GETLBTEXT, i, (LPARAM)esp);
							if (strcmp(esp, m->especialidad) == 0)
							{
								SendMessage(hComboEspecialidad, CB_SETCURSEL, i, 0);
								break;
							}
						}
					}
				}
			}
			break;
		}

		case BT_MODIFICAR_MEDICO:
		{
			HWND hListBox = GetDlgItem(hDlg, LB_MEDICOS);
			int index = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
			if (index == LB_ERR) {
				MessageBox(hDlg, "Selecciona un m�dico primero.", "Aviso", MB_OK | MB_ICONWARNING);
				break;
			}

			Medico* m = (Medico*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); // m es el m�dico seleccionado
			if (!m) break;

			char cedulaStr[10], nombre[50], apellidoP[50], apellidoM[50], correo[50], telefono[20], especialidad[50];

			//Obtener datos del formulario
			GetDlgItemText(hDlg, EC_NUM_CEDULA_MEDICO, cedulaStr, sizeof(cedulaStr));
			GetDlgItemText(hDlg, EC_NOMBRE_MEDICO, nombre, sizeof(nombre));
			GetDlgItemText(hDlg, EC_APELLIDOP_MEDICO, apellidoP, sizeof(apellidoP));
			GetDlgItemText(hDlg, EC_APELLIDOM_MEDICO, apellidoM, sizeof(apellidoM));
			GetDlgItemText(hDlg, EC_CORREO_MEDICO, correo, sizeof(correo));
			GetDlgItemText(hDlg, EC_TELEFONO_MEDICO, telefono, sizeof(telefono));

			HWND hComboEspecialidad = GetDlgItem(hDlg, CB_ESPECIALIDAD_MEDICO);
			int indexEspecialidad = (int)SendMessage(hComboEspecialidad, CB_GETCURSEL, 0, 0);
			if (indexEspecialidad != CB_ERR) {
				SendMessage(hComboEspecialidad, CB_GETLBTEXT, indexEspecialidad, (LPARAM)especialidad);
			}
			else {
				MessageBox(hDlg, "Selecciona una especialidad.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Validaciones de campos
			if (strlen(cedulaStr) == 0 || strlen(nombre) == 0 || strlen(apellidoP) == 0 || strlen(apellidoM) == 0 ||
				strlen(correo) == 0 || strlen(telefono) == 0) {
				MessageBox(hDlg, "Por favor completa todos los campos.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			if (!nombreMedicoEsValido(nombre) || !nombreMedicoEsValido(apellidoP) || !nombreMedicoEsValido(apellidoM)) {
				MessageBox(hDlg, "Nombre y apellidos solo deben contener letras y espacios.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//validar que la cedula sea un numero
			char bufferCedula[50];
			if (!esNumero(bufferCedula)) {
				MessageBox(hDlg, "La c�dula profesional debe contener solo n�meros.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Actualizar datos del nodo
			m->cedula = atoi(cedulaStr);
			strcpy_s(m->nombre, nombre);
			strcpy_s(m->apellidoPaterno, apellidoP);
			strcpy_s(m->apellidoMaterno, apellidoM);
			strcpy_s(m->correo, correo);
			strcpy_s(m->telefono, telefono);
			strcpy_s(m->especialidad, especialidad);

			//Actualizar en el ListBox
			char nuevoTexto[150];
			sprintf_s(nuevoTexto, sizeof(nuevoTexto), "%s %s %s - %d", apellidoP, apellidoM, nombre, m->cedula);

			SendMessage(hListBox, LB_DELETESTRING, index, 0);
			int nuevoIndex = (int)SendMessage(hListBox, LB_INSERTSTRING, index, (LPARAM)nuevoTexto);
			SendMessage(hListBox, LB_SETITEMDATA, nuevoIndex, (LPARAM)m);
			SendMessage(hListBox, LB_SETCURSEL, nuevoIndex, 0);

			MessageBox(hDlg, "Datos del m�dico modificados correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);
			break;
		}

		case BT_ELIMINAR_MEDICO:
		{
			HWND hListBox = GetDlgItem(hDlg, LB_MEDICOS);
			int index = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
			if (index == LB_ERR) {
				MessageBox(hDlg, "Selecciona un m�dico primero.", "Aviso", MB_OK | MB_ICONWARNING);
				break;
			}

			Medico* m = (Medico*)SendMessage(hListBox, LB_GETITEMDATA, index, 0);
			if (!m) break;

			//Eliminar de la lista doblemente ligada
			if (m->ante) m->ante->sig = m->sig;
			else listaMedicos.cabeza = m->sig;

			if (m->sig) m->sig->ante = m->ante;
			else listaMedicos.cola = m->ante;

			delete m;

			//Actualizar el ListBox
			SendMessage(hListBox, LB_DELETESTRING, index, 0);

			//Limpiar los campos
			SetDlgItemText(hDlg, EC_NUM_CEDULA_MEDICO, "");
			SetDlgItemText(hDlg, EC_NOMBRE_MEDICO, "");
			SetDlgItemText(hDlg, EC_APELLIDOP_MEDICO, "");
			SetDlgItemText(hDlg, EC_APELLIDOM_MEDICO, "");
			SetDlgItemText(hDlg, EC_CORREO_MEDICO, "");
			SetDlgItemText(hDlg, EC_TELEFONO_MEDICO, "");
			SendDlgItemMessage(hDlg, CB_ESPECIALIDAD_MEDICO, CB_SETCURSEL, -1, 0);

			MessageBox(hDlg, "M�dico eliminado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);
			break;
		}

		case BT_ORDENAR_MEDICOS:
		{
			HWND hListBox = GetDlgItem(hDlg, LB_MEDICOS);

			//Ordenar la lista de m�dicos por apellido paterno usando HeapSort
			listaMedicos.heapSortPorApellido();

			//Limpiar el ListBox
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

			//Volver a llenar el ListBox con los datos ordenados
			Medico* m = listaMedicos.cabeza;
			while (m != nullptr)
			{
				char texto[150];
				sprintf_s(texto, sizeof(texto), "%s %s %s - %d", m->apellidoPaterno, m->apellidoMaterno, m->nombre, m->cedula);
				int index = (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)m);
				m = m->sig;
			}

			MessageBox(hDlg, "Lista de m�dicos ordenada por apellido paterno.", "Ordenado", MB_OK | MB_ICONINFORMATION);
			break;
		}

		case BT_SALIR_MEDICOS:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

//Funciones para validar los datos de entrada
bool nombrePacienteEsValido(const char* nombre) { //Valida el nombre del paciente que solo acepte letras y espacios
	for (int i = 0; nombre[i] != '\0'; i++) {
		if (!isalpha(nombre[i]) && nombre[i] != ' ' && nombre[i] != '�' && nombre[i] != '�') {
			return false;
		}
	}
	return true;
}
bool edadPacienteEsValida(const char* edadStr) { //Valida que la edad del paciente sea un numero entre 0 y 130
	if (strlen(edadStr) == 0) return false;
	for (int i = 0; edadStr[i] != '\0'; i++) {
		if (!isdigit(edadStr[i]))
			return false;
	}
	int edad = atoi(edadStr);
	return edad >= 0 && edad <= 130;
}
bool telefonoPacienteEsValido(const char* tel) { //Valida que el telefono del paciente tenga 10 digitos y acepta solo numeros
	if (strlen(tel) != 10) return false;
	for (int i = 0; i < 10; ++i) {
		if (!isdigit(tel[i]))
			return false;
	}
	return true;
}
bool correoPacienteEsValido(const char* correo) { //Valida que el correo del paciente tenga el formato correcto
	// Verificar que no haya espacios
	for (int i = 0; correo[i] != '\0'; ++i) {
		if (isspace(correo[i]))
			return false;
	}
	const char* at = strchr(correo, '@');
	if (!at) return false;
	const char* punto = strchr(at, '.');
	return (punto != nullptr);
}
//Procedimiento para la pantalla de pacientes
INT_PTR CALLBACK DialogPacientesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hListBox;

	switch (message)
	{
	case WM_INITDIALOG: {
		hListBox = GetDlgItem(hDlg, LB_PACIENTES);
		HWND hComboBox = GetDlgItem(hDlg, CB_GENERO_PACIENTE);
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)"Hombre");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)"Mujer");

		// Llenar el ListBox con los pacientes existentes
		Paciente* p = listaPacientes.cabeza;
		while (p != nullptr)
		{
			char texto[200];
			sprintf_s(texto, "%s %s %s - %d", p->apellidoPaterno, p->apellidoMaterno, p->nombre, p->id);
			int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)p);
			p = p->sig;
		}

		return TRUE;
	}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_AGREGAR_PACIENTES: {
			char idStr[10];
			char nombre[50];
			char apellidoP[50];
			char apellidoM[50];
			char correo[50];
			char telefono[20];
			char edad[10];
			char genero[10];

			//Obtener los datos del paciente en los edit controls
			GetDlgItemText(hDlg, EC_ID_PACIENTE, idStr, sizeof(idStr));
			GetDlgItemText(hDlg, EC_NOMBRE_PACIENTE, nombre, sizeof(nombre));
			GetDlgItemText(hDlg, EC_APELLIDOP_PACIENTE, apellidoP, sizeof(apellidoP));
			GetDlgItemText(hDlg, EC_APELLIDOM_PACIENTE, apellidoM, sizeof(apellidoM));
			GetDlgItemText(hDlg, EC_CORREO_PACIENTE, correo, sizeof(correo));
			GetDlgItemText(hDlg, EC_TELEFONO_PACIENTE, telefono, sizeof(telefono));
			GetDlgItemText(hDlg, EC_EDAD_PACIENTE, edad, sizeof(edad));

			//Convertir el ID a int
			int id = atoi(idStr);
			if (id == 0 || strlen(nombre) == 0 || strlen(apellidoP) == 0 || strlen(apellidoM) == 0)
			{
				MessageBox(hDlg, "Por favor llena todos los campos correctamente.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Validar que el ID no exista
			if (listaPacientes.existeID(id)) {
				MessageBox(hDlg, "Ya existe un paciente con ese ID.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Validaciones individuales
			if (!nombrePacienteEsValido(nombre) || !nombrePacienteEsValido(apellidoP) || !nombrePacienteEsValido(apellidoM)) {
				MessageBox(hDlg, "Nombre y apellidos solo deben contener letras y espacios.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			if (!edadPacienteEsValida(edad)) {
				MessageBox(hDlg, "Edad inv�lida. Ingresa un n�mero entre 0 y 130.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			if (!telefonoPacienteEsValido(telefono)) {
				MessageBox(hDlg, "El tel�fono debe contener exactamente 10 d�gitos num�ricos.", "Aviso", MB_OK | MB_ICONWARNING);
				break;
			}

			if (!correoPacienteEsValido(correo)) {
				MessageBox(hDlg, "Correo inv�lido. Aseg�rate de incluir '@' y '.' despu�s, sin espacios.", "Error", MB_OK | MB_ICONERROR);
				break;

				char bufferID[50];
			if (!esNumero(bufferID)) {
					MessageBox(hDlg, "El ID debe contener solo n�meros.", "Error", MB_OK | MB_ICONERROR);
					break;
				}
			}

			// Obtener g�nero del ComboBox
			HWND hComboBox = GetDlgItem(hDlg, CB_GENERO_PACIENTE);
			SendMessage(hComboBox, CB_GETLBTEXT, SendMessage(hComboBox, CB_GETCURSEL, 0, 0), (LPARAM)genero);

			//Agregar a la lista ligada
			int edadInt = atoi(edad);
			listaPacientes.agregarPaciente(id, nombre, apellidoP, apellidoM, correo, telefono, genero, edadInt);
			Paciente* nuevo = listaPacientes.cola;

			//Mostrar en el ListBox
			char textoMostrar[200];
			sprintf_s(textoMostrar, sizeof(textoMostrar), "%s %s %s - %d", apellidoP, apellidoM, nombre, id);

			int index = static_cast<int>(SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)textoMostrar));
			SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)nuevo); // Guardar el puntero al paciente en el item de la lista

			MessageBox(hDlg, "Paciente agregado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);

			//Limpiar los campos
			SetDlgItemText(hDlg, EC_ID_PACIENTE, "");
			SetDlgItemText(hDlg, EC_NOMBRE_PACIENTE, "");
			SetDlgItemText(hDlg, EC_APELLIDOP_PACIENTE, "");
			SetDlgItemText(hDlg, EC_APELLIDOM_PACIENTE, "");
			SetDlgItemText(hDlg, EC_CORREO_PACIENTE, "");
			SetDlgItemText(hDlg, EC_TELEFONO_PACIENTE, "");
			SetDlgItemText(hDlg, EC_EDAD_PACIENTE, "");
			SendMessage(hComboBox, CB_SETCURSEL, -1, 0);
		}
			break;

		case LB_PACIENTES:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int index = static_cast<int>(SendMessage(hListBox, LB_GETCURSEL, 0, 0));
				if (index != LB_ERR)
				{
					Paciente* p = (Paciente*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); // p es el paciente seleccionado
					if (p != nullptr)
					{
						char idStr[10], edadStr[10];
						sprintf_s(idStr, sizeof(idStr), "%d", p->id);
						sprintf_s(edadStr, sizeof(edadStr), "%s", p->edad);

						SetDlgItemText(hDlg, EC_ID_PACIENTE, idStr);
						SetDlgItemText(hDlg, EC_NOMBRE_PACIENTE, p->nombre);
						SetDlgItemText(hDlg, EC_APELLIDOP_PACIENTE, p->apellidoPaterno);
						SetDlgItemText(hDlg, EC_APELLIDOM_PACIENTE, p->apellidoMaterno);
						SetDlgItemText(hDlg, EC_CORREO_PACIENTE, p->correo);
						SetDlgItemText(hDlg, EC_TELEFONO_PACIENTE, p->telefono);
						SetDlgItemText(hDlg, EC_EDAD_PACIENTE, edadStr);

						HWND hCombo = GetDlgItem(hDlg, CB_GENERO_PACIENTE);
						SendMessage(hCombo, CB_SELECTSTRING, -1, (LPARAM)p->genero);
					}
				}
			}
		}
			break;

		case BT_MODIFICAR_PACIENTES: {
			HWND hListBox = GetDlgItem(hDlg, LB_PACIENTES);
			int index = static_cast<int>(SendMessage(hListBox, LB_GETCURSEL, 0, 0));

			if (index == LB_ERR) {
				MessageBox(hDlg, "Selecciona un paciente primero.", "Aviso", MB_OK | MB_ICONWARNING);
			}
			else {
				Paciente* p = (Paciente*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); // p es el paciente seleccionado

				if (p != nullptr) {
					char idStr[10], nombre[50], apellidoP[50], apellidoM[50], correo[50], telefono[20], edad[10], genero[10];

					// Obtener datos del formulario
					GetDlgItemText(hDlg, EC_ID_PACIENTE, idStr, sizeof(idStr));
					GetDlgItemText(hDlg, EC_NOMBRE_PACIENTE, nombre, sizeof(nombre));
					GetDlgItemText(hDlg, EC_APELLIDOP_PACIENTE, apellidoP, sizeof(apellidoP));
					GetDlgItemText(hDlg, EC_APELLIDOM_PACIENTE, apellidoM, sizeof(apellidoM));
					GetDlgItemText(hDlg, EC_CORREO_PACIENTE, correo, sizeof(correo));
					GetDlgItemText(hDlg, EC_TELEFONO_PACIENTE, telefono, sizeof(telefono));
					GetDlgItemText(hDlg, EC_EDAD_PACIENTE, edad, sizeof(edad));
					HWND hComboBox = GetDlgItem(hDlg, CB_GENERO_PACIENTE);
					SendMessage(hComboBox, CB_GETLBTEXT, SendMessage(hComboBox, CB_GETCURSEL, 0, 0), (LPARAM)genero);

					// Validar datos
					int idNuevo = atoi(idStr);
					if (idNuevo == 0 || strlen(nombre) == 0 || strlen(apellidoP) == 0 || strlen(apellidoM) == 0) {
						MessageBox(hDlg, "Por favor llena todos los campos correctamente.", "Error", MB_OK | MB_ICONERROR);
						break;
					}

					if (strlen(apellidoP) == 0 || strlen(apellidoM) == 0) {
						MessageBox(hDlg, "Debes ingresar ambos apellidos.", "Error", MB_OK | MB_ICONERROR);
						break;
					}

					if (!nombrePacienteEsValido(nombre) || !nombrePacienteEsValido(apellidoP) || !nombrePacienteEsValido(apellidoM)) {
						MessageBox(hDlg, "Nombre y apellidos solo deben contener letras y espacios.", "Error", MB_OK | MB_ICONERROR);
						break;
					}

					if (!edadPacienteEsValida(edad)) {
						MessageBox(hDlg, "Edad inv�lida. Ingresa un n�mero entre 0 y 130.", "Error", MB_OK | MB_ICONERROR);
						break;
					}

					if (!telefonoPacienteEsValido(telefono)) {
						MessageBox(hDlg, "El tel�fono debe contener exactamente 10 d�gitos num�ricos.", "Aviso", MB_OK | MB_ICONWARNING);
						break;
					}

					if (!correoPacienteEsValido(correo)) {
						MessageBox(hDlg, "Correo inv�lido. Aseg�rate de incluir '@' y '.' despu�s, sin espacios.", "Error", MB_OK | MB_ICONERROR);
						break;
					}

					// Modificar nodo
					p->id = idNuevo;
					strcpy_s(p->nombre, nombre);
					strcpy_s(p->apellidoPaterno, apellidoP);
					strcpy_s(p->apellidoMaterno, apellidoM);
					strcpy_s(p->correo, correo);
					strcpy_s(p->telefono, telefono);
					strcpy_s(p->edad, edad);
					strcpy_s(p->genero, genero);

					// Actualizar texto en ListBox
					char nuevoTexto[200];
					sprintf_s(nuevoTexto, sizeof(nuevoTexto), "%s %s %s - %d", apellidoP, apellidoM, nombre, idNuevo);
					SendMessage(hListBox, LB_DELETESTRING, index, 0);
					int nuevoIndex = static_cast<int>(SendMessage(hListBox, LB_INSERTSTRING, index, (LPARAM)nuevoTexto));
					SendMessage(hListBox, LB_SETITEMDATA, nuevoIndex, (LPARAM)p);
					SendMessage(hListBox, LB_SETCURSEL, nuevoIndex, 0);

					MessageBox(hDlg, "Paciente modificado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);
				}
			}
		}
			break;

		case BT_ELIMINAR_PACIENTES: {
			HWND hListBox = GetDlgItem(hDlg, LB_PACIENTES);
			int index = static_cast<int>(SendMessage(hListBox, LB_GETCURSEL, 0, 0));
			if (index == LB_ERR) {
				MessageBox(hDlg, "Selecciona un paciente primero.", "Aviso", MB_OK | MB_ICONWARNING);
			}
			else {
				Paciente* p = (Paciente*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); // p es el paciente seleccionado

				//Eliminar de la lista
				if (p != nullptr) //Si la lista no esta vacia
				{
					if (p->ante != nullptr) {
						p->ante->sig = p->sig; //Actualizar el puntero del anterior
					}
					else {
						listaPacientes.cabeza = p->sig; //Actualizar la cabeza de la lista
					}
					if (p->sig != nullptr) {
						p->sig->ante = p->ante; //Actualizar el puntero del siguiente
					}
					else {
						listaPacientes.cola = p->ante; //Actualizar la cola de la lista
					}
					delete p; //Liberar memoria

					//Actualizar el ListBox
					SendMessage(hListBox, LB_DELETESTRING, index, 0); //Eliminar el item del ListBox

					//Limpiar los edit controls
					SetDlgItemText(hDlg, EC_ID_PACIENTE, "");
					SetDlgItemText(hDlg, EC_NOMBRE_PACIENTE, "");
					SetDlgItemText(hDlg, EC_APELLIDOP_PACIENTE, "");
					SetDlgItemText(hDlg, EC_APELLIDOM_PACIENTE, "");
					SetDlgItemText(hDlg, EC_CORREO_PACIENTE, "");
					SetDlgItemText(hDlg, EC_TELEFONO_PACIENTE, "");
					SetDlgItemText(hDlg, EC_EDAD_PACIENTE, "");
					SendDlgItemMessage(hDlg, CB_GENERO_PACIENTE, CB_SETCURSEL, -1, 0); // Deselecciona el g�nero

					//Confirmar la eliminacion
					MessageBox(hDlg, "Paciente eliminado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);
				}
				else {
					MessageBox(hDlg, "Error al eliminar el paciente.", "Error", MB_OK | MB_ICONERROR);
				}
			}
		}
			 break;

		case BT_ORDENAR_PACIENTES: {
			HWND hListBox = GetDlgItem(hDlg, LB_PACIENTES);

			// Ordenar la lista por apellido paterno
			listaPacientes.heapSortPorApellido();

			// Limpiar y volver a llenar el ListBox
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

			Paciente* actual = listaPacientes.cabeza;
			while (actual != nullptr) {
				char texto[200];
				sprintf_s(texto, sizeof(texto), "%s %s %s - %d", actual->apellidoPaterno, actual->apellidoMaterno, actual->nombre, actual->id);
				int index = static_cast<int>(SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto));
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)actual);
				actual = actual->sig;
			}
		}
			break;

		case BT_BUSCAR_APELLIDO_PACIENTE:
		{
			char apellido[50];
			GetDlgItemText(hDlg, EC_BUSCAR_APELLIDO_PACIENTE, apellido, sizeof(apellido));

			if (strlen(apellido) == 0) {
				MessageBox(hDlg, "Ingresa un apellido paterno para buscar.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			vector<Paciente*> encontrados = buscarPacientesPorApellido(apellido);

			HWND hListBox = GetDlgItem(hDlg, LB_PACIENTES);
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

			if (encontrados.empty()) { //Si no se encontraron pacientes
				MessageBox(hDlg, "No se encontraron pacientes con ese apellido.", "Sin resultados", MB_OK | MB_ICONINFORMATION);
				break;
			}

			for (Paciente* p : encontrados) { //Recorrer la lista de pacientes encontrados
				char texto[150];
				sprintf_s(texto, "%s %s %s - %d", p->apellidoPaterno, p->apellidoMaterno, p->nombre, p->id); //Mostrar en el ListBox
				int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto); 
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)p);
			}

			break;
		}

		//Caso para salir de la ventana de pacientes
		case BT_SALIR_PACIENTE:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

//Funciones para validar los datos de entrada
bool nombreEspecialidadEsValido(const char* nombre) { //Valida el nombre de la especialidad que solo acepte letras y espacios
	for (int i = 0; nombre[i] != '\0'; i++) {
		if (!isalpha(nombre[i]) && nombre[i] != ' ' && nombre[i] != '�' && nombre[i] != '�') {
			return false;
		}
	}
	return true;
}
//Procedimiento para la pantalla de especialidades
INT_PTR CALLBACK DialogEspecialidadesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hListBox;
	switch (message)
	{
	case WM_INITDIALOG: {
		hListBox = GetDlgItem(hDlg, LB_ESPECIALIDADES);

		//Llenar el ListBox con las especialidades
		SendMessage(hListBox, LB_RESETCONTENT, 0, 0); // Limpia el ListBox antes de llenarlo
		Especialidad* temp = listaEspecialidades.cabeza;
		while (temp != nullptr)
		{
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)temp->nombre);
			temp = temp->sig;
		}
		return TRUE;
	}
					  break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BT_AGREGAR_ESPECIALIDAD:
		{
			char nombre[50];
			GetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, nombre, sizeof(nombre));

			// Quitar espacios iniciales y finales
			char limpio[50];
			strncpy_s(limpio, nombre, sizeof(limpio) - 1);
			_strlwr_s(limpio); // Convertir a min�sculas para evitar duplicados por may�sculas/min�sculas

			// Eliminar espacios al inicio y final
			int len = (int)strlen(limpio);
			while (len > 0 && isspace(limpio[len - 1])) limpio[--len] = '\0';
			int start = 0;
			while (isspace(limpio[start])) start++;
			memmove(limpio, limpio + start, strlen(limpio + start) + 1);

			if (strlen(limpio) == 0)
			{
				MessageBox(hDlg, "El nombre no puede estar vac�o o ser solo espacios.", "Error", MB_OK | MB_ICONERROR);
				SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, ""); // Limpiar el edit control
				break;
			}
			if (!nombreEspecialidadEsValido(limpio)) {
				MessageBox(hDlg, "El nombre solo debe contener letras y espacios.", "Error", MB_OK | MB_ICONERROR);
				SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, "");
				break;
			}

			if (listaEspecialidades.existeEspecialidad(limpio))
			{
				MessageBox(hDlg, "Esa especialidad ya est� registrada.", "Error", MB_OK | MB_ICONERROR);
				SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, ""); // Limpiar el edit control
				break;
			}
			else
			{
				//Agregar a la lista ligada
				listaEspecialidades.agregarEspecialidad(limpio);
				Especialidad* nueva = listaEspecialidades.cola;

				int index = static_cast<int>(SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)nueva->nombre));
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)nueva); // Guardar el puntero a la especialidad en el item de la lista
				//Confirmar la adici�n
				MessageBox(hDlg, "Especialidad agregada correctamente.", "Exito", MB_OK | MB_ICONINFORMATION);
				//Limpiar el edit control
				SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, "");
			}
		}
		break;

		case LB_ESPECIALIDADES:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int index = static_cast<int>(SendMessage(hListBox, LB_GETCURSEL, 0, 0));
				if (index != LB_ERR)
				{
					Especialidad* e = (Especialidad*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); //e es la especialidad seleccionada
					if (e != nullptr)
					{
						SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, e->nombre);
					}
				}
			}
		}
		break;

		case BT_ELIMINAR_ESPECIALIDAD:
		{
			HWND hListBox = GetDlgItem(hDlg, LB_ESPECIALIDADES);
			int index = static_cast<int>(SendMessage(hListBox, LB_GETCURSEL, 0, 0));
			if (index == LB_ERR)
			{
				MessageBox(hDlg, "Selecciona una especialidad primero.", "Aviso", MB_OK | MB_ICONWARNING);
				break;
			}
			else
			{
				Especialidad* e = (Especialidad*)SendMessage(hListBox, LB_GETITEMDATA, index, 0); //e es la especialidad seleccionada
				//Eliminar de la lista
				if (e != nullptr) //Si la lista no esta vacia
				{
					if (listaMedicos.especialidadEstaEnUso(e->nombre)) {
						MessageBox(hDlg, "No se puede eliminar esta especialidad porque est� asignada a uno o m�s m�dicos.", "Error", MB_OK | MB_ICONERROR);
						break;
					}
					listaEspecialidades.eliminarEspecialidad(e); //Eliminar de la lista ligada
					//Actualizar el ListBox
					SendMessage(hListBox, LB_DELETESTRING, index, 0); //Eliminar el item del ListBox
					//Limpiar el edit control
					SetDlgItemText(hDlg, EC_AGREGAR_ESPECIALIDAD, "");
					//Confirmar la eliminacion
					MessageBox(hDlg, "Especialidad eliminada correctamente.", "Exito", MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					MessageBox(hDlg, "Error al eliminar la especialidad.", "Error", MB_OK | MB_ICONERROR);
				}
			}
		}
		break;

		case BT_ORDENAR_ESPECIALIDAD:
		{
			//Ordenar la lista de especialidades por nombre
			listaEspecialidades.quickSortPorNombre();
			//Limpiar y volver a llenar el ListBox
			HWND hListBox = GetDlgItem(hDlg, LB_ESPECIALIDADES);
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
			
			//Llenar el ListBox con las especialidades ordenadas
			Especialidad* temp = listaEspecialidades.cabeza;
			while (temp) {
				int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)temp->nombre);
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)temp);
				temp = temp->sig;
			}
			MessageBox(hDlg, "Lista de especialidades ordenada por nombre.", "Ordenado", MB_OK | MB_ICONINFORMATION);
		}
		break;

		case BT_SALIR_ESPECIALIDAD:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

//procedimiento para la pantalla de consultorios
INT_PTR CALLBACK DialogConsultoriosProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hListBox;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		hListBox = GetDlgItem(hDlg, LB_CONSULTORIOS);

		// ComboBox: Cantidad de consultorios (1�10)
		HWND hCantidad = GetDlgItem(hDlg, CB_CANTIDAD_CONSULTORIOS);
		for (int i = 1; i <= 10; ++i)
		{
			char texto[3];
			sprintf_s(texto, "%d", i);
			SendMessage(hCantidad, CB_ADDSTRING, 0, (LPARAM)texto);
		}

		// ComboBox: D�as de la semana
		HWND hDia = GetDlgItem(hDlg, CB_DIAS_CONSULTORIOS);
		const char* dias[] = { "Domingo", "Lunes", "Martes", "Mi�rcoles", "Jueves", "Viernes", "S�bado" };
		for (int i = 0; i < 7; ++i)
			SendMessage(hDia, CB_ADDSTRING, 0, (LPARAM)dias[i]);

		// ComboBox: Horas (07:00 a 23:00)
		HWND hInicio = GetDlgItem(hDlg, CB_HORA_INICIO);
		HWND hFin = GetDlgItem(hDlg, CB_HORA_FIN);
		for (int h = 7; h < 24; ++h)
		{
			char hora[6];
			sprintf_s(hora, "%02d:00", h);
			SendMessage(hInicio, CB_ADDSTRING, 0, (LPARAM)hora);
			SendMessage(hFin, CB_ADDSTRING, 0, (LPARAM)hora);
		}

		// Llenar el ListBox con los bloques de disponibilidad ya registrados
		DisponibilidadConsultorio* c = listaConsultorios.cabeza;
		while (c)
		{
			char texto[100];
			sprintf_s(texto, "Consultorio %d - %s: %s a %s", c->numConsultorio, c->dia, c->horaInicio, c->horaFin);
			int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)c);
			c = c->sig;
		}

		return TRUE;
	}
	break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case BT_AGREGAR_CONSULTORIO:
		{
			char consultorioStr[3], diaStr[15], horaInicio[6], horaFin[6];

			HWND hCantidad = GetDlgItem(hDlg, CB_CANTIDAD_CONSULTORIOS);
			HWND hDia = GetDlgItem(hDlg, CB_DIAS_CONSULTORIOS);
			HWND hHoraInicio = GetDlgItem(hDlg, CB_HORA_INICIO);
			HWND hHoraFin = GetDlgItem(hDlg, CB_HORA_FIN);

			int idxC = SendMessage(hCantidad, CB_GETCURSEL, 0, 0);
			int idxD = SendMessage(hDia, CB_GETCURSEL, 0, 0);
			int idxHI = SendMessage(hHoraInicio, CB_GETCURSEL, 0, 0);
			int idxHF = SendMessage(hHoraFin, CB_GETCURSEL, 0, 0);

			if (idxC == CB_ERR || idxD == CB_ERR || idxHI == CB_ERR || idxHF == CB_ERR)
			{
				MessageBox(hDlg, "Selecciona todos los campos correctamente.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			SendMessage(hCantidad, CB_GETLBTEXT, idxC, (LPARAM)consultorioStr);
			SendMessage(hDia, CB_GETLBTEXT, idxD, (LPARAM)diaStr);
			SendMessage(hHoraInicio, CB_GETLBTEXT, idxHI, (LPARAM)horaInicio);
			SendMessage(hHoraFin, CB_GETLBTEXT, idxHF, (LPARAM)horaFin);

			int consultorio = atoi(consultorioStr);
			int hInicio = atoi(horaInicio);
			int hFin = atoi(horaFin);

			if (hInicio >= hFin)
			{
				MessageBox(hDlg, "La hora final debe ser mayor que la inicial.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			for (int h = hInicio; h < hFin; ++h)
			{
				char hI[6], hF[6];
				sprintf_s(hI, "%02d:00", h);
				sprintf_s(hF, "%02d:00", h + 1);

				//Validar si el consultorio ya est� ocupado en ese horario
				if (listaConsultorios.existeBloque(consultorio, diaStr, hI))
				{
					char msg[100];
					sprintf_s(msg, "Ya est� registrado el consultorio %d en %s de %s a %s", consultorio, diaStr, hI, hF);
					MessageBox(hDlg, msg, "Bloque duplicado", MB_OK | MB_ICONERROR);
					continue;
				}

				//Agregar si no existe
				listaConsultorios.agregarBloque(consultorio, diaStr, hI, hF);

				// Mostrar en el ListBox
				char texto[100];
				sprintf_s(texto, "Consultorio %d - %s: %s a %s", consultorio, diaStr, hI, hF);
				int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)listaConsultorios.cola);
			}

			// Limpiar campos
			SendMessage(hCantidad, CB_SETCURSEL, -1, 0);
			SendMessage(hDia, CB_SETCURSEL, -1, 0);
			SendMessage(hHoraInicio, CB_SETCURSEL, -1, 0);
			SendMessage(hHoraFin, CB_SETCURSEL, -1, 0);
		}
		break;

		case BT_ELIMINAR_CONSULTORIO:
		{
			int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
			if (index == LB_ERR)
			{
				MessageBox(hDlg, "Selecciona un registro para eliminar.", "Aviso", MB_OK | MB_ICONWARNING);
				break;
			}

			DisponibilidadConsultorio* d = (DisponibilidadConsultorio*)SendMessage(hListBox, LB_GETITEMDATA, index, 0);
			if (d) {
				if (listaCitas.bloqueConsultorioEnUso(d->numConsultorio, d->dia, d->horaInicio)) {
					MessageBox(hDlg, "No se puede eliminar este bloque de consultorio porque est� asignado a una cita m�dica.", "Error", MB_OK | MB_ICONERROR);
					break;
				}

				listaConsultorios.eliminarBloque(d);
				SendMessage(hListBox, LB_DELETESTRING, index, 0);
			}
		}
		break;

		case BT_ORDENAR_CONSULTORIOS:
		{
			listaConsultorios.quickSortPorDiaYHora(); // Ordena la lista
			// Limpiar y volver a llenar el ListBox
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
			DisponibilidadConsultorio* c = listaConsultorios.cabeza;
			while (c)
			{
				char texto[100];
				sprintf_s(texto, "Consultorio %d - %s: %s a %s", c->numConsultorio, c->dia, c->horaInicio, c->horaFin);
				int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)c);
				c = c->sig;
			}
			MessageBox(hDlg, "Lista de consultorios ordenada por d�a y hora.", "Ordenado", MB_OK | MB_ICONINFORMATION);
		}
		break;

		case BT_SALIR_CONSULTORIOS:
			EndDialog(hDlg, 0);
			break;
		}
	}
	break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}

	return FALSE;
}

//Funcion auxiliar para validar la fecha
int CompareSystemTime(const SYSTEMTIME& a, const SYSTEMTIME& b) {
	if (a.wYear != b.wYear) return a.wYear - b.wYear;
	if (a.wMonth != b.wMonth) return a.wMonth - b.wMonth;
	return a.wDay - b.wDay;
}
//Procedimiento para la pantalla de citas
INT_PTR CALLBACK DialogCitasMedicasProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hListBoxCitas; //ListBox que mostrar� las citas registradas

	switch (message)
	{
	case WM_INITDIALOG:
	{
		//Inicializar el ListBox
		hListBoxCitas = GetDlgItem(hDlg, LB_CITAS);

		//Llenar el ListBox con las citas existentes
		CitaMedica* cita = listaCitas.cabeza;
		while (cita) {
			Paciente* paciente = listaPacientes.buscarPacientePorID(cita->idPaciente);
			Medico* medico = listaMedicos.buscarMedico(cita->idMedico);

			char texto[256];
			sprintf_s(texto, sizeof(texto),
				"Paciente: %d - %s | M�dico: %d - %s | Fecha: %s | Hora: %s | Estatus: %s",
				cita->idPaciente,
				listaPacientes.buscarNombrePorId(cita->idPaciente).c_str(),
				cita->idMedico,
				medico ? listaMedicos.buscarNombrePorCedula(cita->idMedico).c_str() : "Desconocido",
				cita->fecha.c_str(),
				cita->hora.c_str(),
				cita->estatus.c_str()
			);

			int index = SendMessage(hListBoxCitas, LB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hListBoxCitas, LB_SETITEMDATA, index, (LPARAM)cita);

			cita = cita->sig;
		}


		//Llenar el ComboBox de pacientes
		HWND hComboPacientes = GetDlgItem(hDlg, CB_PACIENTE_CITA);
		SendMessage(hComboPacientes, CB_RESETCONTENT, 0, 0);
		Paciente* p = listaPacientes.cabeza;
		while (p)
		{
			char texto[200];
			sprintf_s(texto, sizeof(texto), "%d - %s %s %s", p->id, p->apellidoPaterno, p->apellidoMaterno, p->nombre);
			int index = SendMessage(hComboPacientes, CB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hComboPacientes, CB_SETITEMDATA, index, (LPARAM)p);
			p = p->sig;
		}

		//Llenar el ComboBox de especialidades
		HWND hComboEspecialidades = GetDlgItem(hDlg, CB_ESPECIALIDAD_CITA);
		SendMessage(hComboEspecialidades, CB_RESETCONTENT, 0, 0);
		Especialidad* e = listaEspecialidades.cabeza;
		while (e)
		{
			int index = SendMessage(hComboEspecialidades, CB_ADDSTRING, 0, (LPARAM)e->nombre);
			SendMessage(hComboEspecialidades, CB_SETITEMDATA, index, (LPARAM)e);
			e = e->sig;
		}

		//Inicializar el ComboBox de m�dicos
		HWND hComboMedicos = GetDlgItem(hDlg, CB_MEDICO_CITA);
		SendMessage(hComboMedicos, CB_RESETCONTENT, 0, 0);

		//Configurar el DateTimePicker para impedir fechas pasadas
		SYSTEMTIME st;
		GetLocalTime(&st);
		HWND hFecha = GetDlgItem(hDlg, DT_FECHA_CITA);
		DateTime_SetSystemtime(hFecha, GDT_VALID, &st);
		DateTime_SetRange(hFecha, GDTR_MIN, &st);

		//Limpiar el ComboBox de horas
		HWND hCBHora = GetDlgItem(hDlg, CB_HORA_CITA);
		SendMessage(hCBHora, CB_RESETCONTENT, 0, 0);

		return TRUE;
	}

	//Manejo de comandos de la ventana
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		//Cuando cambia la especialidad, cargar los m�dicos correspondientes
		case CB_ESPECIALIDAD_CITA:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				HWND hComboEspecialidades = GetDlgItem(hDlg, CB_ESPECIALIDAD_CITA);
				HWND hComboMedicos = GetDlgItem(hDlg, CB_MEDICO_CITA);
				SendMessage(hComboMedicos, CB_RESETCONTENT, 0, 0);

				int selIndex = (int)SendMessage(hComboEspecialidades, CB_GETCURSEL, 0, 0);
				if (selIndex != CB_ERR)
				{
					char especialidadSeleccionada[50];
					SendMessage(hComboEspecialidades, CB_GETLBTEXT, selIndex, (LPARAM)especialidadSeleccionada);
					Medico* m = listaMedicos.cabeza;
					while (m)
					{
						if (strcmp(m->especialidad, especialidadSeleccionada) == 0)
						{
							char texto[200];
							sprintf_s(texto, "%d - %s %s %s", m->cedula, m->apellidoPaterno, m->apellidoMaterno, m->nombre);
							int index = SendMessage(hComboMedicos, CB_ADDSTRING, 0, (LPARAM)texto);
							SendMessage(hComboMedicos, CB_SETITEMDATA, index, (LPARAM)m);
						}
						m = m->sig;
					}
				}
			}
			return TRUE;
		}
		break;

		//Cuando cambia el m�dico, calcular las horas disponibles para ese d�a y paciente
		case CB_MEDICO_CITA:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				HWND hComboMedicos = GetDlgItem(hDlg, CB_MEDICO_CITA);
				HWND hComboHoras = GetDlgItem(hDlg, CB_HORA_CITA);
				SendMessage(hComboHoras, CB_RESETCONTENT, 0, 0);

				int indexMedico = (int)SendMessage(hComboMedicos, CB_GETCURSEL, 0, 0);
				if (indexMedico == CB_ERR) break;

				Medico* medico = (Medico*)SendMessage(hComboMedicos, CB_GETITEMDATA, indexMedico, 0);
				if (!medico) break;

				//Obtener el paciente seleccionado
				HWND hComboPacientes = GetDlgItem(hDlg, CB_PACIENTE_CITA);
				int indexPaciente = (int)SendMessage(hComboPacientes, CB_GETCURSEL, 0, 0);
				Paciente* paciente = nullptr;
				if (indexPaciente != CB_ERR) {
					paciente = (Paciente*)SendMessage(hComboPacientes, CB_GETITEMDATA, indexPaciente, 0);
				}

				//Obtener la fecha seleccionada
				SYSTEMTIME st;
				HWND hFecha = GetDlgItem(hDlg, DT_FECHA_CITA);
				DateTime_GetSystemtime(hFecha, &st);

				//Calcular el d�a de la semana
				tm timeStruct = {};
				timeStruct.tm_year = st.wYear - 1900;
				timeStruct.tm_mon = st.wMonth - 1;
				timeStruct.tm_mday = st.wDay;
				mktime(&timeStruct);

				const char* diaSemana[] = { "Domingo", "Lunes", "Martes", "Mi�rcoles", "Jueves", "Viernes", "S�bado" };
				const char* diaSeleccionado = diaSemana[timeStruct.tm_wday];

				char debugDia[200];
				sprintf_s(debugDia, "Fecha seleccionada: %04d-%02d-%02d\nDia calculado: %s",
					st.wYear, st.wMonth, st.wDay, diaSeleccionado);
				MessageBox(hDlg, debugDia, "Verificaci�n d�a actual", MB_OK);

				//Recorrer bloques disponibles ese d�a
				DisponibilidadConsultorio* disp = listaConsultorios.cabeza;
				while (disp)
				{
					if (strcmp(disp->dia, diaSeleccionado) == 0)
					{
						static bool debugShown = false;
						if (!debugShown) {
							char debug[300];
							sprintf_s(debug, "DISP:\nConsultorio %d\nD�a: %s\nHoraInicio: %s\n\nD�aActual: %s",
								disp->numConsultorio, disp->dia, disp->horaInicio, diaSeleccionado);
							MessageBox(hDlg, debug, "Revisando disponibilidad", MB_OK);
							debugShown = true;
						}
						
						bool ocupado = false;
						CitaMedica* cita = listaCitas.cabeza;
						char fechaStr[11];
						sprintf_s(fechaStr, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);

						while (cita)
						{
							if (cita->fecha == fechaStr &&
								cita->hora == disp->horaInicio &&
								(cita->idMedico == medico->cedula || (paciente && cita->idPaciente == paciente->id)))
							{
								char info[200];
								sprintf_s(info, "Cita encontrada\nFecha: %s\nHora: %s\nM�dico: %d\nPaciente: %d",
									cita->fecha.c_str(), cita->hora.c_str(), cita->idMedico, cita->idPaciente);
								MessageBox(hDlg, info, "Empalme detectado", MB_OK);

								ocupado = true;
								break;
							}

							cita = cita->sig;
						}

						if (!ocupado)
						{
							char textoHora[50];
							sprintf_s(textoHora, "%s - %s (Consultorio %d)", disp->horaInicio, disp->horaFin, disp->numConsultorio);

							if (SendMessage(hComboHoras, CB_FINDSTRINGEXACT, -1, (LPARAM)textoHora) == CB_ERR)
							{
								int index = SendMessage(hComboHoras, CB_ADDSTRING, 0, (LPARAM)textoHora);
								SendMessage(hComboHoras, CB_SETITEMDATA, index, (LPARAM)disp); // Guarda puntero si deseas usarlo despu�s
							}
						}
					}
					disp = disp->sig;
				}
			}
			return TRUE;
		}
		break;

		case BT_AGREGAR_CITA:
		{
			//Obtener controles
			HWND hComboPacientes = GetDlgItem(hDlg, CB_PACIENTE_CITA);
			HWND hComboEspecialidades = GetDlgItem(hDlg, CB_ESPECIALIDAD_CITA);
			HWND hComboMedicos = GetDlgItem(hDlg, CB_MEDICO_CITA);
			HWND hHora = GetDlgItem(hDlg, CB_HORA_CITA);
			HWND hFecha = GetDlgItem(hDlg, DT_FECHA_CITA);

			//Validar selecci�n
			int indexPaciente = (int)SendMessage(hComboPacientes, CB_GETCURSEL, 0, 0);
			int indexMedico = (int)SendMessage(hComboMedicos, CB_GETCURSEL, 0, 0);
			int indexHora = (int)SendMessage(hHora, CB_GETCURSEL, 0, 0);

			if (indexPaciente == CB_ERR || indexMedico == CB_ERR || indexHora == CB_ERR)
			{
				MessageBox(hDlg, "Selecciona paciente, m�dico y hora.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Obtener punteros a objetos
			Paciente* paciente = (Paciente*)SendMessage(hComboPacientes, CB_GETITEMDATA, indexPaciente, 0);
			Medico* medico = (Medico*)SendMessage(hComboMedicos, CB_GETITEMDATA, indexMedico, 0);
			DisponibilidadConsultorio* disponibilidad = (DisponibilidadConsultorio*)SendMessage(hHora, CB_GETITEMDATA, indexHora, 0);

			//Obtener especialidad seleccionada
			char especialidad[50];
			int indexEspecialidad = (int)SendMessage(hComboEspecialidades, CB_GETCURSEL, 0, 0);
			if (indexEspecialidad != CB_ERR)
			{
				SendMessage(hComboEspecialidades, CB_GETLBTEXT, indexEspecialidad, (LPARAM)especialidad);
			}
			else
			{
				MessageBox(hDlg, "Selecciona una especialidad.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Obtener hora de inicio desde el texto del ComboBox
			char horaCompleta[50];
			SendMessage(hHora, CB_GETLBTEXT, indexHora, (LPARAM)horaCompleta);
			char horaInicio[6];
			strncpy_s(horaInicio, horaCompleta, 5);

			//Obtener fecha en formato YYYY-MM-DD
			SYSTEMTIME st;
			DateTime_GetSystemtime(hFecha, &st);
			char fecha[11];
			sprintf_s(fecha, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);

			//Validar que no sea una fecha pasada
			SYSTEMTIME hoy;
			GetLocalTime(&hoy);
			if (CompareSystemTime(st, hoy) < 0)
			{
				MessageBox(hDlg, "No puedes registrar citas en fechas pasadas.", "Fecha inv�lida", MB_OK | MB_ICONERROR);
				break;
			}

			//Verificar si ya existe una cita en ese horario con ese m�dico o paciente
			CitaMedica* actual = listaCitas.cabeza;
			bool existe = false;
			while (actual)
			{
				// Validar que los campos clave no est�n vac�os ni corruptos
				if (!actual->estatus.empty() && !actual->fecha.empty() && !actual->hora.empty())
				{
					if (actual->estatus != "Cancelada" &&
						actual->fecha == fecha && actual->hora == horaInicio &&
						(actual->idMedico == medico->cedula || actual->idPaciente == paciente->id))
					{
						existe = true;
						break;
					}
				}

				actual = actual->sig;
			}

			if (existe)
			{
				MessageBox(hDlg, "Ya existe una cita registrada en ese horario para ese paciente o m�dico.", "Cita duplicada", MB_OK | MB_ICONERROR);
				break;
			}

			//Guardar cita
			listaCitas.agregarCita(
				paciente->id,
				medico->cedula,
				disponibilidad->numConsultorio,
				fecha,
				horaInicio,
				especialidad,
				"Agendada",
				"" //Diagn�stico vac�o
			);

			//Mostrar en ListBox
			char resumen[250];
			sprintf_s(resumen, "Paciente: %d - %s | M�dico: %d - %s | Fecha: %s | Hora: %s | Consultorio: %d",
				paciente->id, listaPacientes.buscarNombrePorId(paciente->id).c_str(),
				medico->cedula, listaMedicos.buscarNombrePorCedula(medico->cedula).c_str(),
				fecha, horaInicio, disponibilidad->numConsultorio);

			int index = SendMessage(hListBoxCitas, LB_ADDSTRING, 0, (LPARAM)resumen);
			SendMessage(hListBoxCitas, LB_SETITEMDATA, index, (LPARAM)listaCitas.cola); //Apunta al nodo reci�n agregado

			MessageBox(hDlg, "Cita m�dica registrada correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);

			//Limpiar selecci�n
			SendMessage(hComboPacientes, CB_SETCURSEL, -1, 0);
			SendMessage(hComboEspecialidades, CB_SETCURSEL, -1, 0);
			SendMessage(hComboMedicos, CB_SETCURSEL, -1, 0);
			SendMessage(hHora, CB_SETCURSEL, -1, 0);
			DateTime_SetSystemtime(hFecha, GDT_VALID, &st);
			DateTime_SetRange(hFecha, GDTR_MIN, &st);
			SendMessage(hHora, CB_RESETCONTENT, 0, 0);

			break;
		}

		case BT_CANCELAR_CITA:
		{
			int indexSel = (int)SendMessage(hListBoxCitas, LB_GETCURSEL, 0, 0);
			if (indexSel == LB_ERR)
			{
				MessageBox(hDlg, "Selecciona una cita de la lista para cancelarla.", "Ninguna seleccionada", MB_OK | MB_ICONWARNING);
				break;
			}

			CitaMedica* cita = (CitaMedica*)SendMessage(hListBoxCitas, LB_GETITEMDATA, indexSel, 0);
			if (cita->estatus == "Concretada") //Si la cita ya fue Concretada no se puede cancelar
			{
				MessageBox(hDlg, "No se puede cancelar una cita que ya fue concretada.", "Acci�n no permitida", MB_OK | MB_ICONERROR);
				break;
			}
			if (!cita)
			{
				MessageBox(hDlg, "Error al recuperar los datos de la cita.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			//Cambiar estatus
			cita->estatus = "Cancelada";

			//Crear nuevo texto para el ListBox
			char texto[300];
			sprintf_s(texto, "Paciente: %d - %s | M�dico: %d - %s | Fecha: %s | Hora: %s | Consultorio: %d (Cancelada)",
			cita->idPaciente, listaPacientes.buscarNombrePorId(cita->idPaciente).c_str(),
			cita->idMedico, listaMedicos.buscarNombrePorCedula(cita->idMedico).c_str(),
			cita->fecha.c_str(), cita->hora.c_str(), cita->numConsultorio);

			//Reemplazar en el ListBox
			SendMessage(hListBoxCitas, LB_DELETESTRING, indexSel, 0);
			int nuevoIndex = (int)SendMessage(hListBoxCitas, LB_INSERTSTRING, indexSel, (LPARAM)texto);
			//Actualizar el puntero al nodo
			SendMessage(hListBoxCitas, LB_SETITEMDATA, nuevoIndex, (LPARAM)cita);

			SendMessage(hListBoxCitas, LB_SETCURSEL, nuevoIndex, 0); //Seleccionar el nuevo item

			MessageBox(hDlg, "Cita cancelada correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);
			break;
		}

		case BT_CONFIRMAR_CITA:
		{
			int indexSel = (int)SendMessage(hListBoxCitas, LB_GETCURSEL, 0, 0);
			if (indexSel == LB_ERR)
			{
				MessageBox(hDlg, "Selecciona una cita para confirmar llegada.", "Ninguna seleccionada", MB_OK | MB_ICONWARNING);
				break;
			}

			CitaMedica* cita = (CitaMedica*)SendMessage(hListBoxCitas, LB_GETITEMDATA, indexSel, 0);
			if (!cita) //Si la lista esta vacia
			{
				MessageBox(hDlg, "Error al recuperar los datos de la cita.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			if (cita->estatus == "Concretada") //Se verifica si la cita ya fue confirmada
			{
				MessageBox(hDlg, "Esta cita ya ha sido confirmada previamente.", "Informaci�n", MB_OK | MB_ICONINFORMATION);
				break;
			}

			if (cita->estatus == "Cancelada") //Se verifica si la cita fue cancelada
			{
				MessageBox(hDlg, "No puedes confirmar una cita que ha sido cancelada.", "Acci�n inv�lida", MB_OK | MB_ICONERROR);
				break;
			}

			//Confirmar llegada
			cita->estatus = "Concretada";

			//Reconstruir texto actualizado
			char texto[300];
			sprintf_s(texto, "Paciente: %d - %s | M�dico: %d - %s | Fecha: %s | Hora: %s | Consultorio: %d (Concretada)",
				cita->idPaciente, listaPacientes.buscarNombrePorId(cita->idPaciente).c_str(),
				cita->idMedico, listaMedicos.buscarNombrePorCedula(cita->idMedico).c_str(),
				cita->fecha.c_str(), cita->hora.c_str(), cita->numConsultorio);

			//Actualizar en el ListBox
			SendMessage(hListBoxCitas, LB_DELETESTRING, indexSel, 0);
			int nuevoIndex = (int)SendMessage(hListBoxCitas, LB_INSERTSTRING, indexSel, (LPARAM)texto);
			SendMessage(hListBoxCitas, LB_SETITEMDATA, nuevoIndex, (LPARAM)cita);
			SendMessage(hListBoxCitas, LB_SETCURSEL, nuevoIndex, 0);

			//Habilitar campo de diagn�stico
			HWND hDiagnostico = GetDlgItem(hDlg, EC_DIAGNOSTICO_CITA);
			EnableWindow(hDiagnostico, TRUE);
			SetWindowText(hDiagnostico, ""); // Limpia el campo

			MessageBox(hDlg, "Llegada confirmada. Puedes capturar el diagn�stico ahora.", "�xito", MB_OK | MB_ICONINFORMATION);
			break;
		}

		case BT_GUARDAR_DIAGNOSTICO:
		{
			int indexSel = (int)SendMessage(hListBoxCitas, LB_GETCURSEL, 0, 0);
			if (indexSel == LB_ERR) //Si la lista esta vacia
			{
				MessageBox(hDlg, "Selecciona una cita de la lista para guardar diagn�stico.", "Ninguna seleccionada", MB_OK | MB_ICONWARNING);
				break;
			}

			CitaMedica* cita = (CitaMedica*)SendMessage(hListBoxCitas, LB_GETITEMDATA, indexSel, 0);
			if (!cita) //Si la lista esta vacia
			{
				MessageBox(hDlg, "Error al recuperar los datos de la cita.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			char buffere[256] = ""; //Buffer para el diagn�stico
			strcpy_s(buffere, cita->diagnostico.c_str());
			if (strlen(buffere) > 200) //Si el diagn�stico excede el l�mite
			{
				MessageBox(hDlg, "El diagn�stico es demasiado largo. M�ximo 200 caracteres.", "L�mite excedido", MB_OK | MB_ICONWARNING);
				break;
			}

			if (cita->estatus != "Concretada") //Si la cita no fue confirmada
			{
				MessageBox(hDlg, "Solo puedes guardar diagn�stico para citas concretadas.", "Acci�n no permitida", MB_OK | MB_ICONERROR);
				break;
			}

			//Obtener el contenido del EditControl
			char buffer[300];
			HWND hDiagnostico = GetDlgItem(hDlg, EC_DIAGNOSTICO_CITA);
			GetWindowText(hDiagnostico, buffer, sizeof(buffer));

			if (strlen(buffer) == 0) //Si el campo esta vacio
			{
				MessageBox(hDlg, "El campo de diagn�stico est� vac�o.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}

			//Guardar en la cita
			cita->diagnostico = buffer;

			MessageBox(hDlg, "Diagn�stico guardado correctamente.", "�xito", MB_OK | MB_ICONINFORMATION);

			//Deshabilitar campo despu�s de guardar
			EnableWindow(hDiagnostico, FALSE);
			//Limpiar el campo
			SetWindowText(hDiagnostico, "");
			break;
		}

		case BT_ORDENAR_CITAS_ESTADO:
		{
			listaCitas.heapSortPorEstatus();

			HWND hListBox = GetDlgItem(hDlg, LB_CITAS);
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

			CitaMedica* cita = listaCitas.cabeza;
			while (cita) {
				Paciente* paciente = listaPacientes.buscarPacientePorID(cita->idPaciente);
				Medico* medico = listaMedicos.buscarMedico(cita->idMedico);

				char texto[256];
				sprintf_s(texto, sizeof(texto),
					"Paciente: %d - %s | M�dico: %d - %s | Fecha: %s | Hora: %s | Estatus: %s",
					cita->idPaciente,
					listaPacientes.buscarNombrePorId(cita->idPaciente).c_str(),
					cita->idMedico,
					listaMedicos.buscarNombrePorCedula(cita->idMedico).c_str(),
					cita->fecha.c_str(),
					cita->hora.c_str(),
					cita->estatus.c_str()
				);

				int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)texto);
				SendMessage(hListBox, LB_SETITEMDATA, index, (LPARAM)cita);
				cita = cita->sig;
			}

			MessageBox(hDlg, "Lista de citas ordenada por estatus.", "Ordenado", MB_OK | MB_ICONINFORMATION);
		}
		break;

		case BT_SALIR_CITA:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}

	return FALSE;
}

//Procedimiento para la pantalla de reportes de citas por medico
INT_PTR CALLBACK DialogReporteConsultasMedicoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hCBEspecialidad, hDTFechaInicial, hDTFechaFinal, hLBMedicos;
	static HWND hLBCitas;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Obtener handles de controles
		hCBEspecialidad = GetDlgItem(hDlg, CB_ESPECIALIDAD_REPCONSMEDICO);
		hDTFechaInicial = GetDlgItem(hDlg, DT_FECHAINICIAL_REPCONSMEDICO);
		hDTFechaFinal = GetDlgItem(hDlg, DT_FECHAFINAL_REPCONSMEDICO);
		hLBMedicos = GetDlgItem(hDlg, LB_REPCONSMEDICO);
		hLBCitas = GetDlgItem(hDlg, LB_CITAS_REPCONSMEDICO);

		// Llenar ComboBox con especialidades
		SendMessage(hCBEspecialidad, CB_RESETCONTENT, 0, 0);
		Especialidad* esp = listaEspecialidades.cabeza;
		while (esp)
		{
			SendMessage(hCBEspecialidad, CB_ADDSTRING, 0, (LPARAM)esp->nombre);
			esp = esp->sig;
		}

		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CB_ESPECIALIDAD_REPCONSMEDICO:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				//Limpiar ListBox de m�dicos
				SendMessage(hLBMedicos, LB_RESETCONTENT, 0, 0);

				//Obtener especialidad seleccionada
				char especialidad[50];
				int index = SendMessage(hCBEspecialidad, CB_GETCURSEL, 0, 0);
				SendMessage(hCBEspecialidad, CB_GETLBTEXT, index, (LPARAM)especialidad);

				//Crear lista temporal de m�dicos filtrados por especialidad
				ListaMedicos listaFiltrada;
				Medico* m = listaMedicos.cabeza;
				while (m)
				{
					if (strcmp(m->especialidad, especialidad) == 0)
					{
						listaFiltrada.agregarMedico(m->cedula, m->nombre, m->apellidoPaterno, m->apellidoMaterno, m->correo, m->telefono, m->especialidad);
					}
					m = m->sig;
				}

				//Ordenar la lista filtrada por apellido paterno
				listaFiltrada.heapSortPorApellido();

				//Llenar el ListBox con los m�dicos ordenados
				m = listaFiltrada.cabeza;
				while (m)
				{
					char texto[150];
					sprintf_s(texto, "%s %s %s - %d", m->apellidoPaterno, m->apellidoMaterno, m->nombre, m->cedula);
					int idx = SendMessage(hLBMedicos, LB_ADDSTRING, 0, (LPARAM)texto);

					//Muy importante: asignar el puntero al m�dico real (no de la lista temporal)
					Medico* original = listaMedicos.buscarMedico(m->cedula);
					SendMessage(hLBMedicos, LB_SETITEMDATA, idx, (LPARAM)original);

					m = m->sig;
				}
			}
			break;
		}

		case LB_REPCONSMEDICO:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				//Si el usuario selecciono otro m�dico limpiamos los campos
				SetDlgItemText(hDlg, EC_CEDULA_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_NOMBRE_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_FECHA_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_DIA_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_HORA_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_PACIENTE_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_ESTATUS_REPCONSMEDICO, "");
				SetDlgItemText(hDlg, EC_DIAGNOSTICO_REPCONSMEDICO, "");
			}
			break;

		case LB_CITAS_REPCONSMEDICO:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int idx = SendMessage(hLBCitas, LB_GETCURSEL, 0, 0);
				if (idx != LB_ERR)
				{
					CitaMedica* cita = (CitaMedica*)SendMessage(hLBCitas, LB_GETITEMDATA, idx, 0);
					Paciente* paciente = listaPacientes.buscarPacientePorID(cita->idPaciente);

					//Mostrar datos en EditControls
					SetDlgItemText(hDlg, EC_PACIENTE_REPCONSMEDICO, paciente ? listaPacientes.buscarNombrePorId(paciente->id).c_str() : "Desconocido");
					SetDlgItemText(hDlg, EC_FECHA_REPCONSMEDICO, cita->fecha.c_str());
					SetDlgItemText(hDlg, EC_HORA_REPCONSMEDICO, cita->hora.c_str());
					SetDlgItemText(hDlg, EC_ESTATUS_REPCONSMEDICO, cita->estatus.c_str());
					SetDlgItemText(hDlg, EC_DIAGNOSTICO_REPCONSMEDICO, cita->diagnostico.c_str());

					//Calcular d�a de la semana
					SYSTEMTIME st = {};
					sscanf_s(cita->fecha.c_str(), "%4d-%2d-%2d", &st.wYear, &st.wMonth, &st.wDay);
					tm tm_fecha = {};
					tm_fecha.tm_year = st.wYear - 1900;
					tm_fecha.tm_mon = st.wMonth - 1;
					tm_fecha.tm_mday = st.wDay;
					mktime(&tm_fecha);

					const char* dias[] = { "Domingo", "Lunes", "Martes", "Mi�rcoles", "Jueves", "Viernes", "S�bado" };
					SetDlgItemText(hDlg, EC_DIA_REPCONSMEDICO, dias[tm_fecha.tm_wday]);
				}
			}
			break;

		case BT_CONFIRMAR_REPCONSMEDICO:
		{
			//Obtener fechas
			SYSTEMTIME stInicio, stFin;
			DateTime_GetSystemtime(hDTFechaInicial, &stInicio);
			DateTime_GetSystemtime(hDTFechaFinal, &stFin);

			//Validar que la fecha final no sea menor que la fecha inicial
			FILETIME ftInicio, ftFin;
			SystemTimeToFileTime(&stInicio, &ftInicio);
			SystemTimeToFileTime(&stFin, &ftFin);

			if (CompareFileTime(&ftFin, &ftInicio) < 0) {
				MessageBox(hDlg, "La Fecha Final no puede ser anterior a la Fecha Inicial.", "Error de Fecha", MB_OK | MB_ICONERROR);
				return TRUE; // Detiene el resto del manejo del bot�n Confirmar
			}

			//Obtener m�dico seleccionado
			int idxMedico = SendMessage(hLBMedicos, LB_GETCURSEL, 0, 0);
			if (idxMedico == LB_ERR) {
				MessageBox(hDlg, "Selecciona un m�dico primero.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}
			Medico* medico = (Medico*)SendMessage(hLBMedicos, LB_GETITEMDATA, idxMedico, 0);

			//Convertir fechas a string "YYYY-MM-DD"
			char fechaInicio[11], fechaFin[11];
			sprintf_s(fechaInicio, "%04d-%02d-%02d", stInicio.wYear, stInicio.wMonth, stInicio.wDay);
			sprintf_s(fechaFin, "%04d-%02d-%02d", stFin.wYear, stFin.wMonth, stFin.wDay);


			// Limpiar ListBox de citas anteriores
			SendMessage(hLBCitas, LB_RESETCONTENT, 0, 0);

			// Recorrer todas las citas y filtrar por m�dico y rango de fechas
			CitaMedica* c = listaCitas.cabeza;
			bool encontrado = false;

			while (c)
			{
				if (c->idMedico == medico->cedula &&
					c->fecha >= fechaInicio && c->fecha <= fechaFin)
				{
					encontrado = true;

					// Mostrar resumen de la cita en el nuevo ListBox
					Paciente* p = listaPacientes.buscarPacientePorID(c->idPaciente);
					char textoCita[200];
					sprintf_s(textoCita, sizeof(textoCita), "Paciente: %s | Fecha: %s | Hora: %s | Estatus: %s",
						p ? listaPacientes.buscarNombrePorId(p->id).c_str() : "Desconocido",
						c->fecha.c_str(), c->hora.c_str(), c->estatus.c_str());

					int idx = SendMessage(hLBCitas, LB_ADDSTRING, 0, (LPARAM)textoCita);
					SendMessage(hLBCitas, LB_SETITEMDATA, idx, (LPARAM)c);
				}
				c = c->sig;
			}

			// Mostrar n�mero de c�dula y nombre completo del m�dico seleccionado
			char nombreCompleto[150];
			sprintf_s(nombreCompleto, sizeof(nombreCompleto), "%s %s %s", medico->apellidoPaterno, medico->apellidoMaterno, medico->nombre);
			SetDlgItemInt(hDlg, EC_CEDULA_REPCONSMEDICO, medico->cedula, FALSE);
			SetDlgItemText(hDlg, EC_NOMBRE_REPCONSMEDICO, nombreCompleto);

			if (!encontrado) {
				MessageBox(hDlg, "No se encontraron citas para ese m�dico en ese rango de fechas.", "Sin resultados", MB_OK | MB_ICONINFORMATION);
				return TRUE; // Detiene el resto del manejo del bot�n Confirmar
			}
			break;
		}

		case BT_SALIR_REPCONSMEDICO:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}

	return FALSE;
}

//Procedimiento para la pantalla de reportes de citas por paciente
INT_PTR CALLBACK DialogReporteCitasPacienteProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hDTPFechaInicial, hDTPFechaFinal, hLBPacientes;
	static HWND hLBCitas;


	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Obtener handles de controles
		hDTPFechaInicial = GetDlgItem(hDlg, DT_FECHAINICIAL_REPCITASPACIENTE);
		hDTPFechaFinal = GetDlgItem(hDlg, DT_FECHAFINAL_REPCITASPACIENTE);
		hLBPacientes = GetDlgItem(hDlg, LB_REPCITASPACIENTE);
		hLBCitas = GetDlgItem(hDlg, LB_CITAS_REPPACIENTE);

		// Llenar ListBox con pacientes
		SendMessage(hLBPacientes, LB_RESETCONTENT, 0, 0);
		Paciente* p = listaPacientes.cabeza;
		while (p)
		{
			char texto[200];
			sprintf_s(texto, "%s %s %s - %d", p->apellidoPaterno, p->apellidoMaterno, p->nombre, p->id);
			int idx = SendMessage(hLBPacientes, LB_ADDSTRING, 0, (LPARAM)texto);
			SendMessage(hLBPacientes, LB_SETITEMDATA, idx, (LPARAM)p);
			p = p->sig;
		}

		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case LB_REPCITASPACIENTE:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				// Limpiar los campos de visualizaci�n
				SetDlgItemText(hDlg, EC_ID_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_NOMBREPACIENTE_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_FECHA_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_DIA_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_HORA_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_NOMBREMEDICO_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_ESPECIALIDAD_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_ESTATUS_REPCITASPACIENTE, "");
				SetDlgItemText(hDlg, EC_DIAGNOSTICO_REPCITASPACIENTE, "");
			}
			break;

		case LB_CITAS_REPPACIENTE:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int idx = SendMessage(hLBCitas, LB_GETCURSEL, 0, 0);
				if (idx != LB_ERR)
				{
					CitaMedica* cita = (CitaMedica*)SendMessage(hLBCitas, LB_GETITEMDATA, idx, 0);
					Medico* medico = listaMedicos.buscarMedico(cita->idMedico);

					// Mostrar en EditControls
					SetDlgItemText(hDlg, EC_FECHA_REPCITASPACIENTE, cita->fecha.c_str());
					SetDlgItemText(hDlg, EC_HORA_REPCITASPACIENTE, cita->hora.c_str());
					SetDlgItemText(hDlg, EC_NOMBREMEDICO_REPCITASPACIENTE, medico ? listaMedicos.buscarNombrePorCedula(medico->cedula).c_str() : "Desconocido");
					SetDlgItemText(hDlg, EC_ESTATUS_REPCITASPACIENTE, medico ? medico->especialidad : "");
					SetDlgItemText(hDlg, EC_ESTATUS_REPCITASPACIENTE, cita->estatus.c_str());
					SetDlgItemText(hDlg, EC_DIAGNOSTICO_REPCITASPACIENTE, cita->diagnostico.c_str());

					// D�a de la semana
					SYSTEMTIME st = {};
					sscanf_s(cita->fecha.c_str(), "%4d-%2d-%2d", &st.wYear, &st.wMonth, &st.wDay);
					tm tm_fecha = {};
					tm_fecha.tm_year = st.wYear - 1900;
					tm_fecha.tm_mon = st.wMonth - 1;
					tm_fecha.tm_mday = st.wDay;
					mktime(&tm_fecha);
					const char* dias[] = { "Domingo", "Lunes", "Martes", "Mi�rcoles", "Jueves", "Viernes", "S�bado" };
					SetDlgItemText(hDlg, EC_DIA_REPCITASPACIENTE, dias[tm_fecha.tm_wday]);
				}
			}
			break;

		case BT_CONFIRMAR_REPCITASPACIENTE:
		{
			// Validar selecci�n de paciente
			int idxPaciente = SendMessage(hLBPacientes, LB_GETCURSEL, 0, 0);
			if (idxPaciente == LB_ERR) {
				MessageBox(hDlg, "Selecciona un paciente primero.", "Faltan datos", MB_OK | MB_ICONWARNING);
				break;
			}
			Paciente* paciente = (Paciente*)SendMessage(hLBPacientes, LB_GETITEMDATA, idxPaciente, 0);

			// Obtener fechas
			SYSTEMTIME stInicio, stFin;
			DateTime_GetSystemtime(hDTPFechaInicial, &stInicio);
			DateTime_GetSystemtime(hDTPFechaFinal, &stFin);

			FILETIME ftInicio, ftFin;
			SystemTimeToFileTime(&stInicio, &ftInicio);
			SystemTimeToFileTime(&stFin, &ftFin);

			if (CompareFileTime(&ftFin, &ftInicio) < 0) {
				MessageBox(hDlg, "La Fecha Final no puede ser anterior a la Fecha Inicial.", "Error de Fecha", MB_OK | MB_ICONERROR);
				return TRUE;
			}

			// Convertir fechas a string
			char fechaInicio[11], fechaFin[11];
			sprintf_s(fechaInicio, "%04d-%02d-%02d", stInicio.wYear, stInicio.wMonth, stInicio.wDay);
			sprintf_s(fechaFin, "%04d-%02d-%02d", stFin.wYear, stFin.wMonth, stFin.wDay);

			// Limpiar ListBox de citas anteriores
			SendMessage(hLBCitas, LB_RESETCONTENT, 0, 0);

			// Recorrer citas y filtrar por paciente y fechas
			CitaMedica* c = listaCitas.cabeza;
			bool encontrado = false;

			while (c)
			{
				if (c->idPaciente == paciente->id &&
					c->fecha >= fechaInicio && c->fecha <= fechaFin)
				{
					encontrado = true;

					// Resumen para mostrar en el ListBox
					char texto[200];
					sprintf_s(texto, sizeof(texto), "Fecha: %s | Hora: %s | M�dico: %s | Estatus: %s",
						c->fecha.c_str(),
						c->hora.c_str(),
						listaMedicos.buscarNombrePorCedula(c->idMedico).c_str(),
						c->estatus.c_str());

					int idx = SendMessage(hLBCitas, LB_ADDSTRING, 0, (LPARAM)texto);
					SendMessage(hLBCitas, LB_SETITEMDATA, idx, (LPARAM)c);
				}
				c = c->sig;
			}
			// Mostrar ID del paciente
			SetDlgItemInt(hDlg, EC_ID_REPCITASPACIENTE, paciente->id, FALSE);

			// Mostrar nombre completo del paciente
			char nombreCompleto[150];
			sprintf_s(nombreCompleto, "%s %s %s", paciente->apellidoPaterno, paciente->apellidoMaterno, paciente->nombre);
			SetDlgItemText(hDlg, EC_NOMBREPACIENTE_REPCITASPACIENTE, nombreCompleto);

			if (!encontrado) {
				MessageBox(hDlg, "No se encontraron citas para este paciente en ese rango de fechas.", "Sin resultados", MB_OK | MB_ICONINFORMATION);
				return TRUE; // Detiene el resto del manejo del bot�n Confirmar
			}
			break;
		}

		case BT_SALIR_REPCITASPACIENTE:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}

	return FALSE;
}