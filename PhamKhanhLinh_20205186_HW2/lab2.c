#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX 9999

struct hostent *host;
struct in_addr **addr_list;
struct in_addr ip_addr;
// hostent is a structure that contains information about a host such as host name, IP address, and aliases
// in_addr is a structure that contains an IP address

void getIpFromDomain(char domain[])
{
  // check whether domain name exists
  if ((host = gethostbyname(domain)) == NULL)
  {
    herror("Not found information\n");
    return;
  }

  addr_list = (struct in_addr **)host->h_addr_list;
  printf("Official IP:%s\n", inet_ntoa(*addr_list[0]));
  // inet_ntoa() to convert IP addresses from 32-bit integers to strings to print to the screen.

  // print list of Alias IP addresses
  if (addr_list[1] != NULL)
  {
    printf("Alias IP:\n");
    for (int i = 1; addr_list[i] != NULL; i++)
    {
      printf("%s\n", inet_ntoa(*addr_list[i]));
    }
  }
}

void getDomainFromIp(char ip[])
{
  // convert IP string to IP 32-bit numbers
  inet_aton(ip, &ip_addr);
  // get domain name information of a given IP address.
  host = gethostbyaddr(&ip_addr, sizeof(ip_addr), AF_INET);
  if (host == NULL)
  {
    herror("Not found information");
    return;
  }
  printf("Official name: %s \n", host->h_name);

  // print list of Alias name
  if (host->h_aliases[0] != NULL)
  {
    printf("Alias name: \n");
    for (int i = 0; host->h_aliases[i] != NULL; i++)
    {
      printf("%s\n", host->h_aliases[i]);
    }
  }
}

int main()
{
  struct in_addr addr;
  char input[MAX];
  scanf("%s", input);

  // check the input is domain or IP address
  int result = inet_pton(AF_INET, input, &addr);
  if (result == 1){
    getDomainFromIp(input);        // Input is not an IP address, resolve to IP address
  }
  else{
    getIpFromDomain(input);    // Input is an IP address, resolve to domain name

  }
  
    return 0;
  }
