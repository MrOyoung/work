#ifndef _MODULE_DEF_H_
#define _MODULE_DEF_H_


/*
MODULE_IPC_UART_DATA = 0x0001,
*/
#define MODULE_IPC_UART_DATA_NAME 		"com.saic.IpcUartData"
#define MODULE_IPC_UART_DATA_PATH 		"/com/saic/IpcUartData"
#define MODULE_IPC_UART_DATA_INTERFACE 	"IpcUartData.signal.type"

/*
MODULE_SERVICE_DI_DATA_ANALISIS = 0x0101,
*/
#define MODULE_SERVICE_DI_DATA_ANALISIS_NAME 		"com.saic.ServiceDiData"
#define MODULE_SERVICE_DI_DATA_ANALISIS_PATH 		"/com/saic/ServiceDiData"
#define MODULE_SERVICE_DI_DATA_ANALISIS_INTERFACE 	"ServiceDiData.signal.type"

/*
MODULE_SERVICE_SYSTEM,
*/
#define MODULE_SERVICE_SYSTEM_NAME 		"com.saic.ServiceSystem"
#define MODULE_SERVICE_SYSTEM_PATH 		"/com/saic/ServiceSystem"
#define MODULE_SERVICE_SYSTEM_INTERFACE "ServiceSystem.signal.type"

/*
MODULE_SERVICE_POWER,
*/
#define MODULE_SERVICE_POWER_NAME 		"com.saic.ServicePower"
#define MODULE_SERVICE_POWER_PATH 		"/com/saic/ServicePower"
#define MODULE_SERVICE_POWER_INTERFACE 	"ServicePower.signal.type"

/*
MODULE_SERVICE_CAMERA
*/
#define MODULE_SERVICE_CAMERA_NAME 		"com.saic.ServiceCamera"
#define MODULE_SERVICE_CAMERA_PATH		"/com/saic/ServiceCamera"
#define MODULE_SERVICE_CAMERA_INTERFACE "ServiceCamera.signal.type"

/*
MODULE_SERVICE_LOG
*/
#define MODULE_SERVICE_LOG_NAME			"com.saic.ServiceLog"
#define MODULE_SERVICE_LOG_PATH			"/com/saic/ServiceLog"
#define MODULE_SERVICE_LOG_INTERFACE	"ServiceLog.signal.type"

/*
MODULE_SERVICE_WINDOWS_MGR
*/
#define MODULE_SERVICE_WINDOWS_MGR_NAME 		"com.saic.ServiceWindowsMgr"
#define MODULE_SERVICE_WINDOWS_MGR_PATH 		"/com/saic/ServiceWindowsMgr"
#define MODULE_SERVICE_WINDOWS_MGR_INTERFACE 	"ServiceWindowsMgr.signal.type"

/*
MODULE_SERVICE_UPGRADE
*/
#define MODULE_SERVICE_UPGRADE_NAME				"com.saic.ServiceUpgrade"
#define MODULE_SERVICE_UPGRADE_PATH				"/com/saic/ServiceUpgrade"
#define MODULE_SERVICE_UPGRADE_INTERFACE		"ServiceUpgrade.signal.type"

/*
MODULE_SERVICE_AUDIO_MGR
*/
#define MODULE_SERVICE_AUDIO_MGR_NAME			"com.saic.ServiceAudioMgr"
#define MODULE_SERVICE_AUDIO_MGR_PATH			"/com/saic/ServiceAudioMgr"
#define MODULE_SERVICE_AUDIO_MGR_INTERFACE		"ServiceAudioMgr.signal.type"

/*
MODULE_APP_CLUSTER
*/
#define MODULE_APP_CLUSTER_NAME			"com.saic.AppCluster"
#define MODULE_APP_CLUSTER_PATH			"/com/saic/AppCluster"
#define MODULE_APP_CLUSTER_INTERFACE	"AppCluster.signal.type"


/*
Camera app
*/
#define MODULE_CAMERA_NAME 		"com.saic.Camera"
#define MODULE_CAMERA_PATH		"/com/saic/Camera"
#define MODULE_CAMERA_INTERFACE "Camera.signal.type"

/*dial speed*/
#define MODULE_APP_CLUSTER_DIAL_SPEED_NAME			"com.saic.AppClusterDailSpeed"
#define MODULE_APP_CLUSTER_DIAL_SPEED_PATH			"/com/saic/AppClusterDailSpeed"
#define MODULE_APP_CLUSTER_DIAL_SPEED_INTERFACE	    "AppClusterDailSpeed.signal.type"

/*dial tach*/
#define MODULE_APP_CLUSTER_DIAL_ROTATIONAL_SPEED_NAME			"com.saic.KanziAppTach"
#define MODULE_APP_CLUSTER_DIAL_ROTATIONAL_SPEED_PATH			"/com/saic/KanziAppTach"
#define MODULE_APP_CLUSTER_DIAL_ROTATIONAL_SPEED_INTERFACE	     "KanziAppTach.signal.type"

/*popup*/
#define MODULE_APP_CLUSTER_POPUP_NAME			"com.saic.AppClusterPopup"
#define MODULE_APP_CLUSTER_POPUP_PATH			"/com/saic/AppClusterPopup"
#define MODULE_APP_CLUSTER_POPUP_INTERFACE	    "AppClusterPopup.signal.type"

/*background*/
#define MODULE_APP_CLUSTER_BACKGROUND_NAME			"com.saic.AppClusterBackground"
#define MODULE_APP_CLUSTER_BACKGROUND_PATH			"/com/saic/AppClusterBackground"
#define MODULE_APP_CLUSTER_BACKGROUND_INTERFACE		"AppClusterBackground.signal.type"

/*trip computer*/
#define MODULE_APP_CLUSTER_TRIP_COMPUTER_NAME			"com.saic.AppClusterTripComputer"
#define MODULE_APP_CLUSTER_TRIP_COMPUTER_PATH			"/com/saic/AppClusterTripComputer"
#define MODULE_APP_CLUSTER_TRIP_COMPUTER_INTERFACE		"AppClusterTripComputer.signal.type"




#define _IPC_NAME(name)					"com.saic." name
#define _IPC_INTERFACE(interface)		interface ".signal.type"
#define _IPC_PATH(path)					"/com/saic/" path


/*kanzi applications*/
/*KANZIAPP_SPEED*/
#define KANZIAPP_SPEED_NICK					"KanziAppSpeed"
#define KANZIAPP_SPEED_NAME					_IPC_NAME(KANZIAPP_SPEED_NICK)
#define KANZIAPP_SPEED_INTERFACE				_IPC_INTERFACE(KANZIAPP_SPEED_NICK)
#define KANZIAPP_SPEED_PATH					_IPC_PATH(KANZIAPP_SPEED_NICK)

/*KANZIAPP_TACH*/
#define KANZIAPP_TACH_NICK					"KanziAppTach"
#define KANZIAPP_TACH_NAME					_IPC_NAME(KANZIAPP_TACH_NICK)
#define KANZIAPP_TACH_INTERFACE				_IPC_INTERFACE(KANZIAPP_TACH_NICK)
#define KANZIAPP_TACH_PATH					_IPC_PATH(KANZIAPP_TACH_NICK)

/*KANZIAPP_POPUP*/
#define KANZIAPP_POPUP_NICK					"KanziAppPopup"
#define KANZIAPP_POPUP_NAME					_IPC_NAME(KANZIAPP_POPUP_NICK)
#define KANZIAPP_POPUP_INTERFACE				_IPC_INTERFACE(KANZIAPP_POPUP_NICK)
#define KANZIAPP_POPUP_PATH					_IPC_PATH(KANZIAPP_POPUP_NICK)

/*KANZIAPP_TABICON*/
#define KANZIAPP_TABICON_NICK					"KanziAppTabicon"
#define KANZIAPP_TABICON_NAME					_IPC_NAME(KANZIAPP_TABICON_NICK)
#define KANZIAPP_TABICON_INTERFACE				_IPC_INTERFACE(KANZIAPP_TABICON_NICK)
#define KANZIAPP_TABICON_PATH					_IPC_PATH(KANZIAPP_TABICON_NICK)

/*KANZIAPP_BACKGROUND*/
#define KANZIAPP_BACKGROUND_NICK					"KanziAppBackground"
#define KANZIAPP_BACKGROUND_NAME					_IPC_NAME(KANZIAPP_BACKGROUND_NICK)
#define KANZIAPP_BACKGROUND_INTERFACE				_IPC_INTERFACE(KANZIAPP_BACKGROUND_NICK)
#define KANZIAPP_BACKGROUND_PATH					_IPC_PATH(KANZIAPP_BACKGROUND_NICK)



#endif
