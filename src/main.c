#include <zephyr/kernel.h>
#include "zephyr/sleep.h"
#include "zephyr/sys/printk.h"

int main(void){

    for(;;){
        printk("Hello from ZephyrWatch !\n ");
        k_sleep(K_MSEC(500));
    }

    
    return 0;
}