#include "file_holder.h"

#include "main.h"
#include "status.h"

#include <cstdlib>

#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Window.H>

file_holder* file_holder_ = nullptr;
uint16_t DEBUG_RESET_CONFIG = 0;

file_holder::file_holder(const char* arg0, bool& development) {
	char * pwd = fl_getcwd(nullptr, 256);
	std::string run_dir = directory(arg0);
	printf("ZZALOG: Running in %s\n", run_dir.c_str());
	// Try reading from run directory first - if present then
	// we are development
#ifdef _WIN32
	if (run_dir[1] == ':') {
		// We have an absolutre path
		default_source_directory_ = run_dir + "\\..\\";
	} else {
		default_source_directory_ = std::string(pwd) + "\\" + run_dir + "\\";
	}
#else
    if (run_dir[0] == '/') {
		// We have an absolute path
		default_source_directory_ = run_dir + "/../";
	} else {
		default_source_directory_ = std::string(pwd) + "/" + run_dir + "/../";
	}

#endif
	default_code_directory_ = default_code_directory_;
	// Test the path using the icon
	std::string logo = get_filename(FILE_ICON_ZZA);
	Fl_PNG_Image* ilog = new Fl_PNG_Image(logo.c_str());
	if (ilog && !ilog->fail()) {
		development = true;
	} else {
		development = false;
#ifdef _WIN32
		default_source_directory_ = 
			std::string(getenv("ALLUSERSPROFILE")) + "\\" + 
			VENDOR + "\\" + PROGRAM_ID + "\\";
#else
		default_source_directory_ = 
			"/etc/" + VENDOR + "/" + PROGRAM_ID + "/";
#endif
		// Try again in release directory
		logo = get_filename(FILE_ICON_ZZA);
		ilog = new Fl_PNG_Image(logo.c_str());
		if (!ilog || ilog->fail()) {
			printf("ZZALOG: Unable to find logo file - file accesses will fail\n");
			default_source_directory_ = "";
			default_code_directory_ = "";
			default_data_directory_ = "";
			default_html_directory_ = "";
		}
	}
	Fl_Window::default_icon(ilog);

#ifdef _WIN32
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_data_directory_ =
		std::string(getenv("APPDATA")) + "\\" + VENDOR + "\\" + PROGRAM_ID + "\\";
	// Create the working directory
	std::string unixified = default_data_directory_;
	for (size_t pos = 0; pos < unixified.length(); pos++) {
		if (unixified[pos] == '\\') unixified[pos] = '/';
	}
	fl_make_path(unixified.c_str());
	// Code directory
#else 
	default_html_directory_ = default_source_directory_;
	// Working directory
	default_data_directory_ =
		std::string(getenv("HOME")) + "/.config/" + VENDOR + "/" + PROGRAM_ID + "/";
	// Create the working directory
	fl_make_path(default_data_directory_.c_str());
#endif
}

bool file_holder::get_file(file_contents_t type, std::ifstream& is, std::string& filename) {
	const file_control_t& ctrl = FILE_CONTROL.at(type);
	char msg[128];
	if (ctrl.reference) {
		if (ctrl.read_only) {
			// Open source directory
			filename = default_source_directory_ + ctrl.filename;
			is.open(filename);
			if (is.fail()) {
				snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
				if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
				else {
					printf(msg);
					printf("\n");
				}
				return false;
			}
			else {
				return true;
			}
		}
		else if (DEBUG_RESET_CONFIG & ctrl.reset_mask) {
			filename = default_data_directory_ + ctrl.filename;
			// Copy source to working
			if (copy_source_to_working(ctrl)) {
				is.open(filename);
				if (is.fail()) {
					snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
					if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
					else {
						printf(msg);
						printf("\n");
					}
					return false;
				}
				return true;
			}
			else {
				return false;
			}
		}
		else {
			filename = default_data_directory_ + ctrl.filename;
			is.open(filename);
			if (is.fail()) {
				snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
				if (status_) status_->misc_status(ST_WARNING, msg);
				else {
					printf(msg);
					printf("\n");
				}
				// Copy source to working
				if (copy_source_to_working(ctrl)) {
					is.open(filename);
					if (is.fail()) {
						snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
						if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
						else {
							printf(msg);
							printf("\n");
						}
						return false;
					}
					return true;
				}
				else {
					return false;
				}
			}
			else return true;
		}
	}
	else {
		filename = default_data_directory_ + ctrl.filename;
		if (DEBUG_RESET_CONFIG & ctrl.reset_mask) {
			DEBUG_RESET_CONFIG &= ~(ctrl.reset_mask);
			snprintf(msg, sizeof(msg), "FILE: Ignoring %s", filename.c_str());
			if (status_) status_->misc_status(ST_WARNING, msg);
			else {
				printf(msg);
				printf("\n");
			}
			return false;
		}
		is.open(filename);
		if (is.fail()) {
			snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
			if (status_) status_->misc_status(ST_ERROR, msg);
			else {
				printf(msg);
				printf("\n");
			}
			return false;
		}
		return true;
	}
}

bool file_holder::copy_source_to_working(file_control_t ctrl) {
	// Copy source to working
	std::string source = default_source_directory_ + ctrl.filename;
	std::string filename = default_data_directory_ + ctrl.filename;
#ifdef _WIN32
	std::string command = "copy " + source + " " + filename;
#else
	std::string command = "cp " + source + " " + filename;
#endif
	int result = system(command.c_str());
	char msg[256];
	if (result != 0) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %d", result);
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "File Copied %s to %s", source.c_str(), filename.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
	}
	//char msg[128];
	//ifstream ss(source);
	//if (ss.fail()) {
	//	snprintf(msg, sizeof(msg), "FILE: Cannot open %s", source.c_str());
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
	//	return false;
	//}
	//ofstream ds(filename);
	//if (ds.fail()) {
	//	snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
	//	return false;
	//}
	//// Copy file in 16K byte chunks
	//bool ok = true;
	//const int increment = 16 * 1024;
	//char buffer[increment + 1];
	//snprintf(msg, sizeof(msg), "FILE: Copying %s from %s to %s",
	//	filename.c_str(),
	//	default_source_directory_.c_str(),
	//	default_data_directory_.c_str());
	//if (status_) status_->misc_status(ST_NOTE, msg);
	//while (!ss.eof() && ok) {
	//	ss.read(buffer, increment);
	//	ds.write(buffer, ss.gcount());
	//	ok = ds.good() && (ss.good() || ss.eof());
	//}
	//ss.close();
	//ds.close();
	//if (!ok) {
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, "FILE: Copy failed");
	//	return false;
	//}
	DEBUG_RESET_CONFIG &= ~(ctrl.reset_mask);
	return true;
}

bool file_holder::get_file(file_contents_t type, std::ofstream& os, std::string& filename) {
	const file_control_t ctrl = FILE_CONTROL.at(type);
	char msg[128];
	if (ctrl.read_only) {
		snprintf(msg, sizeof(msg), "FILE: Tring to open %s (read-only)", ctrl.filename.c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	filename = default_data_directory_ + ctrl.filename;
	os.open(filename);
	if (os.fail()) {
		snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	return true;
}

// Copy working copy to source for releasing
bool file_holder::copy_working_to_source(file_contents_t type) {
	file_control_t ctrl = FILE_CONTROL.at(type);
	// Copy source to working
	std::string source = default_source_directory_ + ctrl.filename;
	std::string filename = default_data_directory_ + ctrl.filename;
#ifdef _WIN32
	std::string command = "copy " + filename + " " + source;
#else
	std::string command = "cp " + filename  + " " + source;
#endif
	int result = system(command.c_str());
	char msg[256];
	if (result != 0) {
		snprintf(msg, sizeof(msg), "FILE: Copy failed %d", result);
		if (status_) status_->misc_status(ST_ERROR, msg);
		return false;
	}
	else {
		snprintf(msg, sizeof(msg), "File Copied %s to %s", source.c_str(), filename.c_str());
		if (status_) status_->misc_status(ST_NOTE, msg);
	}
	//char msg[256];
	//if (result != 0) {
	//	snprintf(msg, sizeof(msg), "FILE: Copy failed %d", result);
	//	if (status_) status_->misc_status(ST_ERROR, msg);
	//	return false;
	//}
	//char msg[128];
	//ofstream ss(source);
	//if (ss.fail()) {
	//	snprintf(msg, sizeof(msg), "FILE: Cannot open %s", source.c_str());
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
	//	return false;
	//}
	//ifstream ds(filename);
	//if (ds.fail()) {
	//	snprintf(msg, sizeof(msg), "FILE: Cannot open %s", filename.c_str());
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, msg);
	//	return false;
	//}
	//// Copy file in 16K byte chunks
	//bool ok = true;
	//const int increment = 16 * 1024;
	//char buffer[increment + 1];
	//snprintf(msg, sizeof(msg), "FILE: Copying %s from %s to %s",
	//	filename.c_str(),
	//	default_data_directory_.c_str(),
	//	default_source_directory_.c_str());
	//if (status_) status_->misc_status(ST_NOTE, msg);
	//while (!ds.eof() && ok) {
	//	ds.read(buffer, increment);
	//	ss.write(buffer, ds.gcount());
	//	ok = ss.good() && (ds.good() || ds.eof());
	//}
	//ss.close();
	//ds.close();
	//if (!ok) {
	//	if (status_) status_->misc_status(ctrl.fatal ? ST_FATAL : ST_ERROR, "FILE: Copy failed");
	//	return false;
	//}
	return true;
}
