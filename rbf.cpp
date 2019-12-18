#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define n_rings 1*1024*1024

#define d_rotr(y,x) ((y>>x)|(y<<(32-x)))
#define d_rotl(y,x) ((y<<x)|(y>>(32-x)))
#define one_step_rbf(y,key) ((d_rotr(y,key->rotr))^(d_rotl(y,key->rotl))^(y>>key->shift))


struct key_t
{
	unsigned int rounds;
	unsigned int rotr;
	unsigned int rotl;
	unsigned int shift;
};
unsigned int rings_idx[n_rings];
unsigned int tot_rounds[n_rings+1];
unsigned int tot_nrounds=0;

struct key_t *keys=NULL;
unsigned int nkeys=0;

unsigned int compute_ring(unsigned int start_ring, struct key_t *KEY,unsigned int half_ring)
{
	unsigned int i;
	unsigned int work;

	work = start_ring;

	for (i = 0; i <= KEY->rounds; i++)
	{
		unsigned int result = one_step_rbf(work, KEY);


		if((half_ring!=0)&&(half_ring==i))
			tot_rounds[tot_nrounds++]=result;


		if (i == KEY->rounds) 
		{
			if (result == start_ring)
			{
				return work;
			}
		}
		else
		{
			work = result;
		}

		/* printf("work %x \n", work); */
	}

	return 0;
}

unsigned int find_ring_cycle(struct key_t *KEY)
{
	int goods;
	unsigned int work;
	unsigned int start_ring ,nrounds;
	unsigned int found=0;

	goods=0;

	while(goods<4)
	{
		nrounds = 0;
		start_ring = rand();
		work = start_ring;

		while(1)
		{
			unsigned int result = one_step_rbf(work, KEY);
		
			if(result==start_ring)
			{
				if(goods>0)
				{
					if(KEY->rounds!=nrounds)
					{
						return 0;
					}
				}
				goods++;
				KEY->rounds=nrounds;
				break;
			}

			work = result;
			nrounds++;

			if(nrounds>=256000)
				return 0;

			/* printf("work %x \n", work); */
		}
	}

	if(goods<4)
		return 0;

	return 1;
}


int find_keys()
{
	int r,l,s;
	
	FILE *file=fopen("keylst.csv","wb");
	fprintf(file,"rotr;rotl;shift;rounds\n");

	for(r=1;r<=24;r++){
		for(l=1;l<=24;l++){
			for(s=8;s<=31;s++){

				struct key_t KEY;
				KEY.rotr=r;
				KEY.rotl=l;
				KEY.shift=s;
				
				if(find_ring_cycle(&KEY))
				{
					struct key_t *k = &KEY;
					unsigned int start;
					unsigned int result;
					unsigned int proof;
					unsigned int error,i;


					if(k->rounds>4)
					{
						error = 0;
						for(i=0;i<256;i++)
						{

							start  = rand();
							result = compute_ring(start,k,0);
							proof  = one_step_rbf(result, k);

							if(proof != start)
							{
								error=1;
								break;
							}
						}

						if(!error)
							fprintf(file,"%u;%u;%u;%u\n",KEY.rotr,KEY.rotl,KEY.shift,KEY.rounds);
					}
				}
			}
		}
	}
	fclose(file);
	return 1;
}

int load_keys()
{
	char line[1024];

	FILE *file=fopen("keylst.csv","rb");

	fgets(line, 1024, file);

    while (fgets(line, 1024, file))
    {
		unsigned int num;
		const char* tok;

		if(nkeys==0)
			keys=(struct key_t *)malloc(sizeof(struct key_t));
		else
			keys=(struct key_t *)realloc(keys,(nkeys+1)*sizeof(struct key_t));

		for (num = 0,tok = strtok(line, ";");tok && *tok; num++,tok = strtok(NULL, ";\n"))
		{
			switch(num)
			{
				case 0:
					keys[nkeys].rotr = strtol(tok,NULL,10);
				break;
				case 1:
					keys[nkeys].rotl = strtol(tok,NULL,10);
				break;
				case 2:
					keys[nkeys].shift = strtol(tok,NULL,10);
				break;
				case 3:
					keys[nkeys].rounds = strtol(tok,NULL,10);
				break;
			}
		}
		nkeys++;
    }
	fclose(file);

	return 1;

}

void create_ring_path(unsigned int *rings, unsigned int n)
{
	unsigned int i;
	for(i=0;i<n;i++)
	{
		rings[i]=rand() % nkeys;
	}

}



int main(int argc,char **argv)
{
	unsigned int	START;
	int				rings;
	unsigned int	i,start_ring, proof,n_tests;
	struct			timeval;
	time_t			start_time,compute_time,check_time;

	srand(time(0));

	/*find_keys();*/

	

	printf("loading rings\n");

	load_keys			();
	create_ring_path	(rings_idx,n_rings);

	START		= rand();
	start_ring  = START;
	n_tests		= n_rings;

	

	tot_rounds[tot_nrounds++]=start_ring;
		
	printf("finding signature for %x with %d rings \n", START,n_rings);

	start_time = time(0);

	for(rings =0; rings <n_rings; rings++)
	{
		unsigned int new_ring = compute_ring(start_ring, &keys[rings_idx[rings]],0 );

		if(new_ring == 0)
		{
			n_tests = rings;
			break;
		}
		start_ring=new_ring;

		tot_rounds[tot_nrounds++]=start_ring;

		/* printf("ring %d signature %x \n",  rings + 1, start_ring); */
	}

	proof = start_ring;
	compute_time = time(0);

	printf("work done in %u secs, signature %x \n",compute_time-start_time, proof);

	start_time = time(0);
	
	for (rings = (n_tests-1); rings >= 0; rings--)
	{
		/*printf("sig %d = %x \n", rings, proof);*/
		proof = one_step_rbf(proof, ((struct key_t *)(&keys[rings_idx[rings]])));
	}

	check_time = time(0);


	if (proof == START)
	{
		printf("proof of work valid %x, %u secs \n", proof,check_time-start_time);
	}

	FILE *file=fopen("result.txt","wb");
	for (i = 0; i < tot_nrounds; i++)
	{
		fprintf(file,"%d ", i + 1);
	}

	fprintf(file,"\n");
	for (i = 0; i < tot_nrounds; i++)
	{
		fprintf(file,"%u ", tot_rounds[i]);
	}

	fprintf(file,"\n");
	fclose(file);
	
	/*
	for (i = 0; i < tot_nrounds; i++)
	{
		printf("%x ", tot_rounds[i]);
	}
	printf("\n");
	*/

	getc(stdin);   
	

}