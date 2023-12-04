#include <iostream>
#include <ctime>
#include <cstring>
#include <libusbtc08/usbtc08.h>
#include <unistd.h>
#include <locale>
#include <fstream>

void appendLog( const char * pszEntry )
{
	std::time_t t = std::time(nullptr);
	char szFileName[1000];
	std::strftime( szFileName, sizeof( szFileName ), "TempLogTc08-%Y%m%d.csv", std::localtime(&t) ); 

	std::ofstream outfile( szFileName, std::ofstream::out | std::ofstream::app );

	outfile << pszEntry;

	outfile.close();
}

void writeAlarm( const char * pszEntry )
{
	std::time_t t = std::time(nullptr);
	char szFileName[1000];
	std::strftime( szFileName, sizeof( szFileName ), "TempAlarmTc08.csv", std::localtime(&t) ); 

	std::ofstream outfile( szFileName, std::ofstream::out );

	outfile << pszEntry;

	outfile.close();
}

int main( int argc,
			char ** argv )
{
	int nRetVal = 0;
	int32_t i32RetVal = usb_tc08_open_unit_async();
	
	/* Make sure no errors occurred opening the unit */
	if ( !i32RetVal ) 
	{
		std::cerr << "Error opening unit. Exiting." << std::endl;
		nRetVal = -1;
	}

	else
	{
		int16_t i16Handle = 0;
		
		std::cout << "Open progress USB TC-08: ";
		
		/* Display a text "progress bar" while waiting for the unit to open */
		while ( ( i32RetVal = usb_tc08_open_unit_progress( &i16Handle, NULL ) ) == USBTC08_PROGRESS_PENDING)
		{
			/* Update our "progress bar" */
			std::cout << "|" << std::flush;
			usleep( 200000 );
		}
		std::cout << std::endl;
		
		if ( i16Handle != USBTC08_PROGRESS_COMPLETE || i16Handle <= 0 ) 
		{
			std::cerr << "No USB TC-08 units could be opened. Exiting." << std::endl;
			i32RetVal = -1;
		} 
		else 
		{
			std::cout << "USB TC-08 opened successfully." << std::endl;

			/* collect unit information */
			USBTC08_INFO unitInfo;
			unitInfo.size = sizeof( unitInfo );
			usb_tc08_get_unit_info( i16Handle, &unitInfo );
			
			std::cout << "Unit information:" << std::endl;
			std::cout << "Driver: " << unitInfo.DriverVersion << std::endl
			<< "Serial: " << unitInfo.szSerial << std::endl
			<< "Cal date: " << unitInfo.szCalDate << std::endl;

			/* Set up all channels */
			i16Handle = usb_tc08_set_channel( i16Handle, 0, 'C' );

			int32_t nChannel = 0;
			for ( nChannel = 0; nChannel < USBTC08_MAX_CHANNELS; nChannel++)
			{
				i16Handle &= usb_tc08_set_channel( i16Handle, nChannel + 1, 'K' );
			}
			
			if ( !i16Handle )
			{
				std::cerr << "Error setting up channels. Exiting." << std::endl;
			} 
			else 
			{
				std::cout << "Enabled all channels, selected Type K thermocouple." << std::endl;
				
				usb_tc08_run( i16Handle, 1000 );

				usleep( 2000000 );
				
				while ( true )
				{
					std::time_t t = std::time(nullptr);
					char szLogLine[1000];
					char szAlarmLine[1000];
					std::strftime( szLogLine, sizeof( szLogLine ), "%d.%m.%Y %H:%M:%S", std::localtime(&t) ); 
					std::strftime( szAlarmLine, sizeof( szAlarmLine ), "%d.%m.%Y %H:%M:%S", std::localtime(&t) ); 
					
					for ( nChannel = 0; nChannel < USBTC08_MAX_CHANNELS; nChannel++)
					{
						float   fTemp;
						int32_t i32Time;
						int16_t i16Overflow;
						int32_t nError = usb_tc08_get_temp( i16Handle,
															&fTemp, 
															&i32Time,
															1,
															&i16Overflow,
															nChannel + 1, 
															USBTC08_UNITS_CENTIGRADE,
															0 );
						sprintf( std::strchr( szLogLine, 0 ), ";%.2f", fTemp );
						sprintf( std::strchr( szAlarmLine, 0 ), ";%d", ( fTemp > 100.0 ) ? 1 : 0 );
					}
					sprintf( std::strchr( szLogLine, 0 ), "\n" );
					sprintf( std::strchr( szAlarmLine, 0 ), "\n" );
					std::cout << szLogLine;
					appendLog( szLogLine );
					writeAlarm( szAlarmLine );
					
					usleep( 1000000 );
				}
			}
		}
		
		usb_tc08_close_unit( i16Handle );
	}
	
	return nRetVal;
}
