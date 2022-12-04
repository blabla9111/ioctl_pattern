//
// Created by dinar on 04.12.2022.
//

#include "user_program.h"

#define WR_DATA_NET_DEVICE _IOW('a','a',int32_t*)
#define WR_DATA_DM_IO_MEMORY _IOW('a','c',int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)

void take_data_from_user(char str[30], int32_t *pid){
    printf("Enter the data to send ...\n");
    printf("PID = ");
    scanf("%d",pid);
    printf("\nEnter struct ");
    scanf("%s",str);
    printf("%s",str);
}

int main(){
    int fd;
    int32_t num;
    char net_device[] = "net_device";// your struct name
    char dm_io_memory[] = "dm_io_memory";// your struct name
    char str[30];
    char buf[256];
    printf("\n IOCTL based Character device operation from user space...\n");
    fd = open("/dev/chr_device",O_RDWR);

    if(fd<0){
        printf("Sorry, I can't open the device file...\n\n");
        return 0;
    }
    take_data_from_user(str,&num);
    printf("Writing value to the driver...\n\n");
    if(strcmp((char *)&net_device,str)==0) {
        ioctl(fd, WR_DATA_NET_DEVICE, (int32_t * ) & num);
    }
    else if (strcmp((char *)&dm_io_memory,str)==0){
        ioctl(fd,WR_DATA_DM_IO_MEMORY,(int32_t*)&num);
    }
    else{
        printf("I don't know a struct %s\n\n", str);
        close(fd);
        return 0;
    }
    printf("Reading value from driver...\n\n");
    ioctl(fd,RD_DATA,(char*)&buf);
    printf("%s\n",buf);
    printf("Closing driver...\n\n");
    close(fd);
    return 0;
}