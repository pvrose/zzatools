//! This file will get "touched" before recompiling. It provides a compile timestamp for ZZALOG.

std::string TIMESTAMP = std::string(__DATE__) + " " + std::string(__TIME__) + " Local";
