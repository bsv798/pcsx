/* broken PsxBios.c */
int getchar() { return 0; }

/* broken LoadCdrom and CheckCdrom */
int sscanf(const char *from, const char *format, char *str, ...) {
	int i, j, k;
	int len = strlen(format);

	for (i = 0, j = 0, k = 0; i < len; ) {
		if (format[i] == '%') {
			i+=2; 	// skip assumed %s :-)
			for (;;) {
				if (format[i] == from[j]) break;
				str[k] = from[j];
				k++; j++;
			}
		} else if (format[i] == ' ') {	// match any number of whitespaces
			for (;;) {
				if (from[j] != ' ') break;
				j++;
			}
			i++;
		} else {
			if (format[i] != from[j]) break; // doesn't match
			j++; i++;
		}
	}
	str[k] = 0;
	return k;
}

/* broken Sio.c */
int stat(const char *file_name, struct stat *buf) { return 0; }
