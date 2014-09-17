/*
   Implementation of ls -lah using only system calls.
   Author - Sambuddha Basu
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define ANSI_COLOR_BLUE		"\x1b[34m"
#define ANSI_COLOR_GREEN	"\x1b[32m"
#define BUF_SIZE		1024

struct linux_dirent {
	unsigned long  d_ino;     /* Inode number */
	unsigned long  d_off;     /* Offset to next linux_dirent */
	unsigned short d_reclen;  /* Length of this linux_dirent */
	char           d_name[];  /* Filename (null-terminated) */
};

void missing_path();
void invalid_path();
void read_link_error();
int getlen(int len);
void human_size(int size, int is_h, int space);
void open_etc_passwd(int id, int is_u, int is_g);
long long int seconds_in_year(long long int year);
long long int is_leap(long long int year);
long long int seconds_in_year(long long int year);
long long int timestamp_to_year(long long int *timestamp);
long long int timestamp_to_month(long long int *timestamp, long long int year);
void print_month(int month);

// missing_path() gets called when no path is specified from the command line.
void missing_path() {
	write(2, ANSI_COLOR_RED "No path specified.\n" ANSI_COLOR_RESET, 28);
	exit(1);
}

// invalid_path() gets called when a invalid path is specified from the command line.
void invalid_path() {
	write(2, ANSI_COLOR_RED "Error finding path.\n" ANSI_COLOR_RESET, 29);
	exit(1);
}

// read_link_error() gets called when there is error in reading a symbolic link.
void read_link_error() {
	write(2, ANSI_COLOR_RED "Cannot read symbolic link.\n" ANSI_COLOR_RESET, 36);
	exit(1);
}

// getlen() is used for getting the length of the integer.
int getlen(int len) {
	int i;
	for(i = 0; len >= 1; i++)
		len /= 10;
	return i;
}

// human_size() is used to convert an integer to string and then print it using write().
void human_size(int size, int is_h, int space) {
	int i = 0, file_size, j;
	char hs[2], file_size_str[BUF_SIZE];
	if(is_h) {
		while(size >= 1024) {
			i++;
			size /= 1024;
		}
	}
	file_size = size;
	if(i == 1)
		hs[0] = 'K';
	else if(i == 2)
		hs[0] = 'M';
	else if(i == 3)
		hs[0] = 'G';
	else
		hs[0] = ' ';
	if(file_size == 0) {
		file_size_str[0] = '0';
		for(i = 1; i < BUF_SIZE; i++)
			file_size_str[i] = '\0';
		write(1, file_size_str, 2);
		if(space)
			write(1, " ", 1);
	}
	else {
		file_size_str[getlen(file_size)] = '\0';
		j = getlen(file_size) - 1;
		for(i = 0; file_size >= 1; i++) {
			file_size_str[j--] = file_size % 10 + '0';
			file_size /= 10;
		}
		write(1, file_size_str, i);
		if(is_h)
			write(1, hs, 2);
		if(space)
			write(1, " ", 1);
	}
}

// open_etc_passwd() is used to print username for corressponding uid or gid.
void open_etc_passwd(int id, int is_u, int is_g) {
	int fd, nread, len, j, i, found;
	char username[BUF_SIZE], uid[BUF_SIZE], gid[BUF_SIZE], buf[BUF_SIZE], ch, given_id[BUF_SIZE];
	if(id == 0) {
		given_id[0] = '0';
		for(i = 1; i < BUF_SIZE; i++)
			given_id[i] = '\0';
	}
	else {
		j = getlen(id);
		given_id[j] = '\0';
		j--;
		for(len = 0; id >= 1; len++) {
			given_id[j--] = id % 10 + '0';
			id /= 10;
		}
	}
	fd = open("/etc/passwd", O_RDONLY);
	if(fd == -1) {
		write(2, ANSI_COLOR_RED "There was an error in opening /etc/passwd\n" ANSI_COLOR_RESET, 53);
		exit(1);
	}
	while(nread = read(fd, buf, 1) > 0) {
		i = 0;
		ch = buf[0];
		while(ch != ':') {
			username[i++] = ch;
			nread = read(fd, buf, 1);
			ch = buf[0];
		}
		username[i] = '\0';
		nread = read(fd, buf, 1);
		ch = buf[0];
		while(ch != ':') {
			nread = read(fd, buf, 1);
			ch = buf[0];
		}
		nread = read(fd, buf, 1);
		ch = buf[0];
		i = 0;
		while(ch != ':') {
			uid[i++] = ch;
			nread = read(fd, buf, 1);
			ch = buf[0];
		}
		uid[i] = '\0';
		nread = read(fd, buf, 1);
		ch = buf[0];
		i = 0;
		while(ch != ':') {
			gid[i++] = ch;
			nread = read(fd, buf, 1);
			ch = buf[0];
		}
		gid[i] = '\0';
		nread = read(fd, buf, 1);
		ch = buf[0];
		while(ch != '\n') {
			nread = read(fd, buf, 1);
			ch = buf[0];
		}
		if(is_u) {
			found = 1;
			for(i = 0, j = 0; given_id[i] != '\0' || uid[j] != '\0'; i++, j++) {
				if(given_id[i] != uid[j]) {
					found = 0;
					break;
				}
			}
			if(found && given_id[i] == '\0' && uid[j] == '\0') {
				for(len = 0; username[len] != '\0'; len++);
				write(1, username, len);
				write(1, " ", 1);
				return;
			}
		}
		if(is_g) {
			found = 1;
			for(i = 0, j = 0; given_id[i] != '\0' || gid[j] != '\0'; i++, j++) {
				if(given_id[i] != gid[j]) {
					found = 0;
					break;
				}
			}
			if(found && given_id[i] == '\0' && gid[j] == '\0') {
				for(len = 0; username[len] != '\0'; len++);
				write(1, username, len);
				write(1, " ", 1);
				return;
			}
		}
	}
}

long long int seconds_in_year(long long int year) {
	long long int leap_days = 366, non_leap_days = 365, mul = 60 * 60 * 24;
	if(is_leap(year))
		return leap_days * mul;
	return non_leap_days * mul;
}

long long int is_leap(long long int year) {
	if(year % 400 == 0)
		return 1;
	if((year % 4 == 0) && (year % 100 != 0))
		return 1;
	return 0;
}

long long int seconds_in_month(long long int month, long long int year) {
	long long int mul = 60 * 60 * 24, days;
	if((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 10) || (month == 12))
		days = 31;
	else if((month == 4) || (month == 6) || (month == 9) || (month == 11))
		days = 30;
	else if(month == 2) {
		if(is_leap(year))
			days = 29;
		else
			days = 28;
	}
	return days * mul;
}

long long int timestamp_to_year(long long int *timestamp) {
	long long int year;
	for(year = 1970; (*timestamp) > seconds_in_year(year); year++)
		(*timestamp) -= seconds_in_year(year);
	return year;
}

long long int timestamp_to_month(long long int *timestamp, long long int year) {
	long long int month;
	for(month = 1; (*timestamp) > seconds_in_month(month, year); month++)
		(*timestamp) -= seconds_in_month(month, year);
	return month;
}

void print_month(int month) {
	char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	write(1, months[month - 1], 3);
	write(1, " ", 1);
}

int main(int argc, char *argv[]) {
	int dir_content, cnt, status = 0, len, is_a = 0, is_l = 0, is_h = 0, is_arg = 0, i, j, path_file = 0, path_dir = 0, file_stat_possible, path_len, read_link;
	long long int curr_time, time_m, time_h, curr_time_copy, year;
	struct linux_dirent *d;
	struct stat file_stat;
	char buf[BUF_SIZE], d_type, *directory, *argument, full_path[BUF_SIZE], link_target[BUF_SIZE];
	if(argc == 1) {
		missing_path();
	}
	else if(argc == 2) {
		if(argv[1][0] == '-') {
			missing_path();
		}
		else {
			directory = argv[1];
			argument = "";
		}
	}
	else if(argc == 3) {
		is_arg = 1;
		if(argv[1][0] == '-') {
			argument = argv[1];
			directory = argv[2];
		}
		else {
			argument = argv[2];
			directory = argv[1];
		}
	}
	if(is_arg) {
		for(i = 0; argument[i] != '\0'; i++) {
			if(argument[i] == 'l')
				is_l = 1;
			else if(argument[i] == 'a')
				is_a = 1;
			else if(argument[i] == 'h')
				is_h = 1;
		}
	}
	// Check if it is possible to open the specified path.
	file_stat_possible = lstat(directory, &file_stat);
	if(file_stat_possible == -1) {
		invalid_path();
	}
	else {
		if(S_ISREG(file_stat.st_mode)) {
			file_stat_possible = lstat(directory, &file_stat);
			if(file_stat_possible == -1) {
				invalid_path();
			}
			else {
				if(!is_l) {
					if(S_ISDIR(file_stat.st_mode)) {
						write(1, ANSI_COLOR_BLUE, 5);
					}
					else if(S_ISLNK(file_stat.st_mode)) {
						write(1, ANSI_COLOR_GREEN, 5);
					}
					for(len = 0; directory[len] != '\0'; len++);
					write(1, directory, len);
					if(S_ISDIR(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) {
						write(1, ANSI_COLOR_RESET, 4);
					}
					write(1, "  ", 2);
				}
				else {
					write(1, (S_ISDIR(file_stat.st_mode)) ? "d": "-", 1);
					write(1, (file_stat.st_mode & S_IRUSR) ? "r": "-", 1);
					write(1, (file_stat.st_mode & S_IWUSR) ? "w": "-", 1);
					write(1, (file_stat.st_mode & S_IXUSR) ? "x": "-", 1);
					write(1, (file_stat.st_mode & S_IRGRP) ? "r": "-", 1);
					write(1, (file_stat.st_mode & S_IWGRP) ? "w": "-", 1);
					write(1, (file_stat.st_mode & S_IXGRP) ? "x": "-", 1);
					write(1, (file_stat.st_mode & S_IROTH) ? "r": "-", 1);
					write(1, (file_stat.st_mode & S_IWOTH) ? "w": "-", 1);
					write(1, (file_stat.st_mode & S_IXOTH) ? "x": "-", 1);
					write(1, " ", 1);
					human_size((int) file_stat.st_nlink, 0, 1);
					human_size((int) file_stat.st_uid, 0, 1);
					human_size((int) file_stat.st_gid, 0, 1);
					human_size((int) file_stat.st_size, is_h, 1);
					curr_time = (long long int) file_stat.st_ctime + 19800;
					time_m = (curr_time / 60) % 60;
					time_h = (curr_time / (60 * 60)) % 24;
					human_size((int) time_h, 0, 0);
					write(1, ":", 1);
					human_size((int) time_m, 0, 1);
					if(S_ISDIR(file_stat.st_mode)) {
						write(1, ANSI_COLOR_BLUE, 5);
					}
					else if(S_ISLNK(file_stat.st_mode)) {
						write(1, ANSI_COLOR_GREEN, 5);
					}
					for(len = 0; directory[len] != '\0'; len++);
					write(1, directory, len);
					if(S_ISDIR(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) {
						write(1, ANSI_COLOR_RESET, 4);
					}
					if(S_ISLNK(file_stat.st_mode)) {
						write(1, " -> ", 4);
						read_link = readlink(full_path, link_target, file_stat.st_size + 1);
						if(read_link == -1)
							read_link_error();
						else {
							link_target[file_stat.st_size] = '\0';
							for(i = 0; link_target[i] != '\0'; i++);
							write(1, ANSI_COLOR_GREEN, 5);
							write(1, link_target, i);
							write(1, ANSI_COLOR_RESET, 4);
						}
					}
					write(1, "\n", 1);
				}
				if(!is_l)
					write(1, "\n", 1);

			}
		}
		else if(S_ISDIR(file_stat.st_mode)) {
			dir_content = open(directory, O_RDONLY | O_DIRECTORY);
			if(dir_content == -1) {
				write(2, ANSI_COLOR_RED "There was an error while fetching directory contents.\n" ANSI_COLOR_RESET, 63);
				exit(1);
			}
			else {
				for( ; ; ) {
					status = syscall(SYS_getdents, dir_content, buf, BUF_SIZE);
					if(status == 0)
						break;
					for(cnt = 0; cnt < status; ) {
						d = (struct linux_dirent *) (buf + cnt);
						for(len = 0; d->d_name[len] != '\0'; len++);
						if((is_a) || (!is_a && d->d_name[0] != '.')) {
							j = 0;
							for(i = 0; directory[i] != '\0'; i++)
								full_path[j++] = directory[i];
							if(full_path[j-1] != '/')
								full_path[j++] = '/';
							for(i = 0; d->d_name[i] != '\0'; i++)
								full_path[j++] = d->d_name[i];
							full_path[j] = '\0';
							file_stat_possible = lstat(full_path, &file_stat);
							if(file_stat_possible == -1) {
								invalid_path();
							}
							else {
								if(!is_l) {
									if(S_ISDIR(file_stat.st_mode)) {
										write(1, ANSI_COLOR_BLUE, 5);
									}
									else if(S_ISLNK(file_stat.st_mode)) {
										write(1, ANSI_COLOR_GREEN, 5);
									}
									write(1, d->d_name, len);
									if(S_ISDIR(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) {
										write(1, ANSI_COLOR_RESET, 4);
									}
									write(1, "  ", 2);
								}
								else {
									write(1, (S_ISDIR(file_stat.st_mode)) ? "d": "-", 1);
									write(1, (file_stat.st_mode & S_IRUSR) ? "r": "-", 1);
									write(1, (file_stat.st_mode & S_IWUSR) ? "w": "-", 1);
									write(1, (file_stat.st_mode & S_IXUSR) ? "x": "-", 1);
									write(1, (file_stat.st_mode & S_IRGRP) ? "r": "-", 1);
									write(1, (file_stat.st_mode & S_IWGRP) ? "w": "-", 1);
									write(1, (file_stat.st_mode & S_IXGRP) ? "x": "-", 1);
									write(1, (file_stat.st_mode & S_IROTH) ? "r": "-", 1);
									write(1, (file_stat.st_mode & S_IWOTH) ? "w": "-", 1);
									write(1, (file_stat.st_mode & S_IXOTH) ? "x": "-", 1);
									write(1, " ", 1);
									human_size((int) file_stat.st_nlink, 0, 1);
									open_etc_passwd((int) file_stat.st_uid, 1, 0);
									open_etc_passwd((int) file_stat.st_gid, 0, 1);
									human_size((int) file_stat.st_size, is_h, 1);
									curr_time = (long long int) file_stat.st_ctime + 19800;
									curr_time_copy = curr_time;
									year = timestamp_to_year(&curr_time_copy);
									print_month((int) timestamp_to_month(&curr_time_copy, year));
									curr_time_copy /= (60 * 60 * 24);
									curr_time_copy += 1;
									human_size((int) curr_time_copy, 0, 1);
									time_m = (curr_time / 60) % 60;
									time_h = (curr_time / (60 * 60)) % 24;
									human_size((int) time_h, 0, 0);
									write(1, ":", 1);
									human_size((int) time_m, 0, 1);
									if(S_ISDIR(file_stat.st_mode)) {
										write(1, ANSI_COLOR_BLUE, 5);
									}
									else if(S_ISLNK(file_stat.st_mode)) {
										write(1, ANSI_COLOR_GREEN, 5);
									}
									write(1, d->d_name, len);
									if(S_ISDIR(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) {
										write(1, ANSI_COLOR_RESET, 4);
									}
									if(S_ISLNK(file_stat.st_mode)) {
										write(1, " -> ", 4);
										read_link = readlink(full_path, link_target, file_stat.st_size + 1);
										if(read_link == -1)
											read_link_error();
										else {
											link_target[file_stat.st_size] = '\0';
											for(i = 0; link_target[i] != '\0'; i++);
											write(1, ANSI_COLOR_GREEN, 5);
											write(1, link_target, i);
											write(1, ANSI_COLOR_RESET, 4);
										}
									}
									write(1, "\n", 1);
								}
							}
						}
						cnt += d->d_reclen;
					}
					if(!is_l) {
						write(1, "\n", 1);
					}
				}
			}
		}
	}
	return 0;
}
