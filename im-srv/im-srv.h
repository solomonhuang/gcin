typedef enum {
  Connection_type_unix = 1,
  Connection_type_tcp = 2
} Connection_type;

typedef struct {
  ClientState *cs;
  int tag;
  unsigned long seed;
  Connection_type type;
} GCIN_ENT;

extern GCIN_ENT *gcin_clients;
extern int gcin_clientsN;
extern Server_IP_port srv_ip_port;
