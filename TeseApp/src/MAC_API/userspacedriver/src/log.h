static FILE *f; // log file
static int l;   // log level

//#define LOG_LEVEL_EMERG 0
//#define LOG_LEVEL_ALERT 1
#define LOG_LEVEL_ERROR 2
//#define LOG_LEVEL_WARNI 3
//#define LOG_LEVEL_NOTIC 4
//#define LOG_LEVEL_INFOR 5
#define LOG_LEVEL_DEBUG 6

#define LOG_INIT(log_file,log_level) { \
	f = log_file;  \
	l = log_level; \
}

#define LOG_ERROR(m, ...) {            \
	if(l >= LOG_LEVEL_ERROR) { \
		_LOG("ERROR", m, ##__VA_ARGS__);  \
	}                                  \
}

#define LOG_DEBUG(m, ...) {            \
	if(l >= LOG_LEVEL_DEBUG) { \
		_LOG("DEBUG", m, ##__VA_ARGS__);  \
	}                                  \
}

#define _LOG(p, m, ...) {                                                                 \
	fprintf(f, "%s - %s:%s:%d: " m "\n", p, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
}
