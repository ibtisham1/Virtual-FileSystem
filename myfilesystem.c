#include <stdlib.h>
#include <stdio.h>
#include "myfilesystem.h"
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>

/*A struct called container was created, that contains three filepointers,
	and 3 lengths of the each filepointers*/
typedef struct container container;

struct container {
	FILE* fptr_1;
	FILE* fptr_2;
	FILE* fptr_3;
	long int File_Data_Length;
	long int Directory_Table_Length;
	long int Hash_Table_Length;
};



/* This the initilaising function. I take each file and open it in read mode,
and then I find the sizes of each file. Then,using a struct pointer,
I set all these values to the struct elements.*/
void * init_fs(char * f1, char * f2, char * f3, int n_processors) {
	long int size=0;
	long int size_2=0;
	long int size_3=0;
	
	FILE* fil1= fopen(f1, "rb+"); //this following lines like the rest, uses fseek to go the end,
	fseek(fil1,0,SEEK_END);        // and it uses ftell to find the size in int type.
	size=ftell(fil1);
	fseek(fil1, 0, SEEK_SET);
	rewind(fil1);
	
	FILE* fil2= fopen(f2,"rb+");
	fseek(fil2,0,SEEK_END);
	size_2=ftell(fil2);
	fseek(fil2, 0, SEEK_SET);
	rewind(fil2);
	
	FILE* fil3= fopen(f3,"rb+");
	fseek(fil3,0,SEEK_END);
	size_3=ftell(fil3);
	fseek(fil3, 0, SEEK_SET);
	rewind (fil3);

	container* new_pointer= malloc(sizeof(container)); //struct pointer memory allocated
	
	new_pointer->fptr_1=fil1;
	new_pointer->fptr_2=fil2;
	new_pointer->fptr_3=fil3;
	new_pointer->File_Data_Length=size;
	new_pointer->Directory_Table_Length=size_2;
	new_pointer->Hash_Table_Length=size_3;
	

	return (void*)new_pointer;	
}

/* This function closes all open file pointers and frees the helper*/
void close_fs(void * helper) {
	
	container* pointer=helper;
	fclose(pointer->fptr_2);
	fclose(pointer->fptr_3);
    free(helper);
}


/*The create file function first checks if the file exists,
	and if it doesnt, it creates the file*/
int create_file(char * filename, size_t length, void * helper) {	
	
	container* ptr=helper;
	rewind(ptr->fptr_1);

	unsigned char name_of_file[strlen(filename)]; //creating array that has filename
	memcpy(name_of_file,filename,strlen(filename));
	
	unsigned char zero[length]; //creating a length 0 byte array
	for(int i=0;i<length;i++){
		zero[i]=0;
	}
	unsigned char directory_space[72]; //creating array with 72 0 bytes
	for(int i=0;i<72;i++){
		directory_space[i]=0;
	}	
	//check if file exists
	rewind(ptr->fptr_2);
	int found=-1;
	int i=0;
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer[strlen(filename)];
		fread(buffer,strlen(filename),1,ptr->fptr_2);		
		int ans=memcmp(name_of_file,buffer,strlen(filename));
		if(ans==0){
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	if(found==1){ // If the file exists, it returns 1.
		return 1;
	}

	//Gets all the offsets from the Directory Table
	int num_2=0;
	num_2=(int)((ptr->Directory_Table_Length)/72);
	unsigned char offset_buffer[num_2*4];	//The array that contains all binary values of offsets
	int index_i=64;
	int index_j=0;
	fseek(ptr->fptr_2,64,SEEK_SET);
	while(index_i<ptr->Directory_Table_Length){
		fread(&offset_buffer[index_j],1,1,ptr->fptr_2);
		fseek(ptr->fptr_2,index_i+1,SEEK_SET);
		fread(&offset_buffer[index_j+1],1,1,ptr->fptr_2);
		fseek(ptr->fptr_2,index_i+2,SEEK_SET);
		fread(&offset_buffer[index_j+2],1,1,ptr->fptr_2);
		fseek(ptr->fptr_2,index_i+3,SEEK_SET);
		fread(&offset_buffer[index_j+3],1,1,ptr->fptr_2);
		index_j+=4;
		index_i+=72;
		fseek(ptr->fptr_2,index_i,SEEK_SET);
		
	}
	int num_3=0;
	for(int i=0;i<sizeof(offset_buffer);i++){ //loop to remove the excess zeros
		if ((offset_buffer[i]==0)&&(offset_buffer[i+1]==0)&&(offset_buffer[i+2]==0)&&(offset_buffer[i+3]==0)&&(offset_buffer[i+4]==0)){
			num_3=i+3;
			break;			
		}
	}
	unsigned char new_offset_buffer[num_3];		//contians the hexa values of the offset
	for(int i=0;i<sizeof(new_offset_buffer);i++){
		new_offset_buffer[i]=offset_buffer[i];
	}

	int actual_offset[num_3/4];		//contains the decimal values of the offset
	FILE* new_pointer=fopen("apple2","wb+");
	ftruncate(fileno(new_pointer),12);
	fwrite(new_offset_buffer,sizeof(new_offset_buffer),1,new_pointer);
	fclose(new_pointer);
	FILE* new_ptr=fopen("apple2","rb+");		

	for(int i=0;i<num_3/4;i++){
		actual_offset[i]=getw(new_ptr);
		
	}
	fclose(new_ptr);
	if(((sizeof(actual_offset))/sizeof(int))>2){ //if number of files is greater than two, then continue
		int temp;	//sort the actual_offset array
		for(int i=0;i<num_3/4;i++){
			for(int j=0;j<num_3/4;j++){
				if(actual_offset[i]<actual_offset[j]){
					temp=actual_offset[i];
					actual_offset[i]=actual_offset[j];
					actual_offset[j]=temp;					
				}
			}			
		}
		int yes=-1; //check if there is space between files
		for(int i=0;i<num_3/4-1;i++){
			if((actual_offset[i+1]-actual_offset[i])!=0){
				yes=1;				
			}
		}
		if(yes==1){ //if there is space between files, then repack
			repack(helper);
		}
	}
	size_t offset_of_file=0; //find the offset to put the file in
	int has_space=-1; //check if there is space in the file data table
	int file_index=0;
	rewind(ptr->fptr_1);
	while(file_index<ptr->File_Data_Length){		
		unsigned char filebuffer3[length];
		fread(filebuffer3,length,1,ptr->fptr_1);			
		int ans2=memcmp(filebuffer3,zero,length);
		if (ans2==0){
			has_space=1;
			offset_of_file=ftell(ptr->fptr_1)-length;
			break;
		}
		fseek(ptr->fptr_1,file_index+1,SEEK_SET);
		file_index++;

	}

	if(has_space==-1){ //if there is no space, then return 2 and exit
		return 2;
	}
	if(has_space==1){
		
		FILE *new_pointer_3= fopen(filename,"wb+"); //creating the file array
		ftruncate(fileno(new_pointer_3), length+1);
		fclose(new_pointer_3);
		FILE*new_pointer_4=fopen(filename,"rb+");
		
		//put the file into in the file data
		fseek(ptr->fptr_1,(int)offset_of_file,SEEK_SET);
		fwrite(new_pointer_4,length,1,ptr->fptr_1);
		fclose(new_pointer_4);
		
		//check where to put the filename, length and offset in the directory table
		size_t directory_offset=0;
		int directory_index=0;
		rewind(ptr->fptr_2);
		while(directory_index<ptr->Directory_Table_Length){
			unsigned char buffer[72];
			fread(buffer,sizeof(buffer),1,ptr->fptr_2);
			int ans3=memcmp(buffer,directory_space,72);
			if(ans3==0){
				directory_offset=ftell(ptr->fptr_2);
				break;
			}
			fseek(ptr->fptr_2,directory_index+1,SEEK_SET);
			directory_index++;
			
		}
		
		//put the filename,offset and length in the directory table
		fseek(ptr->fptr_2,(int)directory_offset-72,SEEK_SET);
		fwrite(filename,strlen(filename),1,ptr->fptr_2);
		fseek(ptr->fptr_2,(int)directory_offset-72+64,SEEK_SET);
		fwrite(&offset_of_file,4,1,ptr->fptr_2);
		fseek(ptr->fptr_2,(int)directory_offset-72+68,SEEK_SET);
		fwrite(&length,4,1,ptr->fptr_2);
		fseek(ptr->fptr_2,0,SEEK_SET);
		rewind(ptr->fptr_2);
		fseek(ptr->fptr_2,0,SEEK_END);

		return 0;		
	}	
	return 2;   

}
/*This function resizes the size of a file given*/
int resize_file(char * filename, size_t length, void * helper) {

	container* ptr=helper;
	rewind(ptr->fptr_2);
	
	//Find total space left in file data
	unsigned char bufffer[1];
	bufffer[0]=0;
	int total_space=0;
	int ind=0;
	rewind(ptr->fptr_1);
	while(ind<ptr->File_Data_Length){		
		unsigned char file_buffer[1];
		fread(file_buffer,1,1,ptr->fptr_1);			
		int ans2=memcmp(file_buffer,bufffer,1);
		if (ans2==0){
			total_space+=1;		
		}
		fseek(ptr->fptr_1,ind+1,SEEK_SET);
		ind++;

	}
	//get the offset and the size of the file
	unsigned char offset_of_file[4];
	unsigned char file_name_set[4];
	int directory_offset=0;
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	int i=0;
	int found=-1;
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer[strlen(filename)];
		fread(buffer,sizeof(buffer),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer,strlen(filename));
		if(ans==0){
			directory_offset=ftell(ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+68,SEEK_SET);
			fread(file_name_set,sizeof(file_name_set),1,ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+64,SEEK_SET);
			fread(offset_of_file,sizeof(offset_of_file),1,ptr->fptr_2);
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	if(found==-1){
		return 1;
	}
	///the original offset,converting to int from hexa
	FILE* p=fopen("op","wb+");
	ftruncate(fileno(p),4);
	fwrite(offset_of_file,sizeof(offset_of_file),1,p);
	fclose(p);
	FILE* p_2=fopen("op","rb+");
	int off;			
	off =getw(p_2);
	fclose(p_2);
	
	
	if(found==1){		
		
	//the original length,converting to int from hexa
		FILE* pt=fopen("new3","wb+");
		ftruncate(fileno(pt),4);
		fwrite(file_name_set,sizeof(file_name_set),1,pt);
		fclose(pt);
		FILE* pt_2=fopen("new3","rb+");
		int file_size_1;			
		file_size_1 =getw(pt_2);
		fclose(pt_2);
		
	//taking in all the offsets;		
		int numbers=0;
		numbers=(int)((ptr->Directory_Table_Length)/72);
		unsigned char all_buffer[numbers*4];
		int index_i=64;
		int index_j=0;
		fseek(ptr->fptr_2,64,SEEK_SET);
		while(index_i<ptr->Directory_Table_Length){
			fread(&all_buffer[index_j],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+1,SEEK_SET);
			fread(&all_buffer[index_j+1],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+2,SEEK_SET);
			fread(&all_buffer[index_j+2],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+3,SEEK_SET);
			fread(&all_buffer[index_j+3],1,1,ptr->fptr_2);
			index_j+=4;
			index_i+=72;
			fseek(ptr->fptr_2,index_i,SEEK_SET);

		}

		int actual_offset[numbers];///contains the decimal values of the offsets
		FILE* op_3=fopen("apple2","wb+");
		ftruncate(fileno(op_3),12);
		fwrite(all_buffer,sizeof(all_buffer),1,op_3);
		fclose(op_3);
		FILE* op_4=fopen("apple2","rb+");		
		for(int i=0;i<numbers;i++){
			actual_offset[i]=getw(op_4);
		}
		fclose(op_4);

		///check how many files there are
		int count_=0;
		for(int i=0;i<numbers;i++){
			if(actual_offset[i]>0){
				count_++;
			}
		}
		int next_big_offset=0; //find the next biggest offset
		for(int i=0;i<numbers;i++){
			if(actual_offset[i]>off){
				next_big_offset=actual_offset[i];
				
			}
		}
		if(next_big_offset==0){
			next_big_offset=ptr->File_Data_Length;//if there is no bigger offset,
		}											//it is set to the length of the file
		
		int p=0;// p is the offset of the directory table where the file was found
		for(int i=0;i<numbers;i++){
			if(actual_offset[i]==off){
				break;
				}
			p+=72;
			}
		
		//if the total space is greater than or equal to the length needed, then continue
		if(((length-file_size_1)<=total_space) &&(count_>1)){
			//if the space between the two files is lesser than the length needed, thenrepack
			if((next_big_offset-off)<=(length-file_size_1)){
				
				///saved directory table data
				unsigned char directory_data[72];
				fseek(ptr->fptr_2,p,SEEK_SET);
				fread(directory_data,sizeof(directory_data),1,ptr->fptr_2);
				
				///search for the 72 file in dt table and put 0
				unsigned char all_zeros[72];
				for(int i=0;i<72;i++){
					all_zeros[i]=0;
				}
				fseek(ptr->fptr_2,p,SEEK_SET);
				fwrite(all_zeros,72,1,ptr->fptr_2);
				rewind(ptr->fptr_2);
				
				///now put 0 in the file data and  save data
				unsigned char saved_data[file_size_1];
				fseek(ptr->fptr_1,off,SEEK_SET);
				fread(saved_data,sizeof(saved_data),1,ptr->fptr_1);
				unsigned char file_zero[file_size_1];
				for(int i=0;i<file_size_1;i++){
					file_zero[i]=0;
				}
				fseek(ptr->fptr_1,off,SEEK_SET);
				fwrite(file_zero,sizeof(file_zero),1,ptr->fptr_1);
				rewind(ptr->fptr_1);
				
				//so after taking information of the file out of the directory table
				//and file data table, we repack
				repack(helper);
				
				///find the new offset to put the file in
				int new_offset=0;
				unsigned char file_data_1[ptr->File_Data_Length];
				rewind(ptr->fptr_1);
				fread(file_data_1,sizeof(file_data_1),1,ptr->fptr_1);
				for(int i=0;i<sizeof(file_data_1);i++){
					if(file_data_1[i]==0){
						new_offset=i;
					}
				}
				//write back the filedata
				fseek(ptr->fptr_1,new_offset,SEEK_SET);
				fwrite(saved_data,file_size_1,1,ptr->fptr_1);
				rewind(ptr->fptr_1);
				fseek(ptr->fptr_2,p,SEEK_SET);
				fwrite(directory_data,72,1,ptr->fptr_2);
				
				//write back the directory
				fseek(ptr->fptr_2,p+64,SEEK_SET);
				fwrite(&new_offset,4,1,ptr->fptr_2);
				fseek(ptr->fptr_2,p+68,SEEK_SET);
				fwrite(&length,4,1,ptr->fptr_2);
				rewind(ptr->fptr_2);
				return 0;
			
			}
			fseek(ptr->fptr_2,p+68,SEEK_SET);
			fwrite(&length,4,1,ptr->fptr_2);
			rewind(ptr->fptr_2);
			return 0;
			
		}
		// if the given length is greater than the original length
		if(length>file_size_1){

			fseek(ptr->fptr_2,p+68,SEEK_SET);
			fwrite(&length,4,1,ptr->fptr_2);
			rewind(ptr->fptr_2);			
			
			if(total_space<=length){//if theres no space to write the remaining length, exit
				return 2;
				}
			return 0;
			}
			
		fseek(ptr->fptr_2,p+68,SEEK_SET);
		fwrite(&length,4,1,ptr->fptr_2);		
		rewind(ptr->fptr_2);
		return 0;
	
	}
	return 1;
   
}
/*This function repacks the whole file data*/
void repack(void * helper) {
	container* ptr=helper;
	rewind(ptr->fptr_2);

	//checking if directory tbale is empty or not
	rewind(ptr->fptr_2);
	int empty=1;
	unsigned char len[ptr->Directory_Table_Length];
	fread(len,sizeof(len),1,ptr->fptr_2);
	for(int i=0;i<ptr->Directory_Table_Length;i++){
		if((int)len[i]!=0){
		empty=-1;
		
		}
	}
	if(empty==-1){//if the directory table is not empty, then continue


		///taking in all the sizes 

		int num=0;
		num=(int)((ptr->Directory_Table_Length)/72);
		unsigned char buffer[num*4];
		int i=68;
		int j=0;
		fseek(ptr->fptr_2,68,SEEK_SET);
		while(i<ptr->Directory_Table_Length){
			fread(&buffer[j],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,i+1,SEEK_SET);
			fread(&buffer[j+1],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,i+2,SEEK_SET);
			fread(&buffer[j+2],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,i+3,SEEK_SET);
			fread(&buffer[j+3],1,1,ptr->fptr_2);
			j+=4;
			i+=72;
			fseek(ptr->fptr_2,i,SEEK_SET);

		}

		int actual_size[num];///contains the decimal values of the sizes
		FILE* op=fopen("apple","wb+");
		ftruncate(fileno(op),12);
		fwrite(buffer,sizeof(buffer),1,op);
		fclose(op);
		FILE* op_2=fopen("apple","rb+");		
		for(int i=0;i<num;i++){
			actual_size[i]=getw(op_2);

		}
		fclose(op_2);

		////making a tag number to keep track
		unsigned char tag[num];
		for(int i=0;i<num;i++){
			tag[i]=i;
		}

		////taking in all he offsets;
		int num_2=0;
		num_2=(int)((ptr->Directory_Table_Length)/72);
		unsigned char all_buffer[num_2*4];
		int index_i=64;
		int index_j=0;
		fseek(ptr->fptr_2,64,SEEK_SET);
		while(index_i<ptr->Directory_Table_Length){
			fread(&all_buffer[index_j],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+1,SEEK_SET);
			fread(&all_buffer[index_j+1],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+2,SEEK_SET);
			fread(&all_buffer[index_j+2],1,1,ptr->fptr_2);
			fseek(ptr->fptr_2,index_i+3,SEEK_SET);
			fread(&all_buffer[index_j+3],1,1,ptr->fptr_2);
			index_j+=4;
			index_i+=72;
			fseek(ptr->fptr_2,index_i,SEEK_SET);

		}


		int actual_offset[num_2];///contains the decimal values of the offsets
		FILE* op_3=fopen("apple2","wb+");
		ftruncate(fileno(op_3),12);
		fwrite(all_buffer,sizeof(all_buffer),1,op_3);
		fclose(op_3);
		FILE* op_4=fopen("apple2","rb+");		
		for(int i=0;i<num_2;i++){
			actual_offset[i]=getw(op_4);

		}
		fclose(op_4);

		//Putting them all into one 2D array
		size_t all_in_one[num][3];
		for(int i=0;i<num;i++){
			all_in_one[i][0]=tag[i];
			all_in_one[i][1]=actual_offset[i];
			all_in_one[i][2]=actual_size[i];
		}
			//sorting them by their offsets
		size_t temp[3];
		for(int i=0;i<num;i++){
			for(int j=0;j<num;j++){
				if(all_in_one[i][1]<all_in_one[j][1]){
					temp[0]=all_in_one[i][0];
					temp[1]=all_in_one[i][1];
					temp[2]=all_in_one[i][2];
					all_in_one[i][0]=all_in_one[j][0];
					all_in_one[i][1]=all_in_one[j][1];
					all_in_one[i][2]=all_in_one[j][2];
					all_in_one[j][0]=temp[0];
					all_in_one[j][1]=temp[1];
					all_in_one[j][2]=temp[2];
				}
			}
		}
			//main algorithm, that edits the offsets of each file
		all_in_one[0][1]=0; //sets the startinf offset as 0
		for(int i=1;i<num;i++){ //basically shifts everything to the left, and calculates new offsets.
			all_in_one[i][1]=all_in_one[i-1][1]+all_in_one[i-1][2];
		}
			//sorts them back to their original position, by their tag numbers;
		size_t temp_2[3];
		for(int i=0;i<num;i++){
			for(int j=0;j<num;j++){
				if(all_in_one[i][0]<all_in_one[j][0]){
					temp_2[0]=all_in_one[i][0];
					temp_2[1]=all_in_one[i][1];
					temp_2[2]=all_in_one[i][2];
					all_in_one[i][0]=all_in_one[j][0];
					all_in_one[i][1]=all_in_one[j][1];
					all_in_one[i][2]=all_in_one[j][2];
					all_in_one[j][0]=temp_2[0];
					all_in_one[j][1]=temp_2[1];
					all_in_one[j][2]=temp_2[2];
				}
			}
		}

		//all the edits were done, now to update the offsets in the diectory table

		size_t another[num];//contains all the offsets
		for(int i=0;i<num;i++){
			another[i]=all_in_one[i][1];
		}
		//all the offsets are being put back
		int index_ii=64;
		int index_jj=0;
		fseek(ptr->fptr_2,64,SEEK_SET);
		while(index_jj<num){
			fwrite(&another[index_jj],4,1,ptr->fptr_2);
			index_ii+=72;
			fseek(ptr->fptr_2,index_ii,SEEK_SET);

			index_jj++;


		}
		fseek(ptr->fptr_2,0,SEEK_SET);

		//Now i want to edit the file data file
		rewind(ptr->fptr_1);
		unsigned char filed[ptr->File_Data_Length];
		fread(filed,sizeof(filed),1,ptr->fptr_1);

			// the following 20 lines, shifts all the zeros to the left
		int count_=0;
		for(int i=0;i<sizeof(filed);i++){
			if(filed[i]==0){
				count_++;

			}
		}
		unsigned char without_zeros[sizeof(filed)-count_];
		int i2=0;
		for(int i=0;i<sizeof(filed);i++){
			if(filed[i]!=0){
				without_zeros[i2]=filed[i];
				i2++;
			}
		}
		unsigned char final_array[sizeof(filed)];
		for(int i=0;i<sizeof(without_zeros);i++){
			final_array[i]=without_zeros[i];
		}
		for(int i=sizeof(without_zeros);i<sizeof(final_array);i++){
			final_array[i]=0;
		}
		//then write the edited file data back
		rewind(ptr->fptr_1);
		fwrite(&final_array,ptr->File_Data_Length,1,ptr->fptr_1);
		rewind(ptr->fptr_1);

		fseek(ptr->fptr_1,0,SEEK_SET);
	}
	else{
		
	}
	return;

}
/*This function deletes a file*/
int delete_file(char * filename, void * helper) {
	
	container* ptr=helper;	
	rewind(ptr->fptr_2);
	//checks if the file exists, and if so, it takes in the file name;
	unsigned char file_name_set[64];
	int directory_offset=0;	
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	int i=0;
	int found=-1;
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer_2[strlen(filename)];
		fread(buffer_2,sizeof(buffer_2),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer_2,strlen(filename));
		if(ans==0){
			directory_offset=ftell(ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename),SEEK_SET);
			fread(file_name_set,sizeof(file_name_set),1,ptr->fptr_2);			
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}	
	if(found==1){
		//if the file exists, it then writes a zero in the first chsracter of its name
		// this deletes the file
		int t=0;
		fseek(ptr->fptr_2,directory_offset-strlen(filename),SEEK_SET);
		fwrite(&t,1,1,ptr->fptr_2);		
		return 0;
	}
	else{
		
	}
    return 1;


}
/*This function renames a file*/
int rename_file(char * oldname, char * newname, void * helper) {
	
	container* ptr=helper;	
	rewind(ptr->fptr_2);
	unsigned char buffer_[strlen(newname)];		//Makes buffer that contains newname
	memcpy(buffer_,newname,strlen(newname));
	
	int directory_offset=0; //gets the offset of the direcotry table if the oldname exists
	unsigned char buffer_3[strlen(oldname)];	//Makes buffer than contains  oldname
	memcpy(buffer_3,oldname,strlen(oldname));
	
	int j=0;
	int found_2=-1;	//checks if the oldname exists
	while(j<ptr->Directory_Table_Length){
		unsigned char buffer[strlen(oldname)];
		fread(buffer,sizeof(buffer),1,ptr->fptr_2);
		int ans=memcmp(buffer_3,buffer,strlen(oldname));		
		if(ans==0){			
			directory_offset=ftell(ptr->fptr_2);
			found_2=1;
			break;
		}
		fseek(ptr->fptr_2,j+1,SEEK_SET);
		j++;
	}

	int i=0;
	int found=-1; //checks if the newname exists
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer_42[strlen(newname)];
		fread(buffer_42,sizeof(buffer_42),1,ptr->fptr_2);
		int ans=memcmp(buffer_,buffer_42,strlen(newname));	
		if(ans==0){						
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	if((found==1)||(found_2!=1)){ //if the newname exists, or if the odlname doesn't, returns 1
		return 1;
	}
	else{
		//writes in the new name
		fseek(ptr->fptr_2,directory_offset-strlen(oldname),SEEK_SET);
		fwrite(buffer_,sizeof(buffer_),1,ptr->fptr_2);		
	}
    return 0;
}
/*This function reads from a file*/
int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
	
	container* ptr=helper; 
	rewind(ptr->fptr_1);

	rewind(ptr->fptr_2);
	
	//if the file exists, it takes in the size and the offset values of the file
	unsigned char size_[4];
	unsigned char offset_[4];		
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	int directory_offset=0;	
	int i=0;
	int found=-1;
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){	
		unsigned char buffer_2[strlen(filename)];
		fread(buffer_2,sizeof(buffer_2),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer_2,strlen(filename));
		if(ans==0){
			directory_offset=ftell(ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+68,SEEK_SET);
			fread(size_,sizeof(size_),1,ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+64,SEEK_SET);
			fread(offset_,sizeof(offset_),1,ptr->fptr_2);
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	
	
	//calculating the decimal of the file offset
	FILE* pt=fopen("new3","wb+");
	ftruncate(fileno(pt),4);
	fwrite(offset_,sizeof(offset_),1,pt);
	fclose(pt);
	FILE* pt_2=fopen("new3","rb+");		
	int file_offset;
	file_offset =getw(pt_2);
	fclose(pt_2);

	//calculating the decimal of the file size
	FILE* new=fopen("new","wb+");
	ftruncate(fileno(new),4);
	fwrite(size_,sizeof(size_),1,new);
	fclose(new);
	FILE* new_2=fopen("new","rb+");			
	int file_size;
	file_size =getw(new_2);
	fclose(new_2);

	if(found==1){
		if(((file_size-offset)<count)||(offset>file_size)){ //if the offset given, makes it hard to read
			return 2;											// count bytes, return 2
		}
		
		//reading it into the buf
		fseek(ptr->fptr_1,file_offset+offset,SEEK_SET);
		fread(buf,count,1,ptr->fptr_1);
		fseek(ptr->fptr_1,0,SEEK_SET);
		return 0;		
	}
	else{
		
	}
    return 1;

}
/*This function writes into a file*/
int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
	
    container* ptr=helper; 	
	
	//if the file exists, it takes in the size and the offset values of the file
	unsigned char size_[4];
	unsigned char offsett_[4];		
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	int directory_offset=0;	
	int i=0;
	int found=-1;
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){	
		unsigned char buffer_2[strlen(filename)];
		fread(buffer_2,sizeof(buffer_2),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer_2,strlen(filename));
		if(ans==0){
			directory_offset=ftell(ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+68,SEEK_SET);
			fread(size_,sizeof(size_),1,ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+64,SEEK_SET);
			fread(offsett_,sizeof(offsett_),1,ptr->fptr_2);
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	
	
	//calculating the decimal of the file offset
	FILE* pt=fopen("new3","wb+");
	ftruncate(fileno(pt),4);
	fwrite(offsett_,sizeof(offsett_),1,pt);
	fclose(pt);
	FILE* pt_2=fopen("new3","rb+");	
	int file_offset;
	file_offset =getw(pt_2);
	fclose(pt_2);

	
	//calculating the decimal of the file size
	FILE* new=fopen("new","wb+");
	ftruncate(fileno(new),4);
	fwrite(size_,sizeof(size_),1,new);
	fclose(new);
	FILE* new_2=fopen("new","rb+");				
	int file_size;
	file_size =getw(new_2);
	fclose(new_2);

	if(found==1){
		if(offset>file_size){//if the offset if greater than the size, we cannot write anything
			return 2;
		}
		//write fromt he buf and into the file
		fseek(ptr->fptr_1,file_offset+offset,SEEK_SET);
		fwrite(buf,count,1,ptr->fptr_1);
		rewind(ptr->fptr_1);
		
		
		return 0;
		
	}
	else{
		
	}
    return 1;


}
/*This function retrieves the size of a file*/
ssize_t file_size(char * filename, void * helper) {
	
	container* ptr=helper;
	rewind(ptr->fptr_2);

	
	//if the file exists, then take in the size of the file
	unsigned char size[4];
	int  directory_offset=0;		
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	int i=0;
	int found=-1;
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer_2[strlen(filename)];
		fread(buffer_2,sizeof(buffer_2),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer_2,strlen(filename));
		if(ans==0){
			directory_offset=ftell(ptr->fptr_2);
			fseek(ptr->fptr_2,directory_offset-strlen(filename)+68,SEEK_SET);
			fread(size,sizeof(size),1,ptr->fptr_2);
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}

	if (found==1){
		//calculating the decimal of the size found and returning it
		FILE* new=fopen("new","wb+");
		ftruncate(fileno(new),4);
		fwrite(size,sizeof(size),1,new);
		fclose(new);
		FILE* new_2=fopen("new","rb+");
		int file_size;
		file_size =getw(new_2);
		fclose(new_2);
		return file_size;
		
	}
	else{
	}
	
	return -1;

}
//this is just  a function i use to help me with my testcases
int checker(char* filename,void* helper){
	container* ptr=helper;

	
	/*rewind(ptr->fptr_1);
	unsigned char dt[ptr->File_Data_Length];
	fread(dt,sizeof(dt),1,ptr->fptr_1);
	for(int i=0;i<sizeof(dt);i++){
		printf("%u ",dt[i]);
	}
	printf(" \n");*/
	rewind(ptr->fptr_2);
	unsigned char buffer[strlen(filename)];
	memcpy(buffer,filename,strlen(filename));
	/*for(int i=0;i<sizeof(buffer);i++){
		printf("%u ",buffer[i]);
	}
	printf(" \n");*/
	int i=0;
	int found=-1;
	rewind(ptr->fptr_2);
	while(i<ptr->Directory_Table_Length){
		unsigned char buffer_2[strlen(filename)];
		fread(buffer_2,sizeof(buffer_2),1,ptr->fptr_2);
		int ans=memcmp(buffer,buffer_2,strlen(filename));
		if(ans==0){
			found=1;
			break;
		}
		fseek(ptr->fptr_2,i+1,SEEK_SET);
		i++;
	}
	if(found==1){
		return 0;
	}
	return 1;
	
}
/*This function hashes arrays inputted*/
void fletcher(uint8_t * buf, size_t length, uint8_t * output) {
	
	uint32_t * buf_2=(uint32_t *)buf; //converted to a uint32 buf, as per pseudocode
	size_t newlength= length/4; //as its a uint32 buf now, we now make the length a quarter of what it was
	uint64_t divider=(uint64_t)(pow(2,32)-1);
	uint64_t a=0;
	uint64_t b=0;
	uint64_t c=0;
	uint64_t d=0;
	for(int i=0;i<newlength;i++){
		a = (a + buf_2[i])%divider;
		b = (b + a)%divider;
		c = (c + b)%divider;
		d = (d + c)%divider;
	
		
	}
	//I cast the bytes to uint32 then copy them into the output, by 4 bytes for each
	uint64_t array[4]={a,b,c,d};
	int j=0;
	for(int i=0;i<4;i++){
		uint32_t temp=(uint32_t)array[i];
		memcpy(output+j,&temp,4);
		j+=4;
	}

    return;
}
/*This function creates a has tree*/
void compute_hash_tree(void * helper) {
	
	container* ptr=helper;
	
	//divide the filedata into 256 byte blocks
	size_t size=ptr->File_Data_Length;	
	int number_of_blocks=size/256; //the totoal nubmber of blocks
	int number_of_lines=0;//total number of levels in hash tree
	int total_number_of_arrays=size/256; //the total number of arrays that need to be created
	while((int)number_of_blocks!=1){
		number_of_blocks=number_of_blocks/2;
		total_number_of_arrays+=number_of_blocks;
		number_of_lines+=1;
	}
	uint8_t all_arrays[total_number_of_arrays][16]; //2d array that will contain the fletcher values
	rewind(ptr->fptr_1);								
	//does a loop through the 256 byte blocks, and fletches them into 16 byte hash blocks
	int j=0;
	for(int i=0;i<size/256;i++){
		fseek(ptr->fptr_1,j,SEEK_SET);
		uint8_t temp_array[256];
		fread(temp_array,sizeof(temp_array),1,ptr->fptr_1);
		fletcher(temp_array,256,all_arrays[i]);
		j+=256;
		
	}
	//now it does a nested loop by the number of levels it calculated at the start
	//and store the fletcher values array by array
	int index=0;
	int all_array_index=size/256;
	int decreasing_index=(size/256)/2;
	for(int i=0;i<number_of_lines;i++){
		for(int k=0;k<decreasing_index;k++){			
			uint8_t temp_array[32]; //for the concatenation of two fletcher arrays
		
			for(int j=0;j<16;j++){
				temp_array[j]=all_arrays[index][j];
			}
			int l=0;
			for(int j=16;j<32;j++){
				temp_array[j]=all_arrays[index+1][l];
				l++;
			}

			fletcher(temp_array,32,all_arrays[all_array_index]);
			index+=2;
			all_array_index++;
			
		}
		decreasing_index-=decreasing_index/2;

	}

	uint8_t one_dimensional_array[total_number_of_arrays*16];//1d form of the previous 2d array

	//I find a way to arrange the fletcher hash arrays to the wanted hash data format
	int initial_boundary=(size/256);
	int barrier_boundary=0;
	int barrier=barrier_boundary;
	int t=0;
	for(int i=initial_boundary-1;i>=barrier;i--){		
		if(t>=total_number_of_arrays*16){
			break;
		}
		for(int j=15;j>=0;j--){
			one_dimensional_array[t]=all_arrays[i][j];
			t++;
		}
		
		if(i==barrier){			
			initial_boundary=(size/256)+initial_boundary/2;
			i=initial_boundary;			
			barrier_boundary=(size/256)+barrier_boundary/2;
			barrier=barrier_boundary;
		}	
	}
	//i reverse the array order into the reverse_array
	uint8_t rerverse_array[total_number_of_arrays*16];
	int h=0;
	for(int i=(total_number_of_arrays*16)-1;i>=0;i--){
		rerverse_array[h]=one_dimensional_array[i];
		h++;
	}
	//I then write the array into the hash data
	rewind(ptr->fptr_3);
	fwrite(rerverse_array,sizeof(rerverse_array),1,ptr->fptr_3);
	fseek(ptr->fptr_3,0,SEEK_SET);
	rewind(ptr->fptr_3);


    return;

}

void compute_hash_block(size_t block_offset, void * helper) {
    return;
}

