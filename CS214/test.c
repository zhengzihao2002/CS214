#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
struct node {
	int data;
	struct node *next;
};

void insert_head(int n, struct node **p)
{
	assert(p != NULL);
	struct node *new = malloc(sizeof(struct node));
	new->data = data;
	new->next = *p;
	/* HERE */
}
int main(int argc, char **argv)
{

    //获取每个单位的数字，然后输出数字
    /*int num=3838438;
    int temp=num;
    int count=0;
    while(num!=0){
        int digit=num%10;
        num=num/10;
        count++;
    }
    num=temp;
    int arr[count];
    while(num!=0){
        int digit=num%10;
        num=num/10;
        arr[count-1]=digit;
        count--;
    }
    for(int i=0;arr[i]!='\0';i++){
        printf("%d ",arr[i]);
    }
    printf("!\n");
    */

    /*
    int *ptr2;
    ptr2
    int **ptr1;
    ptr2 = &var;
    ptr1 = &ptr2;

    printf("Value of var = %d\n", var );
    printf("Value of var using single pointer = %d\n", *ptr2 );
    printf("Value of var using double pointer = %d\n", **ptr1);
    */
   /*int i=2;
   
    while(i>0){
        printf("%d\n",i);
        i--;
        continue;
    }
    */
   return 0;
}
