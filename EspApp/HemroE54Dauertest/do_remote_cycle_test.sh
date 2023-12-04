#/bin/sh

if [[ $# -lt 1 ]];
then
	echo "Error: port num as parameter 1 expected"
	exit -3
fi

if [[ $# -gt 1 ]];
then
	echo "Error: too much parameters"
	exit -3
fi

source ./config.sh

PORT=$1

if [[ $PORT -le 0 ]] || [[ $PORT -ge 5 ]];
then
	echo "Error: port num out of range"
	exit -2
fi
  
GPIO_MOTOR_ON=${GRINDER_GPIO_MOTOR_ON[$PORT]}
DESCRIPTION=${GRINDER_DESCRIPTION[$PORT]}
MONITOR_ESP=${GRINDER_MONITOR_ESP[$PORT]}
MONITOR_DEV=${GRINDER_MONITOR_DEV[$PORT]}

TEMP_ALARM_LOG="Logs/TempAlarmTc08.csv"
CYCLE_NUM=0
ERROR_CNT=0
TOTAL_ERROR_CNT=0
MONITOR_PID=0

timestamp()
{
	date +"%d.%m.%Y %H:%M:%S"
}

logFile()
{
	echo Logs/Grinder${PORT}/HemroE54CycleTest_${PORT}_$(date +"%Y%m%d").csv
}

espLogFile()
{
	echo Logs/Grinder${PORT}/EspLogs/HemroE54CycleTest_${PORT}_EspLog_$(date +"%Y%m%d_%H%M%S")_${CYCLE_NUM}.log
}

logEntry()
{
	echo "$(timestamp);${CYCLE_NUM};${TOTAL_ERROR_CNT};$1" >> $(logFile)
}

logError()
{
	logEntry "$1"
	echo "$(timestamp) - $1"
}

motorOn()
{
	sshpass -p ${RASPI_PASSWORD} ssh ${RASPI_USER}@${RASPI_IP} gpio -g read ${GPIO_MOTOR_ON}
}

hasTempAlarm()
{
	cut -d ';' -f $((${PORT} + 1)) < ${TEMP_ALARM_LOG}
}

monitorPid()
{
	ps -au | grep ${MONITOR_DEV} | tr -s ' ' | cut -d ' ' -f 2 | head -n 1
}

mkdir -p Logs/Grinder${PORT}
mkdir -p Logs/Grinder${PORT}/EspLogs
touch $(logFile)

logEntry "Starting test"
logEntry "Port '${PORT}'"
logEntry "Power switch IP '${ETH008_IP}'"
logEntry "Raspi IP '${RASPI_IP}'"
logEntry "Description '${DESCRIPTION}'"
logEntry "GPIO motor-on '${GPIO_MOTOR_ON}'"
logEntry "Monitor ESP32 '${MONITOR_ESP}'"

logEntry "Switch motor off"

for ((;;))
do
	echo "New test cycle ${CYCLE_NUM}, total errors ${TOTAL_ERROR_CNT}"
	logEntry "Next cycle"
	
	ping -W 2 -q -c 1 ${ETH008_IP} > /dev/null
	if [[ $? -ne 0 ]];
	then
		((ERROR_CNT++))
		((TOTAL_ERROR_CNT++))
		logError "Error: power switch server not available"
		break
	fi

	if [[ $(hasTempAlarm) -eq 1 ]];
	then
		((ERROR_CNT++))
		((TOTAL_ERROR_CNT++))
		logError "Error: temperature alarm raised"
	fi

	# close and open monitor file for ESP32
	if [ "${MONITOR_ESP}" = "y" ];
	then
		if [[ $(monitorPid) -gt 0 ]];
		then
			kill $(monitorPid) > /dev/null
			sync
		fi
		
		ESP_LOG_FILE=$(espLogFile)
		logEntry "ESP monitor log is ${ESP_LOG_FILE}"
		touch ${ESP_LOG_FILE}
		stty -F ${MONITOR_DEV} 115200
		if [[ $? -ne 0 ]];
		then
			((ERROR_CNT++))
			((TOTAL_ERROR_CNT++))
			logError "Error: cannot access dev file '${MONITOR_DEV}' for ESP monitor"
			break
		fi
		cat ${MONITOR_DEV} >> ${ESP_LOG_FILE} &
	fi
	
	# we start with the grinder-off sequence if the condition of power switch is unsafe
	logEntry "Switch off grinder"
	./do_switch.sh ${ETH008_IP} ${PORT} off
	if [[ $? -ne 0 ]];
	then
		((ERROR_CNT++))
		((TOTAL_ERROR_CNT++))
		logError "Error: switch off grinder failed"
		break
	fi

	if [[ ${ERROR_CNT} -ge 3 ]];
	then
		logError "abort test after 3 errors"
		break
	fi

	# 5sec pause
	sleep 5

	logEntry "Switch on grinder"
	./do_switch.sh ${ETH008_IP} ${PORT} on
	if [[ $? -ne 0 ]];
	then
		((ERROR_CNT++))
		((TOTAL_ERROR_CNT++))
		logError "Error: switch on grinder failed"
		break
	fi
	sleep 1

	if [ "$GPIO_MOTOR_ON" = "" ];
	then
		sleep 14
	else
		ping -W 2 -q -c 1 ${RASPI_IP} > /dev/null
		if [[ $? -ne 0 ]];
		then
			logError "Error: raspberry server not available"
			break
		fi

		if [[ $(motorOn) -eq 1 ]];
		then
			((ERROR_CNT++))
			((TOTAL_ERROR_CNT++))
			logError "Error: motor is on before being started"
		else
			sleep 7

			if [[ $(motorOn) -eq 0 ]];
			then
				((ERROR_CNT++))
				((TOTAL_ERROR_CNT++))
				logError "Error: motor is off although started"
			else
				ERROR_CNT=0

				sleep 7
			fi
		fi
	fi
	
	((CYCLE_NUM++))
done

echo "Stop test - switch off grinder finally"

./do_switch.sh ${ETH008_IP} ${PORT} off

if [[ $(monitorPid) -gt 0 ]];
then
	kill $(monitorPid)
fi
