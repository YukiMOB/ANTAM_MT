#include "sys.h"
#include "sub_method.h"
int pos_x = CAM_W / 2, pos_y = CAM_H / 2;
int THRE = 140;
double length = 0, radian = 0;
//矩形画像領域切り抜き用コピー画像領域生成
//cv::Mat circleFrame = cv::Mat::zeros(cv::Size(CAM_W, CAM_H), CV_8UC1);
cv::Mat circleFrame = cv::Mat::zeros(cv::Size(CAM_W, CAM_H), CV_8UC1);
const int opening_size = 1;
cv::Mat element = cv::getStructuringElement(cv::MORPH_OPEN,
	cv::Size(2 * opening_size + 1, 2 * opening_size + 1),
	cv::Point(opening_size, opening_size));

//重心を求める関数
void moment_task(cv::Mat *dst, cv::Moments mom) {
	double dotnum_dbl, x_cm_dbl, y_cm_dbl;
	int dotnum;
	dotnum_dbl = mom.m00;
	dotnum = (int)dotnum_dbl;
	//重心が存在するなら
	if (dotnum != 0) {
		x_cm_dbl = mom.m10 / dotnum_dbl;//重心のx座標
		y_cm_dbl = mom.m01 / dotnum_dbl;//重心のｙ座標
	}
	else {
		x_cm_dbl = CAM_W / 2;
		y_cm_dbl = CAM_H / 2;
	}
	length = sqrt((CAM_W / 2 - x_cm_dbl)*(CAM_W / 2 - x_cm_dbl) + (CAM_H / 2 - y_cm_dbl)*(CAM_H / 2 - y_cm_dbl));
	radian = fabs(atan((x_cm_dbl - CAM_H / 2) / (y_cm_dbl - CAM_H / 2)));
	pos_x = (int)x_cm_dbl;
	pos_y = (int)y_cm_dbl;
	//cv::circle(*dst, cv::Point(pos_x, pos_y), 5, cv::Scalar(100, 150, 100), 1, 5);
	//std::cout <<"state"<<state<< "length" << target_length << "radian" << target_radian << std::endl;
}
//メイン処理関数
void img_processing_main(cv::Mat *src, cv::Mat *back, cv::Mat *dst) {
	cv::Mat gray(cv::Size(CAM_W, CAM_H), CV_8UC1), diff(cv::Size(CAM_W, CAM_H), CV_8UC1);
	//背景差分法で二値化画像の作成
	cv::cvtColor(*src, gray, cv::COLOR_RGB2GRAY);
	cv::bitwise_or(gray, circleFrame, gray);
	cv::threshold(gray, *dst, THRE, 255, cv::THRESH_BINARY_INV);
	cv::morphologyEx(*dst, *dst, cv::MORPH_OPEN, element);
	//二値化画像の重心を求める
	cv::Moments mom = cv::moments(*dst);
	moment_task(dst, mom);
	gray.release();
}

