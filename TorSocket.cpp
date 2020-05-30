#include<iostream>
#include<cstring>
#include<cstdlib>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<unistd.h>
#include<signal.h>
 
class TorSocket{
   private:
   int SocketD;
   public:
   int Connect(const char *cHostname, int iPort, const char *torhost = "127.0.0.1", const char *torport = "9050"){
      struct sigaction act;
      memset(&act, 0, sizeof(act));
      act.sa_handler = SIG_IGN;
      act.sa_flags = SA_RESTART;
      sigaction(SIGPIPE, &act, 0);
      struct addrinfo client, *addrs, *addr;
      char rBuffer[10];
      int iReceived = 0;
      memset(&client, 0, sizeof(client));
      client.ai_family = AF_UNSPEC;
      client.ai_socktype = SOCK_STREAM;
      client.ai_protocol = IPPROTO_TCP;
      int iStatus = getaddrinfo(torhost, torport, &client, &addrs);
      if(iStatus!=0){
         std::cout<<"Error getaddrinfo\n";
         return -1;
      }
      for(addr = addrs; addr != nullptr; addr = addr->ai_next){
         if((this->SocketD = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1){
            continue;
         }
         if(connect(this->SocketD, addr->ai_addr, addr->ai_addrlen) == -1){
            continue;
         }
         break;
      }
      freeaddrinfo(addrs);
      if(this->SocketD == -1){
         std::cout<<"No se pudo conectar al proxy\n";
         return -1;
      }
      char cByteSequense1[3] = {0x05, 0x01, 0x00}; 
      send(this->SocketD, cByteSequense1, 3, 0);
      iReceived = recv(this->SocketD, rBuffer, 10, 0);
      if(iReceived > 0){
         if(rBuffer[0] != 0x05){   
            std::cout<<"Version de socks invalida\n";
            return -1;
         }
         if(rBuffer[1] == 0x00){
            char cByteSequense2[256];
            memset(cByteSequense2, 0, 256);
            cByteSequense2[0] = 0x05; 
            cByteSequense2[1] = 0x01; 
            cByteSequense2[2] = 0x00; 
            cByteSequense2[3] = 0x03;
            short sHost_Port = htons(iPort);
            char cHost_Len = (char)strlen(cHostname);
            memcpy(cByteSequense2 + 4, &cHost_Len, 1);
            memcpy(cByteSequense2 + 5, cHostname, cHost_Len);
            memcpy(cByteSequense2 + 5 + cHost_Len, &sHost_Port, 2);
            send(this->SocketD, cByteSequense2, 7 + cHost_Len, 0);
            memset(rBuffer, 0, sizeof(rBuffer));
            iReceived = recv(this->SocketD, rBuffer, 10, 0);
            if(rBuffer[1] == 0x00){
               std::cout<<"Conexion satisfatoria\n";
               //Ahora se puede enviar datos atraves de este socket
               return this->SocketD;
            } else {
               return -1;
            }
         }
      } else {
         return -1;
      }
      return -1;
   }
 
   void CloseSocket(){
      close(this->SocketD);
   }
 
   int SendData(const char *data, int size){
      return send(this->SocketD, data, size, 0);
   }
 
   int ReadData(char*& buffer){
      buffer = (char *)malloc(2048);
      unsigned int so_far = 0;
      char u_c[2];
      while(recv(this->SocketD, u_c, 1, 0) > 0){
         if(so_far >= 2048){
            buffer = (char *)realloc(buffer, so_far + 1);
         }
         buffer[so_far++] = u_c[0];
      }
      buffer[so_far] = '\0';
      return so_far;
   }
};
 
//Simple ejemplo de peticion HTTP a url specificada
int main(int argc, char **argv){
   if(argc < 2){
      std::cout<<argv[0]<<" url\n";
      return 0;
   }
   TorSocket tor;
   char domain[128];
   char *response;
   strncpy(domain, argv[1], 128);
   int stat = tor.Connect(domain, 80);
   if(stat == -1){
      std::cout<<"No se pudo conectar\n";
      return -1;
   }
   std::cout<<"Conectado\n";
   char packet[512];
   snprintf(packet, 512, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", domain);
   if(tor.SendData(packet, strlen(packet)) > 0){
      unsigned int bytes = tor.ReadData(response);
      if(bytes>0){
         std::cout<<response<<"\n\n";
      } else {
         std::cout<<"No se recibio nada\n";
      }
      free(response);
   } else {
      std::cout<<"Error al enviar el paquete\n";
   }
   tor.CloseSocket();
   return 0;
}
 