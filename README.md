redis_monitor
=============

监控redis集群.

1: 添加服务器
	
	% add.sh "127.0.0.1 6379"


2: 功能:
以下数据已受监控:

类静态监控:
	
	% connected_clients
	% redis_git_sha1
	% rdb_last_save_time
	% keyspace_misses
	% used_memory
	% used_memory_peak
	% used_memory_lua
	% process_id
	% tcp_port
	% mem_fragmentation_ratio
	% keyspace_hits
	% expired_keys
	% total_connections_received
	% arch_bits
	% role
	% gcc_version
	% used_cpu_user
	% used_cpu_sys
	% uptime_in_seconds
	% ALL
	% uptime_in_days
	% multiplexing_api
	% total_commands_processed
	% redis_version
	% used_cpu_user_children
	% used_cpu_sys_children
	% mem_allocator
	% connected_slaves
	% aof_enabled
	% used_memory_rss
	% run_id
	% os

动态数据监控:
	
	% total_connections_received
	% used_cpu_user
	% used_cpu_sys
	% used_memory
	% used_memory_rss
	% used_memory_peak
	% keyspace_hits
	% keyspace_misses
	% expired_keys
	% connected_slaves
	% mem_fragmentation_ratio
	% total_commands_processed
	% rdb_last_save_time(老版本的redis是last_save_time)
	% uptime_in_seconds
	% uptime_in_days

