#define SYN_FLOODING 9550
#define SYSTEM_ATTACK 9500
#define SYSTEM_FAULT 9400
#define MANAGER_UDP_PORT 8081
#define AGENT_TCP_PORT 9000

typedef struct agent
{
	char* ip;
	int port;
	char* host_name;
} agent;

// Statistic Info 
typedef struct statistic
{
	int numofPacket;
	int numofIP;
	int numofARP;
	int numofTCP;
	int numofTCPSYN;
	int numofUDP;
	unsigned int sumofPktLen;
	double bitrate;
}statistic;


// Trap Info 
typedef struct trap
{
	int type;
	int code;

}trap;

