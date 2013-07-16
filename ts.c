#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<stdint.h>
#include<stdlib.h>
#include<math.h>


FILE *tfp;
//timestamps are recorded in 'tfp' to calculate deviation
 
unsigned long int tm_total=0,processcount=0,dtime=0; //time statistics
float mean=0,deviation=0,ftm_total=0;

struct timeval start,stop; //time fetchers

char d_input[10],d_output[10],e_input[10],e_output[10],encode_stat[20]="encode_stat.txt",decode_stat[20]="decode_stat.txt";
/*
d_input[10]: input filename for decoding function
d_output[10]: output filename for decoding function
e_input[10]:input filename for encoding function
e_output[10]:output filename for encoding function
*/

FILE *fp,*fp1;
//fp:input file pointer(to read)
//fp1:output file pointer(to write)

int8_t num,astat=0; 
//num is the buffer used here. 
//astat is the flag indicating  -ve /+ve sign of the difference  of values after dot

unsigned long p_ad,p_bd,c_ad,c_bd,abase,bbase;
/*
 p_ad:previous after-dot value   p_bd:previous before-dot value   
 c_ad:current after-dot value    c_bd:previous before-dot value
 abase:base value to add with differences of after-dot values, in decoding
 bbase:base value to add with differences of before-dot values, in decoding
*/
int bit[32],lcount=1;     
//lcount is for counting decoded lines. From the TS_decode function, first line is taken and so initialized to 1
//bit[32] is used to test bits of the scanned number

static unsigned long int bit_total=64,bytecount=8;
//bit_total and bytecount count bits and bytes respectively in encoding process


// displays  mean time and deviation for encoding or decoding each timestamp
void print_time_stat(){

  tfp=fopen("time_log.txt","r");
	ftm_total=0;
	while(!feof(tfp)){
			if(fread(&dtime,sizeof(dtime),1,tfp)==0)
				break;
			ftm_total+=(dtime-mean)*(dtime-mean);	
	}
	fclose(tfp);
	printf("Mean in micro seconds:%f\n",mean);
	printf("Deviation in micro seconds:%f\n",deviation=sqrt((double)ftm_total/processcount));
}


//save timestamp for decoding or encoding each timestamp in the local file "time_log.txt"
void processtime(struct timeval v1,struct timeval v2){
dtime=(v1.tv_sec*1000000+v1.tv_usec)-(v2.tv_sec*1000000+v2.tv_usec);
tm_total+=dtime;
fwrite(&dtime,sizeof(dtime),1,tfp);
processcount++;
}



//this function returns the number of bytes needed for currently read line of input file, in encoding function
int no_struct(int count){
    if(count<=8)
                  return 1;
    else if(count <=16)
                  return 2;
    else if(count <= 24)
                  return 3;
}


//this function returns the power of a to b
long int pw(int a,int b){
     long int temp=1;
     int i;
     for(i=0;i<b;i++){
                      temp*=2;
                      }
     return temp;
     }
     

//this displays bits of 32 bit integer
void displaybit32(int no){
     int i;
      for(i=31;i>=0;i--){
                        printf(" %d ",(no&pw(2,i))?1:0);
            }                  
                        printf(" \n ");                  
}     
     

//Displays bits of 8 bit integer
void displaybit(int no){
     int i;
      for(i=7;i>=0;i--){
                        printf(" %d ",(no&pw(2,i))?1:0);
            }                  
                        printf(" \n ");                  
}
     
     


//counts bits of given integer
int bitcount(long int a){
    int i=0;
    for(i=32;i>0;i--){
                      if((pw(2,i-1)&a)!=0){
                      return i;
                      }
          }
          if(i==0)
          return 0;
}     


//encodes f:firstpart and l:lastpart of the line (if line is 13645298765.678543   f=13645298765  and l:678543)         
void encode(long int f,long int l){
     int fbit,lbit,nos,bits,i,j,nos_t;
     
/*
fbit: holds the number of bits required to makeup firstpart
lbit:holds the number of bits required to makeup lastpart
nos: holds the number of bytes required for both 'f' and 'l'
nos_t: tracks the number of used bytes
bits:holds the number of bits needed 
*/

//set buffer num to zero; all bits to '0'
     num=0;

//count bits required to make up 'f' and 'l'
     fbit=bitcount(f);
     lbit=bitcount(l);

//calculate the total bits needed to hold up the information in 'f' and 'l'
     if(f==0)
     bits=3+1+1+lbit;   //3: no. of bytes;  1: flag representing f=0;  1:flag representing the sign of 'l' (1:-ve , 0: +ve); lbit:bits of 'l'
     else
     bits=3+1+3+fbit+1+lbit; //3: no. of bytes;  1:flag saying f!=0; 3:count of bits in 'f';  fbit:bits of 'f'; 1: flag for sign of 'l';  lbit:bits of 'l';  
       

//round bits to nearest upper byte   
        if(bits%8==0)  
        bit_total+=bits;
        else{
             bits=(bits+8-(bits%8));
        bit_total+=bits;
        }

        nos=bits/8;
        nos_t=nos;
        num=nos;

//set the total byte count
        bitcount(nos);
        num<<=5;
        j=0;

//if f!=0 set 'f' in the current byte
                if(f!=0){
                          num+=pw(2,4);                         
                         j=0;
                 num|=(fbit<<1); 
                               
                 i=0;
                 j=0;

                     while(f){
                                  if(f&pw(2,j))
                                   num+=pw(2,i);                                   

                                   
                                   if(i==0){

                                   fwrite(&num,sizeof(int8_t),1,fp1);
                                   
                                   bytecount++;
                          
                                   nos_t--;
                                   if(nos_t==0)
                                   return;
                                   num=0;
                                   i=7;
                                            }
                                            else
                                            i--;                              
                                            f>>=1;
                              }
                 }
                 else{
//if f==0 set index of bit to 3 
                      i=3;
                      }


//set 'l' sign flag
                             if(astat==1){

                          num+=pw(2,i); 
                          }

                                         i--;

//set number l and keep writing the byte when filled, resetting the buffer
                 while(l!=0){                            
                          if(l&pw(2,j))
                          num+=pw(2,i);
                          if(i==0){

                                   fwrite(&num,sizeof(int8_t),1,fp1);
                                   bytecount++;                                                                      
                                   nos_t--;

                                   if(nos_t==0)
                                   return;                                   

                                   num=0;
                                   i=7;
                                   }
                                   else{
                                        i--;
                                        }
                                        l>>=1;
                         }
                         

                         if(i<=7){
                                   fwrite(&num,sizeof(int8_t),1,fp1);                                   
                                   bytecount++;                                   
                                  }                 

                                  
}          

void TS_encode(){
        unsigned long i=0;
	long int ahold,bhold;
	float compression=0;
	
       char *str,*temp;
       str=(char *)malloc(18*sizeof(char));
       temp=(char *)malloc(18*sizeof(char));

	printf("Enter name of  input file for encoding:");
	scanf("%s",e_input);
	printf("Enter name for output encoded file:");
	scanf("%s",e_output);		

        fp=fopen(e_input,"r");
	fp1=fopen(e_output,"w+");       
	tfp=fopen("time_log.txt","w");

//read till end of file
           while(!feof(fp)){
                            i++;
                       

//read a line of 18 bytes       
if(i==1){
gettimeofday(&start,NULL);
}
                if(fread(str,18*sizeof(char),1,fp)==0)
			break;




//split the read line into two parts at the dot

//write the first line as two 32bit integers and encode the rest through the encode function
                temp=(char *)strtok(str,".");

                c_bd=strtoul(temp,NULL,10);
                if(i==1){
                fwrite(&c_bd,sizeof(long int),1,fp1);            
                }
              while(temp){
                          temp=(char *)strtok(NULL,".");
                            if(temp){
                                            c_ad=strtoul(temp,NULL,10);
                                            if(i==1){
                                            fwrite(&c_ad,sizeof(long int),1,fp1);
						gettimeofday(&stop,NULL);
						processtime(stop,start);
                                            }
                                     }
              }

              if(i!=1){                                                                               

                bhold=(c_bd-p_bd);
                ahold=(c_ad-p_ad);

                  if(ahold<0){
                              astat=1;
                              ahold=-ahold;
                  }
                  else
                  astat=0;
		gettimeofday(&start,NULL);
                  encode(bhold,ahold);                  
		gettimeofday(&stop,NULL);
		processtime(stop,start);
			
              }
              p_ad=c_ad; p_bd=c_bd;

		}


       fclose(fp);
       fclose(fp1);
       fclose(tfp);

		printf("\n\nTotal time in micro secs:%ld\n",tm_total);
	  	mean=(float)tm_total/processcount;
	
		print_time_stat();
	        printf("\nbytecount (written):%ld  bitcount:%ld, total lines:%ld, avg_bitcount:%ld\n",bytecount,bit_total,i,bit_total/i);
		
		compression=(float)(18*8-(bit_total/i))*100/(18*8);
		
		printf("Percentage of Compression achieved is %f\n",compression);

		printf("\nCheck '%s' for encoded content\nCheck '%s' for stats\n",e_output,encode_stat);	

	tfp=fopen(encode_stat,"w");
	str=(char *)malloc(50*sizeof(char));
	sprintf(str,"Average bits per line: %ld\n",bit_total/i);
	fputs(str,tfp);	
	sprintf(str,"Degree of Compression: %f\n",compression/100);
	fputs(str,tfp);
	sprintf(str,"Mean time for encoding:%f microseconds \nDeviation:%f microseconds",mean,deviation);	
	fputs(str,tfp);
	fclose(tfp);
}

void decode(int base,FILE *fp,FILE *fp1){
	int noi=0,nob=0,nastat=0,bcount=0,i,ib,ia;
	char *buf,*buffinal;
	unsigned long int ahold=0,bhold=0;
	double fc_ad;
	int8_t arr[8],push=0;

//prepare buffers with memory
	buf=(char *)malloc(sizeof(char)*10);
	buffinal=(char *)malloc(sizeof(char)*18);

//count the no. of bytes to be read further
	for(i=0;i<=2;i++){
		if(base&pw(2,7-i))
		noi+=pw(2,2-i);
	}
	noi--;


//read the remaining bytes
	for(i=0;i<noi;i++){
		fread(&arr[i],sizeof(int8_t),1,fp);
	}


//test bit-5 for first part;  if it is 1, read count of  bits in next 3 bits and read 'f' in that count of bits
	if(base&pw(2,4)){
		for(i=3;i>=1;i--)
			if(base&pw(2,i))
				bcount+=pw(2,i-1);
		ib=0;
		while(bcount--){
			if(base&pw(2,i))
			bhold+=pw(2,ib);
			ib++;
			if(i==0){
				i=7;
				base=arr[push++];
				noi--;
			}
			else
				i--;
		}	


//test for sign of 'l' number
		if(base&pw(2,i))
			nastat=1;
			
			if(i==0){
				i=7;
				base=arr[push++];
				noi--;
			}
			else
				i--;
//read 'l' as 32bit integer	
		ia=0;
		while(noi+1){
			if(base&pw(2,i))
			ahold+=pw(2,ia);				
			ia++;
			if(i==0){
				i=7;
				base=arr[push++];
				noi--;
			}
			else
				i--;
				
		}
		
	}
	else{ 
//if f=0, test for sign of 'l' and read number 'l' in 32 bit integer
		i=3;

		if(base&pw(2,i))
			nastat=1;
			
			if(i==0){
				i=7;
				base=arr[push++];
				noi--;
			}
			else
				i--;
	
		ia=0;
		while(noi+1){
			if(base&pw(2,i))
			ahold+=pw(2,ia);				
			ia++;
			if(i==0){
				i=7;
				base=arr[push++];
				noi--;
			}
			else
				i--;
				
		}

	}

//add the differences to previous offsets
	c_bd=p_bd+bhold;
	if(nastat==0)
	c_ad=p_ad+ahold;
	else
	c_ad=p_ad-ahold;	
	
	p_bd=c_bd; p_ad=c_ad;	

//put the second in .XXXXXX format
	fc_ad=c_ad/1000000.0;

	sprintf(buf,"%6f",fc_ad);

	buf=(char *)strchr(buf,'.');

	sprintf(buffinal,"%ld%s\n",c_bd,buf);

// write the final buffer to output file
	fputs(buffinal,fp1);	
	lcount++;
}

void TS_decode(){

//FILE *fp,*fp1,*tfp;
int8_t hold,count=2;
char *buf,*buffinal;
double fc_ad;
	buf=(char *)malloc(sizeof(char)*10);
	buffinal=(char *)malloc(sizeof(char)*18);

	printf("Enter name of input file for decoding:");
	scanf("%s",d_input);
	printf("Enter name for decoded output file:");
	scanf("%s",d_output);

	fp=fopen(d_input,"r");
	fp1=fopen(d_output,"w");
	tfp=fopen("time_log.txt","w");

//read first two  32 bit integers and set them as base offsets
	gettimeofday(&start,NULL);
	fread(&bbase,sizeof(4),1,fp);
	fread(&abase,sizeof(4),1,fp);
	p_bd=bbase; p_ad=abase;

	fc_ad=abase/1000000.0;
	sprintf(buf,"%6f",fc_ad);
	buf=(char *)strchr(buf,'.');
	sprintf(buffinal,"%ld%s\n",bbase,buf);

//write the two numbers as 18 byte string
	fputs(buffinal,fp1);	
	gettimeofday(&stop,NULL);
	processtime(stop,start);	
//read till end of file and decode using each byte, read
	while(!feof(fp)){

		if(fread(&hold,sizeof(int8_t),1,fp)==0)
		break;
		gettimeofday(&start,NULL);
		decode(hold,fp,fp1);
		gettimeofday(&stop,NULL);
		processtime(stop,start);		
	}

	fclose(fp1);
	fclose(fp);
    	fclose(tfp);
	printf("\n\nTotal time in micro secs:%ld\n",tm_total);
	mean=(float)tm_total/processcount;
	
	print_time_stat();


	tfp=fopen(decode_stat,"w+");

	buf=(char *)malloc(sizeof(char)*80);
	sprintf(buf,"Mean time for decoding:%f microseconds\nDeviation:%f microseconds",mean,deviation);	
	fputs(buf,tfp);
	fclose(tfp);

	printf("\n%d lines written\n",lcount);

	printf("\nCheck '%s' for decoded content\nCheck '%s' for stats\n",d_output,decode_stat);	

}

int main(){

	int x;
	printf("Enter   1:encode\n\t2:decode\n");
	scanf("%d",&x);

	switch(x){
	case 1:TS_encode();
		break;
	case 2:TS_decode();
		break;
	}
       return 0;
       }
