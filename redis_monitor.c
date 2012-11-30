#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "common/hiredis.h"

#define TIME_STEP	10 /*seconds.*/

#define IP		"218.30.115.72"
#define PORT	6378

/*srv item.*/
typedef struct _srv_it_{
		char ip[128];
		int port;
		char info[4096];
} SRV_ITEM;

/*redis & reply.*/
redisContext *monitor_srv;
redisReply *reply;

SRV_ITEM *psrv;
int srv_num;

/*global time.*/
time_t g_tm;

/*static data.*/
typedef struct _st_param_{
		char *cap;
}ST_PARAM;

/*static data table.*/
ST_PARAM st_param_tb[] = {
		{"redis_version"},
		{"arch_bits"},
		{"gcc_version"},
		{"process_id"},
		{"os"},
		{"redis_git_sha1"},
		{"multiplexing_api"},
		{"run_id"},
		{"tcp_port"},
		{"uptime_in_seconds"},
		{"uptime_in_days"},
		{"connected_clients"},
		{"used_memory"},
		{"used_memory_rss"},
		{"used_memory_peak"},
		{"used_memory_lua"},
		{"mem_fragmentation_ratio"},
		{"mem_allocator"},
		{"rdb_last_save_time"},
		{"aof_enabled"},
		{"total_connections_received"},
		{"total_commands_processed"},
		{"expired_keys"},
		{"keyspace_hits"},
		{"keyspace_misses"},
		{"role"},
		{"connected_slaves"},
		{"used_cpu_sys"},
		{"used_cpu_user"},
		{"used_cpu_sys_children"},
		{"used_cpu_user_children"}
};

/*dynamic data.*/
typedef struct _dy_param_{
		int db;
		char *dy_cap;
}DY_PARAM;

/*dynamic data table.*/
DY_PARAM dy_param_tb[] = {
		{2,"total_connections_received"},
		{3,"used_cpu_user"},
		{4,"used_cpu_sys"},
		{5,"used_memory"},
		{6,"used_memory_rss"},
		{7,"used_memory_peak"}
};


static void parse(char* ip,int port,char *info)
{
		/*assert info is not empty.*/
		if(!strlen(info)) return ;

		redisContext *redis;
		redisReply *reply;
		struct timeval timeout = { 1, 0 };
		
		/*connect redis_monitor.*/
		redis = redisConnectWithTimeout(IP,PORT, timeout);
		if (redis->err) 
		{
				printf("Connection error: %s IP:%s Port:%d\n", redis->errstr,ip,port);
		}
		/*import ALL info information.*/
		reply = redisCommand(redis, "SELECT 1");
		reply = redisCommand(redis, "HSET ALL %s:%d %s",ip,port,info);
		
		char *pst ,*ped;
		pst = info;

		/*parse info string.*/
		while((ped = strstr(pst,"\r\n")))
		{
				memset(ped,0,2);
				char *pcolon = strchr(pst,':');

				int i;
				for(i = 0; pcolon && ( i < sizeof(st_param_tb)/sizeof(ST_PARAM));i++)
				{
						memset(pcolon,0,1);
						ST_PARAM *st_p = &st_param_tb[i];
						int len = strlen(st_p->cap);
						if(!strcmp(pst,st_p->cap))
						{
								/*import static info.*/
								reply = redisCommand(redis, "SELECT 1");
								reply = redisCommand(redis,"HSET %s %s:%d %s",st_p->cap,ip,port,pst+len+1);
						}
						int j;
						for(j = 0;j<sizeof(dy_param_tb)/sizeof(DY_PARAM);j++)
						{
								DY_PARAM *dy_p = &dy_param_tb[j];
								if(!strcmp(pst,dy_p->dy_cap))
								{
										/*import dynamic info.*/
										reply = redisCommand(redis, "SELECT %d",dy_p->db);
										reply = redisCommand(redis,"HSET %s:%d %d %s",ip,port,g_tm,pst+strlen(dy_p->dy_cap)+1);
								}
						}
				}
				pst = ped+2;
		}
		redisFree(redis);
}

/*parse all.*/
static void parseall()
{
		int i;
		/*parse every one.*/
		for(i = 0 ;i < srv_num;i++) parse(psrv[i].ip, psrv[i].port ,psrv[i].info);	
}

/*get all info from redis by infoCommand.*/
static void get_srv_info()
{
		redisContext *redis;
		redisReply *reply;
		struct timeval timeout = { 1, 0 };
		int i;
		for(i = 0;i < srv_num;i++)
		{
				SRV_ITEM *p = &psrv[i];
				
				/*connect.*/
				redis = redisConnectWithTimeout((char*)p->ip, p->port, timeout);
				if (redis->err) 
				{
						printf("Connection error: %s IP:%s Port:%d\n", redis->errstr,p->ip,p->port);
						memset(p->info,0,4096);
						continue;
				}
				
				/*get info.*/
				reply = redisCommand(redis, "INFO");
				if(reply) strcpy(p->info,reply->str);
				redisFree(redis);
		}
}

/*connect monitor.*/
static int connect_monitor()
{
		/*var.*/
		struct timeval timeout = { 1, 0 }; // 1.5 seconds

		/*connect to monitor srv.*/
		monitor_srv = redisConnectWithTimeout((char*)IP, 6378, timeout);
		if (monitor_srv->err) 
		{
				printf("Connection error: %s IP:%s Port:%d\n", monitor_srv->errstr,IP,PORT);
				exit(1);
		}

		/*get server_list.*/
		reply = redisCommand(monitor_srv, "SELECT 0");
		reply = redisCommand(monitor_srv, "SMEMBERS server_list ");
		
		srv_num = reply->elements;
		if(!srv_num) return -1;/*no server exist ,do nothing.*/
	
		/*realloc for all srv.*/
		psrv = (SRV_ITEM* )realloc(psrv,srv_num*sizeof(SRV_ITEM));
		if(!psrv)
		{
				fprintf(stderr,"realloc failed.\n");
				return -1;
		}

		memset(psrv,0,sizeof(SRV_ITEM)*srv_num);

		int i;
		for( i=0 ; i< srv_num ; i++)
				sscanf(reply->element[i]->str,"%s %d",psrv[i].ip, &psrv[i].port);

		redisFree(monitor_srv);
		return 0;
}

/*main*/
int main(int argc, char** argv)
{
		fprintf(stdout,"%s\'pid: %d\r\n",argv[0],getpid());
		while(1)
		{
				sleep(1);
				time_t t = time(0);
				if(t%TIME_STEP) continue ;
				g_tm = t;

				/*connect monitor.*/
				if(-1==connect_monitor())
				{
						fprintf(stderr,"no server exist, then exit.\n");
						exit(0);
				}

				/*get info.*/
				get_srv_info();
				parseall();
		}
		return 0;
}

