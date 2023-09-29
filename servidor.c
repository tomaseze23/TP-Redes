#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int myAtoi(char* str);

int main(int argc, char* argv[]){
    while(1){
        system("cls");

        WSADATA wsa;

        SOCKET skt, skt2;
        int longitud_cliente, puerto=5005, recv_size;
        struct sockaddr_in server, cliente;
        char mensaje[2000];

        /*====================INICIALIZACI�N WINSOCK====================*/
        printf("Inicializando Winsock...\n");
        if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
            printf("Error al inicializar el winsock\n");
            exit(-1);
        }
        printf("\nWinsock inicializado.\n");

        /*====================CREACI�N SOCKET====================*/
        printf("\nCreando socket...\n");
        if((skt = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
            printf("Error creando socket\n");
            exit(-1);
        }
        printf("\nSocket creado.\n");

        /*====================CONFIGURACION SOCKADDR_IN====================*/
        server.sin_family = AF_INET; //familia TCP/IP
        server.sin_port = htons(puerto); //puerto
        server.sin_addr.s_addr = INADDR_ANY; // cualquier cliente puede conectarse

        /*====================REALIZAR EL BIND====================*/
        if(bind(skt, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
            printf("Error al realizar el bind\n");
            exit(-1);
        }
        printf("\nBind realizado.\n");

        listen(skt, 5);
        printf("Esperando conexiones entrantes...");

        longitud_cliente = sizeof(struct sockaddr_in);
        skt2 = accept(skt, (struct sockaddr *)&cliente, &longitud_cliente);
        if(skt2 == INVALID_SOCKET){
            printf("Fallo al aceptar la conexion\n");
        }

        system("cls");
        printf("Cliente %s conectado exitosamente.\n", inet_ntoa(cliente.sin_addr));
        int opcion;
        do{
            if(send(skt2,"Ingrese numero distinto de 0 para entrar al sistema: ",strlen("Ingrese numero distinto de 0 para entrar al sistema: "), 0) < 0){
                printf("Error al enviar mensaje\n");
                exit(-1);
            }
            if((recv_size = recv(skt2, mensaje, 2000, 0)) == SOCKET_ERROR){
                printf("Recepcion fallida\n");
            }

            mensaje[recv_size] = '\0';
            opcion = myAtoi(mensaje);
            printf("Cliente conectado exitosamente. Aguardando su salida.\n");
        }while(opcion!=0);

        system("pause");

        closesocket(skt);
        closesocket(skt2);
        WSACleanup();
    }
    return 0;
}

int myAtoi(char* str){
    // Initialize result
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
    // return result.
    return res;
}
