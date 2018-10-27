////////////////////////////////////////////////////////////////////////////////////////
/////////////////sys.hを開いてファイル名やポート番号の設定を行ってください//////////////
//プログラムを開始したら、スライドバーで閾値の設定、mbed,dynamixel,カメラが
//接続されているか確認してください
//プログラム開始時はTESTモード（録画はしていない）状態
//録画した動画の再生時間はおよそ５分に１秒ほど早めにずれます
//使用方法
//背景画像を更新：’Sキー’を押してください
//計測を開始：’Rキー’を押してください
//TESTモード（計測をいったん中止、タイマーリセット）：’Tキー’を押して下さい
//上記以外のキーを押すとプログラムが終了
/////////////////////////////////////////////////////////////////////////////////////////

#include "sys.h"
#include "sub_method.h"

int main() {
	set_fname();
	//////カメラの準備///////
	cv::VideoCapture cap;
	cap.open(CAP_NUM);
	if (!cap.isOpened())
		return(1);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CAM_W);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAM_H);
	cap.set(CV_CAP_PROP_FPS, FPS);

	//dynamixelとの通信準備
	init_Device();

	int key, first = 0;
	int r = 0, g = 255, b = 0;
	bool calib = false;

	//円形切り抜き画像
	//cv::circle(circleFrame, cv::Point(CAM_W / 2, CAM_H / 2), CAM_W / 2 - CAM_W/12, 255, -1);
	cv::circle(circleFrame, cv::Point(CAM_W / 2, CAM_H / 2), CAM_W / 2 - CAM_W / 12 -40, 255, -1);
	cv::bitwise_not(circleFrame, circleFrame);

	cv::VideoWriter writer;
	//cv::VideoWriter writer(VIDEO_NAME, -1, FPS, cv::Size(CAM_W, CAM_H));
	std::ofstream ofs;
	CvFont dfont;
	char message[64] = "";
	//フォントの初期化
	cvInitFont(&dfont, CV_FONT_HERSHEY_SIMPLEX, 1.0f, 1.0f, 0.0f, 2, CV_AA);
	//画像処理用の変数
	cv::Mat frame,
		img(cv::Size(CAM_W, CAM_H), CV_8UC3),
		back(cv::Size(CAM_W, CAM_H), CV_8UC1),
		dst(cv::Size(CAM_W, CAM_H), CV_8UC1);
	//時間計測変数
	int64 t0, t1, t2, t3, t4, t5;
	double end_t = 0;
	int64 time_log;

	cv::namedWindow(WIN_NAME, CV_WINDOW_AUTOSIZE);
	cv::createTrackbar("thresh", WIN_NAME, &THRE, 255, NULL);
	std::thread mbed(serial_task);//mbedからの通信受付スレッド
	
	//処理の開始
	cap.read(frame);
	int64 start_t = cv::getTickCount();
	while (1) {
		if (check_mode() == RELEASE_MODE && first == 0) {
			std::cout << "REC STRT" << std::endl;
			b = 0;
			g = 0;
			r = 255;
			start_t = cv::getTickCount();
			first = 1;
		}
		t0 = cv::getTickCount();
		time_log = (int64)((t0 - start_t) * 1000 / cv::getTickFrequency());
		//カメラから画像を取得
		if (cap.grab() == false)
			continue;
		if (cap.retrieve(frame, 0) == false)
			continue;
		t1 = cv::getTickCount();
		frame.copyTo(img);
		sprintf(message, "time: %I64d h %I64d m %I64d s   %2.2lf fps", (time_log / 1000) / 3600, ((time_log / 1000) / 60) % 60, (time_log / 1000) % 60, (double)(1000 / end_t));
		putText(frame, message, cv::Point(10, 20), cv::FONT_HERSHEY_PLAIN, 1.0, cvScalar(b, g, r), 2, CV_AA);
		t2 = cv::getTickCount();
		//現在のフレームを書き出す
		if (check_mode() == RELEASE_MODE)
			writer << frame;
		t3 = cv::getTickCount();
		//二値化画像の作成と重心の取得
		img_processing_main(&img, &back, &dst);
		t4 = cv::getTickCount();
		//画像処理が終わったら別スレッドでモータ制御開始
		//calib_mがtrueならモータを動かさない
		std::thread m_th(motor_task);
		//二値化画像の出力
		cv::imshow(WIN_NAME, dst);
		//calibration
		if (calib) {
			cv::line(frame, cv::Point(0, CAM_H / 2), cv::Point(CAM_W, CAM_H / 2), cv::Scalar(0, 0, 255), 1, CV_AA);
			cv::line(frame, cv::Point(CAM_W / 2, 0), cv::Point(CAM_W / 2, CAM_H), cv::Scalar(0, 0, 255), 1, CV_AA);
			putText(frame, "Calibration Mode", cv::Point(10, 40), cv::FONT_HERSHEY_PLAIN, 1.0, cvScalar(255, 0, 0), 2, CV_AA);
			if(calib_motor(MOTOR_STATE))
				putText(frame, "Motor OFF", cv::Point(10, 60), cv::FONT_HERSHEY_PLAIN, 1.0, cvScalar(255, 0, 0), 2, CV_AA);
			else
				putText(frame, "Motor ON", cv::Point(10, 60), cv::FONT_HERSHEY_PLAIN, 1.0, cvScalar(255, 0, 0), 2, CV_AA);

		}
		//結果出力
		cv::imshow("frame", frame);
		key = cv::waitKey(1);
		//sキーを押されたら背景画像を更新する
		if (key == 0x072) {
			//rキーが押されたら録画モード
			writer = cv::VideoWriter(video_filename, CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(CAM_W, CAM_H));
			//cv::VideoWriter writer(VIDEO_NAME, -1, FPS, cv::Size(CAM_W, CAM_H));
			ofs = std::ofstream(main_filename);
			ofs << "time_log" << "," << "キャプチャ時間" << "," << "書き出し処理時間" << "," << "画像処理時間" << ","
			<< "モータ制御開始から終了" << "," << "1ループ" << std::endl;
			start_time = timeGetTime();
			mode_releace();
		}
		else if (key == 0x63){
			//cキーを押されたらキャリブレーションモードの切り替え
				if(calib)
					//キャリブレーションモード終了時にモータの動作がOFFならONに切り替え
					if (calib_motor(MOTOR_STATE)) {
						calib_motor(MOTOR_SWICH);
					}
				calib = !calib;
		}
		else if (key == 0x6d) {
			//キャリブレーションモード中にmキーが押されたらモータの動作ON・OFF切り替え
			if (calib)
				calib_motor(MOTOR_SWICH);
		}
		else if (key == 0x1b) {
			//escキーを押されたらループを抜ける
			m_th.join();
			break;
		}

		//モータ制御スレッドの終了を待つ
		m_th.join();
		t5 = cv::getTickCount();
		if (check_mode() == RELEASE_MODE && first != 0) {
			ofs << time_log << ","
				<< (double)((t1 - t0) * 1000 / cv::getTickFrequency()) << ","
				<< (double)((t3 - t2) * 1000 / cv::getTickFrequency()) << ","
				<< (double)((t4 - t3) * 1000 / cv::getTickFrequency()) << ","
				<< (double)((t5 - t4) * 1000 / cv::getTickFrequency()) << ","
				<< end_t << std::endl;
		}
		//debug
		if (timer_flag) {
			//時間を越えたら終了
			if (time_log > (REC_HOUR * 3600 + REC_MINUTE * 60) * 1000)
				break;
		}
		end_t = (double)((cv::getTickCount() - t0) * 1000 / cv::getTickFrequency());
	}
	end_device_rx28();
	change_flag();
	mbed.join();
	//終了処理
	ofs.close();
	writer.release();
	dst.release();
	back.release();
	frame.release();
	cap.release();
	img.release();
	cv::destroyAllWindows();
	std::cout << "すべのてプロセスの終了" << std::endl;
	return 0;
}