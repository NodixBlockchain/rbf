#include <stdio.h>

#define d_rotr(y,x) ((y>>x)|(y<<(32-x)))
#define d_rotl(y,x) ((y<<x)|(y>>(32-x)))
#define one_step_rbf(y,x) ((d_rotr(y,KEY[x][1]))^(d_rotl(y,KEY[x][2]))^(y>>KEY[x][3]))


unsigned int tot_rounds[1024];
unsigned int tot_nrounds=0;


int main(int argc,char **argv)
{
	unsigned int START=  0x00100000;
	/*unsigned int START = 0x00000001;*/
	/*unsigned int START = 3394426275;*/
	unsigned int i;
	int rings;
	unsigned char KEY[4][4] = { {119,7,9,31},{185,2,3,31},{167,6,5,31},{125,4,11,31} };
	unsigned int start_ring, work, signature;

	start_ring = START;
	work = start_ring;

	tot_rounds[tot_nrounds++] = work;
	
	printf("start num %x \n", START);

	for(rings =0; rings <4; rings++)
	{
		unsigned int rounds = KEY[rings][0];
		work = start_ring;
		
		for (i = 0; i <= rounds; i++)
		{
			unsigned int result = one_step_rbf(work, rings);

			if (i == rounds) 
			{
				if (result == start_ring)
				{
					printf("ring number %d round %d start %x result %x signature %x \n", rings + 1, i, start_ring, result, work);
					start_ring = work;
					tot_rounds[tot_nrounds++] = work;
				}
			}
			else
			{
				work = result;
			}
		}

		
	}
	signature = work;

	for (rings = 3; rings >= 0; rings--)
	{
		unsigned int result = one_step_rbf(work, rings);
		printf("sig %d = %x \n", rings, work);
		if (result == START)
		{
			printf("proof of work valid %x \n", result);
			break;
		}
		work = result;
	}


	for (i = 0; i < tot_nrounds; i++)
	{
		printf("%d ", i + 1);
	}

	printf("\n");
	for (i = 0; i < tot_nrounds; i++)
	{
		printf("%u ", tot_rounds[i]);
	}

	printf("\n");

	for (i = 0; i < tot_nrounds; i++)
	{
		printf("%x ", tot_rounds[i]);
	}

	printf("\n");

}