#include "sys.h"
#include "sub_method.h"

//排他制御が必要な変数を扱う関数

//mbedとの通信を切断するフラグ
//メインスレッドでも扱うため排他制御を行う
bool mbed_end_flag = false;
int mode = TEST_MODE;
std::mutex mbed_mtx;
std::mutex mode_mtx;
std::mutex calib_mtx;

//データが競合するのを防ぐため排他制御でフラグを取り扱う
//終了フラグを立ち上げる変数
void change_flag() {
	mbed_mtx.lock();
	mbed_end_flag = TRUE;
	mbed_mtx.unlock();
}
//終了フラグの値を返す関数
bool check_flag() {
	bool check;
	mbed_mtx.lock();
	check = mbed_end_flag;
	mbed_mtx.unlock();
	return check;
}
void mode_releace() {
	mode_mtx.lock();
	mode = RELEASE_MODE;
	mode_mtx.unlock();
}
int check_mode() {
	int n;
	mode_mtx.lock();
	n = mode;
	mode_mtx.unlock();
	return n;
}

void change_calib() {

}

bool calib_motor(int ch) {
	bool m;
	if (ch == 0) {
		calib_mtx.lock();
		calib_m = !calib_m;
		calib_mtx.unlock();
	}
	else {
		calib_mtx.lock();
		m = calib_m;
		calib_mtx.unlock();
	}
	return m;
}