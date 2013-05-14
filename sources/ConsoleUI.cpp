/*
 * ConsoleUI.cpp
 *
 *  Created on: 15/08/2011
 *      Author: Ignacio Mellado
 */

// This is the horizontal speed reference (forward or lateral speed)
#define XY_SPEED			0.5			// [m/s]
// Every time you press 'q' or 'a', they altitude will be incremented or decremented this amount, respectively
#define ALTITUDE_INCREMENT		0.2			// [m]
// Every time you press 'x' or 'z', they yaw will be incremented or decremented this amount, respectively
#define YAW_INCREMENT			18.0			// [deg]

#include <ncurses.h>
#include <ConsoleUI.h>
#include <atlante.h>
#include <math.h>
#include <string.h>
#include <highgui.h>

#define LOGS_TO_FILE

using namespace DroneProxy;
using namespace DroneProxy::Comm;

ConsoleUI::ConsoleUI(MyDrone *drone) {
	this->drone = drone;
	isInit = false;
	drone->addLogConsumer(this);
}

ConsoleUI::~ConsoleUI() {
	destroy();
	drone->removeLogConsumer(this);
}

void ConsoleUI::init() {
	if (consoleMutex.lock()) {
		if (!isInit) {
			isInit = true;

#ifdef LOGS_TO_FILE
			logFile = fopen("log.txt", "w");
			if (logFile == NULL)
				throw cvgException("[ConsoleUI] Cannot open log file");
#endif

			initscr();
			raw();
			keypad(stdscr, true);
			noecho();
			nodelay(stdscr, true);

			// Log window creation
			int row, col;
			getmaxyx(stdscr, row, col);
			logWin = newwin(row / 2, col, row / 2, 0);
			box(logWin, 0, 0);
			scrollok(logWin, true);
			wmove(logWin, 1, 0);
			refresh();
			wrefresh(logWin);

			memset(&lastDroneInfo, 0, sizeof(lastDroneInfo));

		}
		consoleMutex.unlock();
		drone->getChannelManager().getFeedbackChannel().addEventListener(this);
	} else throw cvgException("[ConsoleUI::init] Cannot lock console mutex");

	logConsume(NULL, "User interface ready");
}

void ConsoleUI::destroy() {
	if (consoleMutex.lock()) {
		if (isInit) {

			drone->getChannelManager().getFeedbackChannel().removeEventListener(this);

			// Destroy log window
			wborder(logWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
			wrefresh(logWin);
			delwin(logWin);

			endwin();

			isInit = false;

			printf("[ConsoleUI] User interface closed\n");
#ifdef LOGS_TO_FILE
			fclose(logFile);
#endif
		}
		consoleMutex.unlock();
	}
}

void ConsoleUI::doLoop() {
/*	static cvg_double desiredYaw = 0.0;
	static cvg_double desiredAlt = 1.0;
*/
	FeedbackPacket p;

	bool loop = true;
	while(loop) {
		refreshInfo();
		switch(getch()) {
			case 27:
				loop = false;
				break;
			case '8':
			case KEY_UP:
				drone->setForwardSpeed(XY_SPEED);
				break;
			case '2':
			case KEY_DOWN:
				drone->setForwardSpeed(-XY_SPEED);
				break;
			case '4':
			case KEY_LEFT:
				drone->setLateralSpeed(-XY_SPEED);
				break;
			case '6':
			case KEY_RIGHT:
				drone->setLateralSpeed(XY_SPEED);
				break;
			case '5':
				drone->setForwardSpeed(0.0);
				drone->setLateralSpeed(0.0);
				break;
			case '7':
			case 'z':
				drone->incrementYaw(-YAW_INCREMENT * M_PI / 180.0);
				break;
			case '9':
			case 'x':
				drone->incrementYaw(YAW_INCREMENT * M_PI / 180.0);
				break;
			case 'q':
				drone->incrementAltitude(ALTITUDE_INCREMENT);
				break;
			case 'a':
				drone->incrementAltitude(-ALTITUDE_INCREMENT);
				break;

			// Control modes
			case 'h':
				drone->setControlMode(HOVER);
				break;
			case 'm':
				drone->setControlMode(MOVE);
				break;
			case ' ':
				drone->setControlMode(EMERGENCYSTOP);
				break;
			case 'l':
				drone->setControlMode(LAND);
				break;
			case 't':
				drone->setControlMode(TAKEOFF);
				break;

			// Start or stop video sources
			case '0':
				drone->getChannelManager().enableVideoChannel(0, !drone->getChannelManager().isVideoChannelEnabled(0));
				break;
			case '1':
				drone->getChannelManager().enableVideoChannel(1, !drone->getChannelManager().isVideoChannelEnabled(1));
				break;
		}
		usleep(100000);
	}
}

void ConsoleUI::writeLabel(cvg_uint y, cvg_uint x, const char *label, cvg_int valueLength, cvg_double value, const char *format) {
	char str[valueLength + 1];
	snprintf(str, valueLength + 1, format == NULL ? "%.2f" : format, value);
	mvprintw(y, x, "%s: %s", label, str);
	cvg_int l = strlen(str);
	cvg_int labelLen = strlen(label);
	for (cvg_int i = 0; i < valueLength - l; i++) mvprintw(y, x + labelLen + 2 + l + i, " ");
}

void ConsoleUI::refreshInfo() {
	char fmStr[32];
	flyingModeToString(fmStr, drone->getControlMode());
	char dmStr[64];
	droneModeToString(dmStr, lastDroneInfo.feedbackData.droneMode);
	sprintf(&dmStr[strlen(dmStr)], " (native: 0x%x)", lastDroneInfo.feedbackData.nativeDroneState);
	cvg_bool connToDrone = drone->connToDrone();	// This must be outside the lock-unlock to avoid potential deadlock
	if (consoleMutex.lock()) {
		int y = 0;
		mvprintw(y, 0, "Comm. with proxy:");
		if (drone->connToProxy()) {
			Channel::Statistics cStats = drone->getChannelManager().getControlChannel().getSendStatistics();
			Channel::Statistics fStats = drone->getChannelManager().getFeedbackChannel().getRecvStatistics();
			Channel::Statistics vStats = drone->getChannelManager().getVideoChannel(0).getRecvStatistics();
			mvprintw(y, 18, "OK (c:%.1f|f:%.1f|v:%.1f)     ", cStats.throughput, fStats.throughput, vStats.throughput);
		} else {
			mvprintw(y, 18, "ERROR                         ");
		}
		mvprintw(y++, 50, "Comm. with drone: %s   ", connToDrone ? "OK" : "ERROR");
		y++;
		mvprintw(y, 0, "Control mode: %s      ", fmStr);
		mvprintw(y, 30, "Drone mode: %s      ", connToDrone ? dmStr : "??");
		//mvprintw(y += 2, 0, "Battery: %.2f%%  ", lastDroneInfo.feedbackData.batteryLevel * 100);
		writeLabel(y += 2, 0, "Battery", 7, lastDroneInfo.feedbackData.batteryLevel * 100, "%.2f%%");
		y += 2;
		cvg_int backupY = y;
		// Odometry
		mvprintw(y++, 0, "ODOMETRY");
		writeLabel(y, 0, "x", 7, 0.0);
		writeLabel(y, 11, "y", 7, 0.0);
		writeLabel(y++, 22, "z", 7, -lastDroneInfo.feedbackData.altitude);
		writeLabel(y, 0, "Y", 7, lastDroneInfo.feedbackData.yaw);
		writeLabel(y, 11, "P", 7, lastDroneInfo.feedbackData.pitch);
		writeLabel(y++, 22, "R", 7, lastDroneInfo.feedbackData.roll);
		writeLabel(y, 0, "Vf", 7, lastDroneInfo.feedbackData.speedX);
		writeLabel(y++, 11, "Vl", 7, lastDroneInfo.feedbackData.speedY);
//		mvprintw(y++, 25, "Vyaw: %.2f", lastDroneInfo.feedbackData.speedYaw);

		// TODO: remove this after testing
		//writeLabel(y++, 0, "Alt. cmd.", 7, lastDroneInfo.feedbackData.userData[2]);

		// Vicon
		y = backupY;
		MyDrone::ViconData vd;
		if (drone->getViconData(&vd)) {
			mvprintw(y++, 35, "VICON");
			Vector3 pos = vd.localTransform.getTranslation() * 1e-3;
			writeLabel(y, 35, "x", 7, pos.x);
			writeLabel(y, 46, "y", 7, pos.y);
			writeLabel(y++, 57, "z", 7, pos.z);
			Vector3 euler = vd.localTransform.getRotationMatrix().getEulerAnglesZYX();
			writeLabel(y, 35, "Y", 7, euler.z * 180.0 / M_PI);
			writeLabel(y, 46, "P", 7, euler.y * 180.0 / M_PI);
			writeLabel(y++, 57, "R", 7, euler.x * 180.0 / M_PI);
			writeLabel(y, 35, "Vf", 7, vd.localSpeed.x * 1e-3);
			writeLabel(y, 46, "Vl", 7, vd.localSpeed.y * 1e-3);
//			mvprintw(y++, 25, "Vyaw: %.2f", lastDroneInfo.feedbackData.speedYaw);
		}

		refresh();
		consoleMutex.unlock();
	}
}

void ConsoleUI::logConsume(DroneProxy::Logs::LogProducer *producer, const cvgString &str) {
	if (consoleMutex.lock()) {
		if (!isInit) {
			printf("%s\n", str.c_str());
		} else {
			// Break string if it's wider than the window
			int maxx, maxy;
			getmaxyx(logWin, maxy, maxx);
			maxx = ((maxx - 2) - 1) + 1;	// Substract border
			cvg_int numFrags = (cvg_int)ceil(str.length() / (cvg_double)maxx);
	//		if (consoleMutex.lock()) {
				for (cvg_int i = 0; i < numFrags - 1; i++) {
					wprintw(logWin, (" " + str.subString(i * maxx, maxx) + "\n").c_str());
				}
				wprintw(logWin, (" " + str.subString((numFrags - 1) * maxx) + "\n").c_str());
	//			consoleMutex.unlock();
	//		}
			box(logWin, 0, 0);
			wrefresh(logWin);

#ifdef LOGS_TO_FILE
			fprintf(logFile, "%f %s\n", timer.getElapsedSeconds(), str.c_str());
#endif
		}
		consoleMutex.unlock();
	}
}

void ConsoleUI::gotData(Channel *channel, void *data) {
	if (channel == &drone->getChannelManager().getFeedbackChannel()) {
		if (consoleMutex.lock()) {
			memcpy(&lastDroneInfo.feedbackData, data, sizeof(lastDroneInfo.feedbackData));
			consoleMutex.unlock();
		}
	}
}
