#/bin/sh

HOST_IP=$1
PORT=$2
MOTOR_ON_PIN=$3
COMMENT=$4
CUR_DATE=`date +"%Y%m%d"`
TIMESTAMP=`date +"%d.%m.%Y %H:%M:%S"`
LOG_FILE='Logs/HemroE54CycleTest_${PORT}_${CUR_DATE}.log'
CYCLE_NUM=0
ERROR_CNT=0
TOTAL_ERROR_CNT=0

timestamp()
{
	date +"%d.%m.%Y %H:%M:%S"
}

logFile()
{
	echo Logs/HemroE54CycleTest_${PORT}_$(date +"%Y%m%d").log
}

motorOn()
{
	gpio -g read ${MOTOR_ON_PIN}
}

mkdir -p Logs
touch $(logFile)

echo "$(timestamp);Starting cycle test for port ${PORT} / power switch IP ${HOST_IP}" >> $(logFile)
echo "$(timestamp);Comment ${COMMENT}" >> $(logFile)
echo "$(timestamp);Assign GPIO ${MOTOR_ON_PIN} to raspberry"

gpio -g mode ${MOTOR_ON_PIN} in

for ((;;))
do
	echo "New test cycle ${CYCLE_NUM}, total errors ${TOTAL_ERROR_CNT}"
	echo "$(timestamp);Next cycle ${CYCLE_NUM}, total errors ${TOTAL_ERROR_CNT}" >> $(logFile)
	echo "$(timestamp);Switch motor on" >> $(logFile)
	./do_switch.sh ${HOST_IP} ${PORT} on >> $(logFile)
	sleep 1
	
	if [[ $(motorOn) -eq 1 ]];
	then
		echo "$(timestamp);Error: motor is on before being started" >> $(logFile)
		echo "$(timestamp);Error: motor is on before being started"
		((ERROR_CNT++))
		((TOTAL_ERROR_CNT++))
	else
		sleep 7

		echo $(motorOn)
		
		if [[ $(motorOn) -eq 0 ]];
		then
			echo "$(timestamp);Error: motor is off although started" >> $(logFile)
			echo "$(timestamp);Error: motor is off although started"
			((ERROR_CNT++))
			
			((TOTAL_ERROR_CNT++))
		else
			ERROR_CNT=0

			sleep 7
		fi
	fi
	
	echo "$(timestamp);Switch motor off" >> $(logFile)
	./do_switch.sh ${HOST_IP} ${PORT} off >> $(logFile)

	if [[ ${ERROR_CNT} -ge 3 ]];
	then
		echo "$(timestamp);abort after 3 errors" >> $(logFile)
		echo "$(timestamp);abort after 3 errors"
		exit -1
	fi

	sleep 5
	((CYCLE_NUM++))
done

./do_switch.sh ${HOST_IP} ${PORT} off >> $(logFile)
