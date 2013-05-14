/*
 * main.cpp
 *
 *  Created on: 12/08/2011
 *      Author: Ignacio Mellado
 */

#define ENABLE_VIDEO_CHANNEL

#include <iostream>
#include <atlante.h>
#include <cv.h>
#include <highgui.h>
#include "ConsoleUI.h"
#include "MyDrone.h"

#include <droneproxy.h>

//#define HOST_IP		"10.0.100.3"
#define HOST_IP	"192.168.0.1"
//#define HOST_IP		"127.0.0.1"

using namespace DroneProxy;

int main() {
	MyDrone myDrone;
	ConsoleUI ui(&myDrone);
	
	try {
		myDrone.open(HOST_IP, COMMANDER, 0);
		myDrone.logDataFrom(MyDrone::ODOMETRY | MyDrone::VIDEO | MyDrone::COMMANDS);
#ifdef ENABLE_VIDEO_CHANNEL
		myDrone.getChannelManager().enableVideoChannel(0);
#endif

		ui.init();
		ui.doLoop();
		ui.destroy();

		myDrone.close();
	} catch(cvgException &e) {
		ui.destroy();
		myDrone.close();
		std::cerr << "[Program exception] " << e.getMessage().c_str() << std::endl;
	}

	return 0;
}




