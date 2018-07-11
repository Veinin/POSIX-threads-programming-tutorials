#include "errors.h"

int main(void) {
	int seconds;
	char line[128];
	char message[64];

	while(1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL)
			exit(0);

		if (strlen(line) == 0)
			continue;

		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
			fprintf(stderr, "Bad command\n");
		} else {
			sleep(seconds);
			printf("(%d) %s\n", seconds, message);
		}
	}
}