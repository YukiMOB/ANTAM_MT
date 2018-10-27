#include "sys.h"
#include "sub_method.h"

char mouse_filename[64] = "";
char main_filename[64] = "";
char video_filename[64];
void set_fname() {
	struct tm *t_st;
	time_t longtime;
	longtime = time(NULL);
	t_st = localtime(&longtime);
	std::cout << t_st->tm_mon + 1 << "," << t_st->tm_mday << "," << t_st->tm_hour << "," << t_st->tm_min << "," << t_st->tm_sec << std::endl;
	sprintf(mouse_filename, "mouse_%d_%d_%d_%d.csv", t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min);
	sprintf(main_filename, "main_%d_%d_%d_%d.csv", t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min);
	sprintf(video_filename, "video_%d_%d_%d_%d.avi", t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min);
	std::cout << mouse_filename << std::endl;
	std::cout << main_filename << std::endl;
}