#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
/*
Program to test the ioctl set of the norandom driver's randomness
*/

#define RANDOM_FILE_PATH ("/dev/urandom")
#define RANDOM_BUFFER_CHANGE_IOCTL (1337)
#define MAX_BUFFER_LEN (32)


int main(int argc, char **argv){
    FILE *random_filep = NULL;
    int retval = 0;
    int i = 0;
    int input_str_len = 0;
    char rand_buff[MAX_BUFFER_LEN] = {0};

    if(argc != 2){
        printf("Usage: %s <new random buffer string>\n", argv[0]);
        goto exit;
    }

    input_str_len = strnlen(argv[1], MAX_BUFFER_LEN);

    for(i=0; i<MAX_BUFFER_LEN; i++){
        rand_buff[i] = argv[1][i%input_str_len];
    }
    random_filep = fopen(RANDOM_FILE_PATH, "r");
    if(NULL == random_filep){
        printf("Coudln't open %s for reading\n", RANDOM_FILE_PATH);
        goto exit;
    }


    ioctl(fileno(random_filep), RANDOM_BUFFER_CHANGE_IOCTL, rand_buff);

exit:
    return 0;
}