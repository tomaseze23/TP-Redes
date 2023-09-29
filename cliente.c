#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <time.h>
#include <ctype.h>

#define MAX 80
#define ARCHIVO_USUARIOS "credenciales.txt"
#define ARCHIVO_TRADUCCION "palabras.txt"
#define ARCHIVO_ACTIVIDAD "server.log"
#define TECLA_ENTER 13
#define TECLA_BACKSPACE 8
#define LONGITUD 5
#define MAX_INTENTOS 3

//estructura del usuario
struct usuario {
    char nombre[MAX];
    char contrasenia[MAX];
    char rol [MAX];
    char intentos[MAX];
};

typedef struct usuario Usuario;
//funciones de menu de usuario
void menuInicial();
void menuListarUsuarios();
void menuRegistrarUsuario();
void menuIniciarSesion();
void menuSistema(Usuario usuario);
void menuDesbloquearUsuarios();

//manipulacion del archivo de usuarios
char insertarUsuario(Usuario usuario);
char existeUsuario(char nombreUsuario[], Usuario *usuario);
void modificacionIntentos(int intentos, char nombre[]);
Usuario *obtenerUsuarios(int *);
void cargarUsuario(char lectura[MAX], int i);
char logear(char nombreUsuario[], char password[]);

//estructura del traductor
struct traductor{
    char formato[MAX];
    char eng[MAX];
    char esp[MAX];
}traduccionArchivo[MAX];

typedef struct traductor Traductor;

//menus del traductor
void menuAniadirPalabras();
void traducir();

//manipulacion de archivos de traductor
int validarFormato(char palabra[]);
char insertarPalabras(Traductor traductor);
char existePalabra(char palabra[]);
int obtenerPalabras(int *);
void cargarTraduccion(char lectura[], int i);
char buscarTraduccion(char palabra[]);
void palabraAMinuscula(char palabra[]);

//registro de actividades
char insertarActividad(char mensaje[]);
void registroActividades();

char retornarFechaHora(char fechaYHora[MAX]);

int leerLinea(char *cad, int n);

char linea[MAX];

int myAtoi(char* str);

int main(int argc, char* argv[]){
    WSADATA wsa;
    SOCKET skt;
    int puerto=0, recv_size;
    char mensaje[2000];
    char actividad[2000];
    char fecha[MAX];
    struct sockaddr_in server;
    retornarFechaHora(fecha);
    strcpy(actividad, fecha);
    strcat(actividad, "\n=============================\n");
    strcat(actividad, "=======Inicia Servidor=======\n");
    strcat(actividad, "=============================\n");
    insertarActividad(actividad);
    /*====================INICIALIZACIÓN WINSOCK====================*/
    printf("Inicializando Winsock...\n");
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
        printf("Error al inicializar el winsock\n");
        exit(-1);
    }
    printf("\nWinsock inicializado.\n");

    /*====================CREACIÓN SOCKET====================*/
    printf("\nCreando socket...\n");
    if((skt = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
        printf("Error creando socket\n");
        exit(-1);
    }

    /*====================CONFIGURACION SOCKADDR_IN====================*/
    server.sin_family = AF_INET;
    printf("ingrese puerto: ");
    fflush(stdin);
    scanf("%d", &puerto);
    server.sin_port = htons(puerto);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(skt, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Error de conexion\n");
        exit(-1);
    }
    printf("Conexion exitosa.\n");
    printf("\nSocket creado.\n");
    retornarFechaHora(fecha);
    strcpy(actividad, fecha);
    strcat(actividad, ": Socket creado. Puerto de escucha: ");
    char puertoAux[MAX];
    itoa(puerto, puertoAux, 10);
    strcat(actividad, puertoAux);
    insertarActividad(actividad);
    int opcion=-1;
    do{
        //ingreso al sistema
        if((recv_size = recv(skt, mensaje, 2000, 0)) == SOCKET_ERROR){
            printf("Recepcion fallida\n");
        }
        mensaje[recv_size] = '\0';
        printf("\nServer - %s",mensaje);
        fflush(stdin);
        gets(mensaje);
        opcion = myAtoi(mensaje);
        //envio al servidor del dato para permitir acceso al sistema
        if(send(skt, mensaje, strlen(mensaje), 0) < 0){
            printf("Error al enviar mensaje\n");
            exit(-1);
        }
        if(opcion!=0)
            menuInicial();
    }while(opcion!=0);

    printf("\n\nElegiste salir! Adios!");


    system("pause");

    closesocket(skt);
    WSACleanup();

    return 0;
}

void menuInicial(){
    char repite = 1;
    int opcion=-1;
    do{
        system("cls");
        printf("\n\t\t\tMENU INICIAL\n");
        printf("\t\t\t============\n");
		printf("\n\t\t[1]. Ver usuarios registrados\n");
		printf("\t\t[2]. Registrar usuario nuevo\n");
		printf("\t\t[3]. Ingresar al sistema\n");
		printf("\t\t[0]. Salir\n");
		printf("\n\t\tIngrese su opcion: [ ]\b\b");
		leerLinea(linea, MAX);
		sscanf(linea, "%d", &opcion);

		switch(opcion) {
            case 1:
                    menuListarUsuarios();
                    break;
            case 2: menuRegistrarUsuario();
                    break;
            case 3:
                    menuIniciarSesion();
                    break;
            case 0: //exit(1);
                    repite = 0;
                    break;
		}
    } while(repite == 1);
}

void menuRegistrarUsuario() {
    Usuario usuario;
    char nombreUsuario[MAX];
    char respuesta[MAX];
    char actividad[MAX];
    char fecha[MAX];
    char repite = 1;
    int intentos = 0;

    do {
		system("cls");
		printf("\n\t\t\tREGISTRAR USUARIO\n");
		printf("\t\t\t=================\n");
		printf("\n\tIngrese nombre de usuario: ");
		leerLinea(linea, MAX);
		sscanf(linea, "%s", nombreUsuario);
        if(nombreUsuario[0] =='\0'){
            printf("\tError al dar de alta el nuevo usuario: datos incompletos");
            getchar();
            break;
        }
		//verificamos que el usuario no exista
		if(!existeUsuario(nombreUsuario, &usuario)) {
                strcpy(usuario.nombre, nombreUsuario);

                printf("\tIngrese la clave: ");
                leerLinea(usuario.contrasenia, MAX);
                if(usuario.contrasenia[0] =='\0'){
                    printf("\tError al dar de alta el nuevo usuario: datos incompletos");
                    getchar();
                    break;
                }
                if(strcmp(usuario.nombre, "admin")==0){
                    strcpy(usuario.rol, "ADMIN");
                }else{
                    strcpy(usuario.rol, "CONSULTA");
                }

                itoa(intentos, usuario.intentos, 10);

                //insertamos el usuario en el archivo
                if(insertarUsuario(usuario)) {
                    printf("\n\t[%s] dado de alta correctamente\n", usuario.nombre);
                    retornarFechaHora(fecha);
                    strcpy(actividad, fecha);
                    strcat(actividad, ": Usuario registrado: ");
                    strcat(actividad, usuario.nombre);
                    insertarActividad(actividad);
                } else {
                    printf("\n\tOcurrio un error al registrar el usuario.\n");
                    printf("\nIntentelo mas tarde.\n");
                }
		} else {
		    printf("\n\tEl usuario \"%s\" ya ha sido registrado previamente\n", nombreUsuario);
			printf("\tNo puede registrar dos usuarios con el mismo nombre de usuario.\n");
		}

        printf("\n\tDesea seguir registrando usuarios? [S/N]: ");
		leerLinea(respuesta, MAX);

		if (!(strcmp(respuesta, "S") == 0 || strcmp(respuesta, "s") == 0)) {
			repite = 0;
		}
    } while(repite == 1);
}

void menuListarUsuarios() {
    int numeroUsuarios = 0;
    Usuario *usuarios;
    int i;
    system("cls");
    //vector dinamico de usuarios
    usuarios = obtenerUsuarios(&numeroUsuarios);

    if(numeroUsuarios == 0){
        printf("\n\tNo existe ningun usuario registrado.\n");
    } else{
		printf("\n\t\t    ==> LISTADO DE USUARIOS REGISTRADOS <==\n");
		printf(" --------------------------------------------------------------------------------------------------------------\n");
		printf("%10s%25s%25s%25s%25s\n", "#", "NOMBRE", "PASSWORD", "ROL", "INTENTOS");
		printf(" --------------------------------------------------------------------------------------------------------------\n");

		/* Se recorre el vector dinámico de productos */
		for (i = 0; i < numeroUsuarios; i++) {
			printf("%10d%25s%25s%25s%25s\n", (i + 1), usuarios[i].nombre, usuarios[i].contrasenia, usuarios[i].rol, usuarios[i].intentos);
		}
		printf(" --------------------------------------------------------------------------------------------------------------\n");
    }

    //liberamos la memoria del vector dinamico
    free(usuarios);
    usuarios = NULL;
    getchar();
}

void menuIniciarSesion() {
    char nombreUsuario[MAX];
    char contrasenia[MAX];
    char maximoIntentos [MAX] = "3";
    char actividad [MAX];
    char fecha[MAX];
    int intento = 0;
    int loginExitoso = 0;
    Usuario usuario;
    int posicion=0;
    do{
        system("cls");
		printf("\n\t\t\tINGRESAR AL SISTEMA\n");
		printf("\t\t\t===================\n");

		printf("\n\t\tUSUARIO: ");
		leerLinea(linea, MAX);
		sscanf(linea, "%s", nombreUsuario);
        if(nombreUsuario[0] =='\0'){
            printf("\tError al dar de alta el nuevo usuario: datos incompletos");
            getchar();
            break;
        }
        if(existeUsuario(nombreUsuario, &usuario)){
            if(strcmp(usuario.intentos, maximoIntentos) == 0){
                printf("\n\tSu usuario se encuentra bloqueado");
                break;
            }

            printf("\t\tCLAVE: ");
            leerLinea(linea, MAX);
            sscanf(linea, "%s", contrasenia);
            if(contrasenia[0] =='\0'){
                printf("\tError al dar de alta el nuevo usuario: datos incompletos");
                getchar();
                break;
            }
            if (logear(nombreUsuario, contrasenia)) {
                loginExitoso = 1;
                retornarFechaHora(fecha);
                strcpy(actividad, fecha);
                strcat(actividad, ": Inicio de sesion – usuario: ");
                strcat(actividad, nombreUsuario);
                insertarActividad(actividad);
            } else {
                printf("\n\n\t\tContrase%ca incorrecta", 164);
                intento++;
                modificacionIntentos(intento, nombreUsuario);
                getchar();
            }
        } else{
            printf("\n\n\t\tEl usuario que desea ingresar no se encuentra en nuestro archivo.");
            getchar();
        }
    } while (intento < MAX_INTENTOS && loginExitoso == 0);

    if (loginExitoso == 1) {
        menuSistema(usuario);
	} else {
		printf("\n\tSe ha superado la cantidad maxima de intentos, la cuenta %s ha sido bloqueada.\n", usuario.nombre);
		getchar();
	}
}

void menuDesbloquearUsuarios(){
    int numeroUsuarios = 0;
    system("cls");
    Usuario *usuarios;
    Usuario usuario;
    char nombre[MAX];
    char respuesta[MAX];
    char maximosIntentos [MAX] = "3";
    char actividad[MAX];
    char fecha[MAX];
    int i, bloqueado = 0;
    //vector dinamico de usuarios
    usuarios = obtenerUsuarios(&numeroUsuarios);

    if(numeroUsuarios == 0){
        printf("\n\tNo existe ningun usuario registrado.\n");
    } else{
		printf("\n\t\t    ==> LISTADO DE USUARIOS BLOQUEADOS <==\n");
		printf(" ------------------------------------------------------------\n");
		printf("%10s%25s%25s\n", "#", "NOMBRE", "INTENTOS");
		printf(" ------------------------------------------------------------\n");

		 //Se recorre el vector dinámico de productos
		for (i = 0; i < numeroUsuarios; i++) {
            if(strcmp(usuarios[i].intentos, maximosIntentos)==0){
                printf("%10d%25s%25s\n", (i + 1), usuarios[i].nombre, usuarios[i].intentos);
                bloqueado=1;
            }
        }
        if(bloqueado==0){
            printf("\n\tNo existe ningun usuario bloqueado.\n");
            printf(" -------------------------------------------------------------\n");
        }
    }
    if(bloqueado == 1){
        printf("\n\t\tDesea desbloquear un usuario? [S/N]: ");
		leerLinea(respuesta, MAX);
		if ((strcmp(respuesta, "S") == 0 || strcmp(respuesta, "s") == 0)) {
            printf("\n\t\tIngrese el usuario que desea desbloquear: ");
            leerLinea(linea, MAX);
            sscanf(linea, "%s", nombre);
            if(existeUsuario(nombre, &usuario)){

                if(strcmp(usuario.intentos, maximosIntentos)==0){
                    strcpy(usuario.intentos, "0");
                    modificacionIntentos(0, nombre);
                    printf("\n\t\tSu usuario fue desbloqueado con exito.\n");
                    retornarFechaHora(fecha);
                    strcpy(actividad, fecha);
                    strcat(actividad, ": Usuario desbloqueado: ");
                    strcat(actividad, usuario.nombre);
                    insertarActividad(actividad);
                }else{
                    printf("\n\t\tEl usuario %s no se encuentra bloqueado.", usuario.nombre);
                }
            }else{
                printf("\n\t\tEl usuario no existe");
            }
        }

    }
    //liberamos la memoria del vector dinamico
    free(usuarios);
    usuarios = NULL;

    getchar();
}

void menuUsuarios(Usuario usuario){
    int repite = 1;
    int opcion = -1;
    printf("\n     ======================================================================\n");
	printf("\t\t\t     BIENVENIDO %s AL PANEL DE USUARIOS\n", usuario.nombre);
	printf("     ======================================================================\n");
	do{
        system("cls");
        printf("\n\t\t\tMENU DE USUARIO\n");
        printf("\t\t\t============\n");
        printf("\n\t\t[1]. ALTA\n");
        printf("\t\t[2]. DESBLOQUEO\n");
        printf("\t\t[0]. Salir\n");
        printf("\n\t\tIngrese su opcion: [ ]\b\b");
        leerLinea(linea, MAX);
        sscanf(linea, "%d", &opcion);

        switch (opcion){
            case 1: menuRegistrarUsuario();
                    break;
            case 2: menuDesbloquearUsuarios();
                    break;
            case 0: repite = 0;
                    break;
        }
	} while(repite == 1);

}

void menuAdmin(Usuario usuario){
    char actividad[MAX];
    char fecha[MAX];
    int repite = 1;
    int opcion = -1;
    printf("\n     ======================================================================\n");
	printf("\t\t\t     BIENVENIDO %s A NUESTRO SISTEMA\n", usuario.nombre);
	printf("\t\t    Usted como %s podra agregar nuevas traducciones y gestionar ingreso y desbloqueo de usuarios.\n", usuario.rol);
	printf("     ======================================================================\n");
	getchar();
    do{
        system("cls");
        printf("\n\t\t\tMENU INICIAL\n");
        printf("\t\t\t============\n");
        printf("\t\t[1]. Nueva traduccion (ROL ADMIN)\n");
        printf("\t\t[2]. Usuarios (ROL ADMIN)\n");
        printf("\t\t[3]. Ver registro de actividades (ROL ADMIN)\n");
        printf("\t\t[0]. Cerrar sesion\n");
        printf("\n\t\tIngrese su opcion: [ ]\b\b");
        leerLinea(linea, MAX);
        sscanf(linea, "%d", &opcion);

        switch(opcion){
            case 1: menuAniadirPalabras();
                    break;

            case 2: menuUsuarios(usuario);
                    break;

            case 3: registroActividades();
                    break;

            case 0: retornarFechaHora(fecha);
                    strcpy(actividad, fecha);
                    strcat(actividad, ": Cierre de sesion – usuario: ");
                    strcat(actividad, usuario.nombre);
                    insertarActividad(actividad);
                    repite = 0;
                    break;
        }
    }while(repite == 1);

    getchar();
}

void menuConsulta(Usuario usuario){
    char actividad[MAX];
    char fecha[MAX];
    int repite = 1;
    int opcion = -1;
    printf("\n     ======================================================================\n");
	printf("\t\t\t     BIENVENIDO %s A NUESTRO SISTEMA\n", usuario.nombre);
	printf("\t\t    Usted con su rol de %s solo podra buscar traducciones.\n", usuario.rol);
	printf("     ======================================================================\n");
	getchar();
    do{
        system("cls");
        printf("\n\t\t\tMENU INICIAL\n");
        printf("\t\t\t============\n");
        printf("\n\t\t[1]. Traducir (ROL CONSULTA)\n");
        printf("\t\t[0]. Cerrar sesion\n");
        printf("\n\t\tIngrese su opcion: [ ]\b\b");
        leerLinea(linea, MAX);
        sscanf(linea, "%d", &opcion);

        switch(opcion){
            case 1: traducir();
                    break;

            case 0:
                    retornarFechaHora(fecha);
                    strcpy(actividad, fecha);
                    strcat(actividad, ": Cierre de sesion – usuario: ");
                    strcat(actividad, usuario.nombre);
                    insertarActividad(actividad);
                    repite = 0;
                    break;
        }
    }while(repite == 1);

    getchar();
}

void menuSistema(Usuario usuario){
    system("cls");
    if(strcmp(usuario.rol, "ADMIN")==0){
        menuAdmin(usuario);
    }else{
        menuConsulta(usuario);
    }

}

//retornamos 1 si se pudo registrar correctamente el usuario en el archivo
char insertarUsuario(Usuario usuario) {
    FILE *archivo;
    char insercion = 0;
    archivo = fopen(ARCHIVO_USUARIOS, "ab");
    if (archivo == NULL) {
        insercion = 0;
    } else {
		fwrite(&usuario, sizeof(usuario), 1, archivo);
		//fprintf(archivo, "%s|%s|%s|%s\n", usuario.nombre, usuario.contrasenia, usuario.rol, usuario.intentos);
        insercion = 1;
        fclose(archivo);
    }
    return insercion;
}

// retorna 1 si existe el nombre de usuario. retorna el usuario buscado si existe
char existeUsuario(char nombreUsuario[], Usuario *usuario) {
    char existe;
    FILE *archivo;
	/* Abre el archivo en modo de lectura */
	archivo = fopen(ARCHIVO_USUARIOS, "rb");
	if (archivo == NULL) {
		existe = 0;
	} else {
		existe = 0;
		/* Lee secuencialmente del archivo de usuarios */
		fread(&(*usuario), sizeof(*usuario), 1, archivo);
		while (!feof(archivo)) {
			if (strcmp((*usuario).nombre, nombreUsuario) == 0) {
				/* Encuentra un usuario del archivo con el nombre de usuario buscado */
				existe = 1;
				break;
			}
			fread(&(*usuario), sizeof(*usuario), 1, archivo);
		}
		/* Cierra el archivo*/
		fclose(archivo);
	}
	return existe;
}

void modificacionIntentos(int intentos, char nombre[]){
    FILE *arch;
    arch=fopen(ARCHIVO_USUARIOS,"r+b");
    if (arch==NULL)
        exit(1);
    Usuario usuario;
    int existe=0;
    fread(&usuario, sizeof(Usuario), 1, arch);
    while(!feof(arch)){
        if (strcmp(usuario.nombre, nombre)==0){
           itoa(intentos, usuario.intentos, 10);
           int pos=ftell(arch)-sizeof(Usuario);
           fseek(arch,pos,SEEK_SET);
           fwrite(&usuario, sizeof(Usuario), 1, arch);
           existe=1;
           break;
        }
        fread(&usuario, sizeof(Usuario), 1, arch);
    }
    if (existe==0)
        printf("No existe dicho usuario\n");
    fclose(arch);
}

Usuario *obtenerUsuarios(int *n) {
 	FILE *archivo;
	Usuario usuario;
	Usuario *usuarios; /* Vector dinámico de usuarios */
	int i;
	/* Abre el archivo en modo lectura */
	archivo = fopen(ARCHIVO_USUARIOS, "rb");
	if (archivo == NULL) { /* Si no se pudo abrir el archivo, el valor de archivo es NULL */
		*n = 0; /* No se pudo abrir. Se considera n = 0 */
		usuarios = NULL;
	} else {
		fseek(archivo, 0, SEEK_END); /* Posiciona el cursor al final del archivo */
		*n = ftell(archivo) / sizeof(Usuario); /* # de usuarios almacenados en el archivo. (# de registros) */
		usuarios = (Usuario *)malloc((*n) * sizeof(Usuario)); /* Se asigna memoria para todos los usuarios almacenados en el archivo */

		/* Se recorre el archivo secuencialmente */
		fseek(archivo, 0, SEEK_SET); /* Posiciona el cursor al principio del archivo */
		fread(&usuario, sizeof(usuario), 1, archivo);
		i = 0;
		while (!feof(archivo)) {
			usuarios[i++] = usuario;
			fread(&usuario, sizeof(usuario), 1, archivo);
		}
		/* Cierra el archivo */
		fclose(archivo);
	}
    return usuarios;
}

// retorna 1 o 0 si el usuario y contrasenia son correctos para un usuario especifico
char logear(char nombreUsuario[], char contrasenia[]){
	FILE *archivo;
	char logeoExitoso;
	Usuario usuario;
	/* Abre el archivo en modo de lectura */
	archivo = fopen(ARCHIVO_USUARIOS, "rb");
	if (archivo == NULL) {
		logeoExitoso = 0;
	} else {
		logeoExitoso = 0;
		/* Lee secuencialmente del archivo de usuarios */
		fread(&usuario, sizeof(usuario), 1, archivo);
		while (!feof(archivo)) {
			if (strcmp(usuario.nombre, nombreUsuario) == 0 && strcmp(usuario.contrasenia, contrasenia) == 0) {
				/* Encuentra un usuario del archivo con el nombre de usuario y password buscado */
				logeoExitoso = 1;
				break;
			}
			fread(&usuario, sizeof(usuario), 1, archivo);
		}
		/* Cierra el archivo*/
		fclose(archivo);
	}
	return logeoExitoso;
}

void menuAniadirPalabras() {
    Traductor traductor;
    char palabra[MAX];
    char respuesta[MAX];
    char actividad[MAX];
    char fecha[MAX];
    char repite = 1;

    do {
		system("cls");
		printf("\n\t\t\tREGISTRAR PALABRA\n");
		printf("\t\t\t=================\n");
		printf("\n\tIngrese traduccion: ");
		leerLinea(linea, MAX);
		sscanf(linea, "%s", palabra);
		if(validarFormato(palabra)==1){
            //verificamos que la palabra no exista
            if(!existePalabra(palabra)) {
                    strcpy(traductor.formato, palabra);
                    //insertamos el usuario en el archivo
                    if(insertarPalabras(traductor)) {
                        printf("\n\tNueva traduccion insertada correctamente.\n");
                        retornarFechaHora(fecha);
                        strcpy(actividad, fecha);
                        strcat(actividad, ": Traduccion agregada: ");
                        strcat(actividad, traductor.formato);
                        insertarActividad(actividad);
                    } else {
                        printf("\n\tOcurri%c un error al registrar la traduccion.\n", 162);
                        printf("\nIntentelo mas tarde.\n");
                    }
            } else {
                printf("\n\tLas palabras ya han sido registradas previamente\n");
                printf("\tNo se puede cargar dos veces la misma traduccion.\n");
            }
		} else{
            printf("\n\tNo fue posible insertar la traducción. El formato de inserción debe ser palabraEnIngles:traducciónEnEspaniol");
		}
        printf("\n\tDesea seguir registrando palabras? [S/N]: ");
		leerLinea(respuesta, MAX);
		if (!(strcmp(respuesta, "S") == 0 || strcmp(respuesta, "s") == 0)) {
			repite = 0;
		}
    } while(repite == 1);
}

void traducir () {
    system("cls");
    printf("\n\t\t\tTRADUCCION\n");
    printf("\t\t\t==========\n");
    char palabraABuscar[MAX];
    char actividad[MAX];
    char fecha[MAX];
    printf("\n\t\tIntroduce la palabra a buscar: ");
    leerLinea(linea, MAX);
    sscanf(linea, "%s", palabraABuscar);
    buscarTraduccion(palabraABuscar);
    retornarFechaHora(fecha);
    strcpy(actividad, fecha);
    strcat(actividad, ": Usuario hizo una consulta de traduccion.");
    insertarActividad(actividad);
    system("pause");
}

//retornamos 1 si se pudo registrar correctamente las palabras en el archivo
char insertarPalabras(Traductor traductor) {
    FILE *archivo;
    char insercion = 0;
    archivo = fopen(ARCHIVO_TRADUCCION, "ab");
    if (archivo == NULL) {
        insercion = 0;
    } else {
        fprintf(archivo, "%s\n", traductor.formato);
        insercion = 1;
        fclose(archivo);
    }
    return insercion;
}

// retorna 1 si existe el nombre de usuario. retorna el usuario buscado si existe
char existePalabra(char palabra[]) {
    char existe = 0;
    int numeroPalabras = 0;
    int i = 0;
    numeroPalabras = obtenerPalabras(&numeroPalabras);
    palabraAMinuscula(palabra);
    char palabraEnIngles[MAX] = "";
    int finPalabraEnIngles=0;
    int tamanio = strlen(palabra);

    //for para encontrar la primer coma
    for (int j = 0; j<tamanio; j++){
        //Encontre primer coma
        if(palabra[j]==':'){
            finPalabraEnIngles = j;
            j = tamanio;
        }
    }//cierra el for

    //cargo la palabra en ingles
    for (int carga = 0; carga<finPalabraEnIngles; carga++){
        palabraEnIngles[carga] = palabra[carga];
    }

    for(i; i < numeroPalabras; i++){
        if(strcmp(traduccionArchivo[i].eng, palabraEnIngles)==0){
            existe = 1;
            break;
        }
    }
    return existe;
}

int obtenerPalabras(int *n) {
    FILE *archivo;
    Traductor *traductor;
    int i=0;
    archivo = fopen(ARCHIVO_TRADUCCION, "rb");
    if(archivo == NULL) {
            *n = 0;
            traductor = NULL;
    } else {
		fseek(archivo, 0, SEEK_END); /* Posiciona el cursor al final del archivo */
		*n = ftell(archivo) / sizeof(Traductor); /* # de usuarios almacenados en el archivo. (# de registros) */
		traductor = (Traductor *)malloc((*n) * sizeof(Traductor)); /* Se asigna memoria para todos los usuarios almacenados en el archivo */

		/* Se recorre el archivo secuencialmente */
		fseek(archivo, 0, SEEK_SET); /* Posiciona el cursor al principio del archivo */
        while(!feof(archivo)){
            char lectura[MAX];
            fgets(lectura, MAX, archivo);
            if(!feof(archivo)){
                cargarTraduccion(lectura, i);
                i++;
            }
        }
		fclose(archivo);
    }
    return i;
}

void cargarTraduccion(char lectura[MAX], int i){
    int tamanio = strlen(lectura);
    char palabraEnEspaniol[MAX] = "";
    char palabraEnIngles[MAX] = "";
    int finPalabraEnIngles=0;

    //for para encontrar el separador
    for (int j = 0; j<tamanio; j++){
        //Encontre primer coma
        if(lectura[j]==':'){
            finPalabraEnIngles = j;
            j = tamanio;
        }
    }//cierra el for

    //cargo la palabra en ingles
    for (int carga = 0; carga<finPalabraEnIngles; carga++){
        palabraEnIngles[carga] = lectura[carga];
    }
    //cargo la palabra en espaniol
    for (int carga = (finPalabraEnIngles+1); carga<tamanio; carga++){
        palabraEnEspaniol[carga-finPalabraEnIngles-1] = lectura[carga];
        //borro salto de linea
        if(palabraEnEspaniol[carga-finPalabraEnIngles-1]=='\n'){
            palabraEnEspaniol[carga-finPalabraEnIngles-1] = '\0';
        }
    }

    strcpy(traduccionArchivo[i].esp, palabraEnEspaniol);
    strcpy(traduccionArchivo[i].eng, palabraEnIngles);
}

// retorna 1 o 0 si las palabras se encuentran en el archivo
char buscarTraduccion(char palabra[]){
    int numeroPalabras = 0;
    int i, temp = 0;
    numeroPalabras = obtenerPalabras(&numeroPalabras);
    palabraAMinuscula(palabra);
    for (i = 0; i < numeroPalabras; i++) {
            if (strcmp(palabra, traduccionArchivo[i].eng) == 0){
                printf("\n\t\t%s en ingles es %s en espaniol.\n", palabra, traduccionArchivo[i].esp);
                temp = 1;
                break;
            }
    }
    if(temp == 0) {
        printf("\n\t\tNo fue posible encontrar la traduccion para: [%s]\n\n", palabra);
    }
}

void palabraAMinuscula(char palabra[]){
	for (int indice = 0; palabra[indice] != '\0'; ++indice){
		palabra[indice] = tolower(palabra[indice]);
	}
}

int validarFormato(char palabra[]){
    int tamanio = strlen(palabra);
    int existe=0;
    for (int i = 0; i < tamanio; i++){
        if(palabra[i] == ':'){
            existe=1;
        }
    }
    return existe;
}

char insertarActividad(char mensaje[]){
    FILE *archivo;
    char insercion = 0;
    archivo = fopen(ARCHIVO_ACTIVIDAD, "ab");
    if (archivo == NULL) {
        insercion = 0;
    } else {
		fprintf(archivo, "%s\n", mensaje);
        insercion = 1;
        fclose(archivo);
    }
    return insercion;

}

void registroActividades(){
    FILE *arch;
    arch=fopen(ARCHIVO_ACTIVIDAD,"rt");
    if (arch==NULL)
        exit(1);
    char lectura[MAX];
    fgets(lectura,100,arch);
    while(!feof(arch)){
        printf("%s",lectura);
        fgets(lectura,100,arch);
    }
    fclose(arch);
    getchar();
}

int leerLinea(char *cad, int n) {
    int i, c;
    //primero comprobamos datos iniciales en el buffer
    c=getchar();

    if(c == EOF) {
        cad[0] = '\0';
        return 0;
    }

    if(c == '\n') {
        i = 0;
    } else{
        cad[0] = c;
        i = 1;
    }

    //lectura de la cadena
    for(; i < n-1 && (c = getchar())!= EOF && c != '\n'; i++) {
       cad[i] = c;
    }
    cad[i] = '\0';

    //limpieza del buffer
    if(c != '\n' && c != EOF){
        while((c = getchar()) != '\n' && c != EOF);
    }
    return 1;
}

char retornarFechaHora(char fechaYHora[MAX]){
    int hours, minutes, day, month, year;
    char dia[MAX], mes[MAX], anio[MAX], minutos[MAX], horas[MAX];
    char fecha[MAX];
    char hora[MAX];
    // `time_t` es un tipo de tiempo aritmético
    time_t now;

    // Obtener la hora actual
    // `time()` devuelve la hora actual del sistema como un valor `time_t`
    time(&now);

    // Convertir al formato de hora local e imprimir a stdout
    struct tm *local = localtime(&now);

    hours = local->tm_hour;         // obtener horas desde la medianoche (0-23)
    minutes = local->tm_min;        // obtener minutos pasados después de la hora (0-59)

    day = local->tm_mday;            // obtener el día del mes (1 a 31)
    month = local->tm_mon + 1;      // obtener el mes del año (0 a 11)
    year = local->tm_year + 1900;   // obtener el año desde 1900

    //convierno enteros en char
    itoa(hours, horas, 10);
    itoa(minutes, minutos, 10);
    itoa(day, dia, 10);
    itoa(month, mes, 10);
    itoa(year, anio, 10);

    //copio los char para la hora
    strcpy(hora, horas);
    strcat(hora, ":");
    strcat(hora, minutos);

    //copio los char para la fecha
    strcpy(fecha, dia);
    strcat(fecha, "-");
    strcat(fecha, mes);
    strcat(fecha, "-");
    strcat(fecha, anio);

    //copio los char para la fecha y la hora
    strcpy(fechaYHora, fecha);
    strcat(fechaYHora, "_");
    strcat(fechaYHora, hora);

    return fechaYHora;
}

int myAtoi(char* str){
    // Initialize result
    int res = 0;

    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';

    // return result.
    return res;
}
