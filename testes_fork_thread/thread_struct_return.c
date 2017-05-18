#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
 
struct nums {
    int num[3];
};
 
void *fac(void *arg) {
    struct nums *args = arg;
    struct nums *result = malloc(sizeof(struct nums));
 
    int i;
 
    result->num[0] = 1;
    result->num[1] = 1;
    result->num[2] = 1;
 
    for(i = args->num[0]; i > 0; i--)
        result->num[0] *= i;
    
 
    printf("%d\n", result->num[0]);
    sleep(1);
 
    for(i = args->num[1]; i > 0; i--)
        result->num[1] *= i;
    
 
    printf("%d\n", result->num[1]);
    sleep(1);
 
    for(i = args->num[2]; i > 0; i--)
        result->num[2] *= i;
    
 
    printf("%d\n", result->num[2]);
    sleep(1);
 
    return((void *) result);
}
 
int main(int argc, char *argv[]) {
    struct nums arg1;
    struct nums arg2;
 
    struct nums *res1;
    struct nums *res2;
 
    int ret = 0;
 
    pthread_t thread1, thread2;
 
    arg1.num[0] = 5;
    arg1.num[1] = 15;
    arg1.num[2] = 12;
 
    arg2.num[0] = 7;
    arg2.num[1] = 8;
    arg2.num[2] = 10;
 
    pthread_create(&thread1, NULL, &fac, &arg1);
    pthread_create(&thread2, NULL, &fac, &arg2);
 
    ret = pthread_join(thread1, (void *) &res1);
    if(ret == 0)
        printf("returned: %d, %d, %d\n", res1->num[0], res1->num[1], res1->num[2]);
    
 
    ret = pthread_join(thread2, (void *) &res2);
    if(ret == 0)
        printf("returned: %d, %d, %d\n", res2->num[0], res2->num[1], res2->num[2]);
    

    return(0);
}
