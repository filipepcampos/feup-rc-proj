#include <stdio.h>
#include <stdlib.h>
#include "core.h"
#include "rcv.h"

int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	setup_sigalarm();

	struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);
	
	char buffer[BUFFER_SIZE];
	framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);

	framecontent fc = create_non_information_frame(CTL_UA);
	emit_frame(fd, &fc);

	int S = 1;
	while(true){
		received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
		printf("	> RCV fc\n");
		if(IS_INFO_CONTROL_BYTE(received_fc.control)){
			bool is_new_frame = S != GET_INFO_FRAME_CTL_BIT(received_fc.control);
			if(received_fc.data_len > 0){
				if(is_new_frame){
					// Store data
					S = 1 - S;
				}
				fc.control = CTL_RR;
			} else {
				if(is_new_frame){
					fc.control = CTL_REJ;
				} else {
					fc.control = CTL_RR;
				}
			}
		}
		if(received_fc.control == CTL_DISC){
			fc.control = CTL_DISC;
			emit_frame_until_response(fd, &fc, CTL_UA);
			break;
		}
		emit_frame(fd, &fc);
	}
	
	disconnect_serial(fd, &oldtio);
	return 0;
}