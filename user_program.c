#include "user_program.h"

#define WR_DATA_TASK_STRUCT _IOW('a','a',int32_t*)
#define WR_DATA_VFSMOUNT _IOW('a','c',int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)

void take_data_from_user(char str[30], int32_t *pid){
    printf("Enter the data to send \n");
    printf("Write your PID: ");
    scanf("%d",pid);
    printf("\nWrite your struct: ");
    scanf("%s",str);
    printf("%s",str);
}

int main(){
    int fd;
    int32_t num;
    char task_struct[] = "task_struct";
    char vfsmount[] = "vfsmount";
    char str[30];
    char buf[256];
    printf("\n IOCTL based Character device operation from user space \n");
    fd = open("/dev/my_device",O_RDWR);

    if(fd<0){
        printf("The device file does not open \n\n");
        return 0;
    }
    take_data_from_user(str,&num);
    printf("Writing value to the driver \n\n");
    if(strcmp((char *)&task_struct,str)==0) {
        ioctl(fd, WR_DATA_TASK_STRUCT, (int32_t * ) & num);
    }
    else if (strcmp((char *)&vfsmount,str)==0){
        ioctl(fd,WR_DATA_VFSMOUNT,(int32_t*)&num);
    }
    else{
        printf("This structure is unknown to me %s\n\n", str);
        close(fd);
        return 0;
    }
    printf("Reading value from driver...\n\n");
    ioctl(fd,RD_DATA,(char*)&buf);
    printf("%s\n",buf);
    printf("Closing driver \n\n");
    close(fd);
    return 0;
}
