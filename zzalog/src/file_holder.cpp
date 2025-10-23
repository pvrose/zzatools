#include "file_holder.h"

#include "main.h"
#include "status.h"

#include <cstdlib>

file_holder* file_holder_ = nullptr;
uint16_t DEBUG_RESET_CONFIG = 0;

file_holder::file_holder(bool development, std::string directory) {
#ifdef _WIN32
	// Source directory - for resetting reference data
	if (development) {
		// $ProjectDir/../reference
		default_source_directory_ =
			directory + "..\\reference\\";
		default_code_directory_ = directory;
		default_html_directory_ = directory;
	}
	else {
		default_source_directory_ =
			std::string(getenv("ALLUSERSPROFILE")) + "\\" + VENDOR + "\\" + PROGRAM_ID + "\\";
		default_code_directory_ = "";
		default_html_directory_ = 
			std::string(getenv("APPDATA")) + "\\" + VENDOR + "\\" + PROGRAM_ID + "\\";
	}
	// Working directory
	default_data_directory_ =
		std::string(getenv("APPDATA")) + "\\" + VENDOR + "\\" + PROGRAM_ID + "\\";
	// Code directory
#else 
	// Source directory - for resetting reference data
	if (development) {
		// $PWD/../reference
		default_source_directory_ =
			directory + "../reference/";
		default_code_directory_ = directory;
		default_html_directory_ = directory;
	}
	else {
		default_source_directory_ =
			"/etc/" + VENDOR + "/" + PROGRAM_ID + "/";
		default_code_directory_ = "";
		default_html_directory_ =
			std::string(getenv("HOME")) + "/.config/" + VENDOR + "/" + PROGRAM_ID + "/";
	}
	// Working directory
	default_data_directory_ =
		std::string(getenv("HOME")) + "/.config/" + VENDOR + "/" + PROGRAM_ID + "/";
#endif
	default_code_directory_ = directory;
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
