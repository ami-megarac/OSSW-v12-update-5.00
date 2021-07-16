#!/bin/sh
#This script will rename the JSON files and compress the files into /conf

feature_file_name="/etc/core_features"
isFeatureEnabled()
{
	if [ -f $feature_file_name ]; then
		feature=$(grep "$1" $feature_file_name)
		if [ "$feature" == "$1" ]; then
			return 1
		else
			return 0
		fi
	fi
}

dump_dir_file_name="/etc/crashdump/crashdumppath"
if [ -f $dump_dir_file_name ]; then
	target=$(grep "crashdump_dir" $dump_dir_file_name)
	crashdump_dir=$(echo $target | awk -F= '{print $2}')
	if [ -z $crashdump_dir ] || [ $crashdump_dir = "" ]; then
		crashdump_dir="/conf/crashdump"
	fi
fi

isFeatureEnabled CONFIG_SPX_FEATURE_GLOBAL_AUTONOMOUS_CRASH_DUMP_DECODE_SUPPORT
featureenabled=$?
if [ $featureenabled == 1 ]; then
	#kill already running decode processes and remove the decoded files
	#new decoded files will be created later after renaming the scripts
	pid=`ps x | grep crashdump_decode.sh | grep -v grep | awk '{print $1}'`
	if [ ! -z $pid ]; then
		kill -9 $pid
	fi
	
	if [ -f "/var/crashdump/json/decodeinprogress" ]; then
		pid=`ps x | grep sysDebug_decode.py | grep -v grep | awk '{print $1}'`
		kill -9 $pid
		
		if [ -f "/var/crashdump/json/sysdebug1_decoded.json" ]; then
			rm /var/crashdump/json/sysdebug1_decoded.json
		fi
		if [ -f "/var/crashdump/json/sysdebug2_decoded.json" ]; then
			rm /var/crashdump/json/sysdebug2_decoded.json
		fi
		
		rm /var/crashdump/json/decodeinprogress
	fi
fi

#rename and compress the JSON files, only if a new debug file is available
if [ -f "/var/crashdump/json/sysdebug.json" ]; then
	#sysdebug.json to sysdebug1.json
	#sysdebug1.json to sysdebug2.json
	if [ -f "/var/crashdump/json/sysdebug2.json" ]; then
		rm /var/crashdump/json/sysdebug2.json
	fi
	
	if [ -f "/var/crashdump/json/sysdebug1.json" ]; then
		mv /var/crashdump/json/sysdebug1.json /var/crashdump/json/sysdebug2.json
	fi

	mv /var/crashdump/json/sysdebug.json /var/crashdump/json/sysdebug1.json

	#compress the sysdebug JSON files and put it in crashdump directory
	if [ -f "/var/crashdump/json/sysdebug1.json" ]; then
		tar -czvf $crashdump_dir/sysdebug.1.gz -C /var/crashdump/json sysdebug1.json
	fi
	if [ -f "/var/crashdump/json/sysdebug2.json" ]; then
		tar -czvf $crashdump_dir/sysdebug.2.gz -C /var/crashdump/json sysdebug2.json
	fi
fi

#rename and compress the JSON files for BAFI, only if a new debug file is available
isFeatureEnabled CONFIG_SPX_FEATURE_ACD_BAFI_SUPPORT
featureenabled=$?
if [ $featureenabled == 1 ]; then
	if [ -f "/var/crashdump/json/bafi_decoded2.json" ]; then
		rm /var/crashdump/json/bafi_decoded2.json
	fi

	if [ -f "/var/crashdump/json/bafi_decoded1.json" ]; then
		mv /var/crashdump/json/bafi_decoded1.json /var/crashdump/json/bafi_decoded2.json
	fi
	
	if [ -f "/var/crashdump/json/bafi_decoder.json" ]; then
		mv /var/crashdump/json/bafi_decoder.json /var/crashdump/json/bafi_decoded1.json
	fi

#	#compress the BAFI JSON files and put it in crashdump directory
	if [ -f "/var/crashdump/json/bafi_decoded1.json" ]; then
		tar -czvf $crashdump_dir/bafi_decoded.1.gz -C /var/crashdump/json bafi_decoded1.json
	fi
	
	if [ -f "/var/crashdump/json/bafi_decoded2.json" ]; then
		tar -czvf $crashdump_dir/bafi_decoded.2.gz -C /var/crashdump/json bafi_decoded2.json
	fi
fi


exit 0
