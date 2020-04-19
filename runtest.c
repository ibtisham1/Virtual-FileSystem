#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>

#define TEST(x) test(x, #x)
#include "myfilesystem.h"

/* You are free to modify any part of this file. The only requirement is that when it is run, all your tests are automatically executed */

//allowd me to create an array from a file
unsigned char* maker(char* name){
    FILE* l =fopen(name,"rb+");
    static unsigned char data_3[1024];
    fread(data_3,1024,1,l);
    fclose(l);
    return data_3;
}
int success() {
    return 0;
}

int failure() {
    return 1;
}

int no_operation() {
    void * helper = init_fs("sample-part1-only_directory_table.bin", "sample-part1-only_file_data.bin", "sample-part1-only_hash_data.bin", 1); // Remember you need to provide your own test files and also check their contents as part of testing
    close_fs(helper);
    return 0;
}
//tests for createfile,filesize,rename,delete and resize
int init_close_createfile_filesize_rename_delete_resize(){
    FILE*f =fopen("empty_file_data.bin","wb+");
	ftruncate(fileno(f),500);
	fclose(f);
	
	FILE*f2 =fopen("empty_directory_table.bin","wb+");
	ftruncate(fileno(f2),1024);
	fclose(f2);
    void * helper = init_fs("empty_file_data.bin", "empty_directory_table.bin", "sample-part1-only_hash_data.bin", 1); // Remember you need to provide your own test files and also check their contents as part of testing
 
    
	char* filename="text.txt";
	unsigned char filename_binary[strlen(filename)];
	memcpy(filename_binary,filename,strlen(filename));
	
	
	char* filename_content="this is my content";
	unsigned char filename_content_binary[strlen(filename_content)];
	memcpy(filename_content_binary,filename_content,strlen(filename_content));
	
	
	unsigned char filename_offset[4];
	int t=0;
	memcpy(filename_offset,&t,4);
	
	
	unsigned char filename_size[4];
	int s=strlen(filename_content);
	memcpy(filename_size,&s,4);
	
	
	unsigned char directory_1[72];
	for(int i=0;i<72;i++){
		directory_1[i]=0;
	}
	for(int i=0;i<sizeof(filename_binary);i++){
		directory_1[i]=filename_binary[i];
	}
	int h=0;
	for(int i=64;i<68;i++){
		directory_1[i]=filename_offset[h];
		h++;
	}
	int h_=0;
	for(int i=68;i<72;i++){
		directory_1[i]=filename_size[h_];
		h_++;
	}
    ////////
    int ans=create_file(filename,s,helper);
    
    unsigned char * map;
    //creates a succesfull file
    map=maker("empty_directory_table.bin");
    int can_create=-1;
    if((memcmp(map,directory_1, 72)==0)&&(ans==0)){
        can_create=1;
    }
    //gets the size of a file that exists
    ssize_t ans_6=file_size(filename,helper);
    int right_size=-1;
    if(ans_6==strlen(filename_content)){
        right_size=1;
        
    }
    //checks if file exists
    int ans_2=create_file(filename,s,helper);
    int file_exists=-1;
    if(ans_2==1){
        file_exists=1;      
    }
    char* other_file="another.txt";
    size_t len=1000;
    int no_space=-1;
    int ans_3 =create_file(other_file,len,helper);
    if(ans_3==2){
        no_space=1;
    }
    //sees that filesize doesnt occur for file that doesnt exist
    ssize_t ans_4=file_size(other_file,helper);
    int no_size=-1;
    if(ans_4==-1){
        no_size=1;
        
    }
    //tests for resize function
    int ans_20=resize_file(filename,10000,helper);
    int ans_21=resize_file(filename,5,helper);
    ssize_t new_length=0;
    if(ans_21==0){
        new_length=file_size(filename,helper);
    }
    int ans_23=resize_file(filename,8,helper);
    ssize_t new_length2=0;
    if(ans_23==0){
        new_length2=file_size(filename,helper);
    }

    int resize_success=-1;
    if((new_length==5)&&(ans_20==2)&&(new_length2==8)){
        resize_success=1;
        
    }
    //tests for rename_file, here they turn filename into newname
    char*newname="somethingelse.txt";
    int newname_success=-1;
    int ans_7 =rename_file(filename,newname,helper);
    if(ans_7==0){
        int ans_8=checker(filename,helper);
        int ans_9=checker(newname,helper);
        if((ans_8==1)&&(ans_9==0)){
            newname_success=1;
        }
    }
    
    
    //tests for delete function, here the actual file name is newname
    int ans_10=delete_file(filename,helper);
    int ans_11=delete_file(newname,helper);
    int ans_12=delete_file(newname,helper);
    int delete_case_success=-1;
    if((ans_10==1)&&(ans_11==0)&&(ans_12==1)){
        delete_case_success=1;
    }
    
    if((can_create==1)&&(file_exists==1)&&(no_space==1)&&(no_size==1)&&(right_size==1)&&(newname_success==1)&&(delete_case_success==1)&&(resize_success==1)){
        close_fs(helper);
        return 0;
        
    }
    close_fs(helper);
    return 1;
}
//testing fro read and write functions
int read_write(){
    char* filename="text2.txt";
	unsigned char filename_binary[strlen(filename)];
	memcpy(filename_binary,filename,strlen(filename));
	
	
	char* filename_content="this is my content";
	unsigned char filename_content_binary[strlen(filename_content)];
	memcpy(filename_content_binary,filename_content,strlen(filename_content));
	
	
	unsigned char filename_offset[4];
	int t=10;
	memcpy(filename_offset,&t,4);
	
	
	unsigned char filename_size[4];
	int s=strlen(filename_content);
	memcpy(filename_size,&s,4);
	
	
	unsigned char directory_1[72];
	for(int i=0;i<72;i++){
		directory_1[i]=0;
	}
	for(int i=0;i<sizeof(filename_binary);i++){
		directory_1[i]=filename_binary[i];
	}
	int h=0;
	for(int i=64;i<68;i++){
		directory_1[i]=filename_offset[h];
		h++;
	}
	int h_=0;
	for(int i=68;i<72;i++){
		directory_1[i]=filename_size[h_];
		h_++;
	}
	
	

	FILE* f_ =fopen("initial_filedata_2.bin","wb+");
	ftruncate(fileno(f_),1024);
	fclose(f_);
	FILE*f_1 =fopen("initial_filedata_2.bin","rb+");
	fseek(f_1,10,SEEK_SET);
	fwrite(filename_content_binary,sizeof(filename_content_binary),1,f_1);
	fclose(f_1);
	
	FILE*f__ =fopen("initial_directory_2.bin","wb+");
	ftruncate(fileno(f__),1024);
	fclose(f__);
	FILE*f__1 =fopen("initial_directory_2.bin","rb+");
	fwrite(directory_1,sizeof(directory_1),1,f__1);
	fclose(f__1);
    
    
    void * helper = init_fs("initial_filedata_2.bin", "initial_directory_2.bin", "sample-part1-only_hash_data.bin", 1); // Remember you need to provide your own test files and also check their contents as part of testing

    unsigned char answer_binary[7];
    int j=0;
    for(int i=10;i<17;i++){
        answer_binary[j]=filename_content_binary[i];
        j++;
    }

    
    unsigned char got_answer[7];
    for(int i=0;i<7;i++){
        got_answer[i]=0;
    }
    char* other_name="apple";
    // tests for read function
    int ans_1=read_file(filename,10,7,(void *)got_answer,helper);
    int ans_2=read_file(filename,20,7,(void*)got_answer,helper);
    int ans_3 =read_file(other_name,1,1,(void*)got_answer,helper);
    int read_success=-1;
    if((ans_1==0)&&(ans_2==2)&&(ans_3==1)){
        if(memcmp(got_answer,answer_binary,7)==0){
            read_success=1;
            
        }
    }
    
    char* words="aaa";
    unsigned char words_binary[strlen(words)];
    memcpy(words_binary,words,strlen(words));
    
    char* total_file="this is my contentaaa";
    unsigned char total_file_binary[strlen(total_file)];
    memcpy(total_file_binary,total_file,strlen(total_file));
    unsigned char compare[1024];
    for(int i=0;i<sizeof(compare);i++){
        compare[i]=0;
    }
    int r=0;
    for(int i=10;i<sizeof(total_file_binary);i++){
        compare[i]=total_file_binary[r];
        r++;
    }
    // tests for write function
    int write_success=-1;
    int ans_4=write_file(filename,strlen(filename_content),3,words_binary,helper);
    int ans_5=write_file(other_name,1,1,words_binary,helper);
    int ans_6=write_file(filename,100,2,words_binary,helper);
    if((ans_4==0)&&(ans_5==1)&&(ans_6==2)){
        unsigned char *map;
        map=maker("initial_filedata_2.bin");
        if(memcmp(map,compare,strlen(total_file))==0){
            write_success=1;
        }
    }
    if((read_success==1)&&(write_success==1)){
        close_fs(helper);
        return 0;
        
    }

    close_fs(helper);
    return 1;
}
//testing for the repack function
int test_repack(){
     char* filename="text.txt";
	unsigned char filename_binary[strlen(filename)];
	memcpy(filename_binary,filename,strlen(filename));
	
	
	char* filename_content="this is my content";
	unsigned char filename_content_binary[strlen(filename_content)];
	memcpy(filename_content_binary,filename_content,strlen(filename_content));
	
	
	unsigned char filename_offset[4];
	int t=10;
	memcpy(filename_offset,&t,4);
	
	
	unsigned char filename_size[4];
	int s=strlen(filename_content);
	memcpy(filename_size,&s,4);
	
	
	unsigned char directory_1[72];
	for(int i=0;i<72;i++){
		directory_1[i]=0;
	}
	for(int i=0;i<sizeof(filename_binary);i++){
		directory_1[i]=filename_binary[i];
	}
	int h=0;
	for(int i=64;i<68;i++){
		directory_1[i]=filename_offset[h];
		h++;
	}
	int h_=0;
	for(int i=68;i<72;i++){
		directory_1[i]=filename_size[h_];
		h_++;
	}
	
	

	FILE* f_ =fopen("initial_filedata_3.bin","wb+");
	ftruncate(fileno(f_),1024);
	fclose(f_);
	FILE*f_1 =fopen("initial_filedata_3.bin","rb+");
	fseek(f_1,10,SEEK_SET);
	fwrite(filename_content_binary,sizeof(filename_content_binary),1,f_1);
	fclose(f_1);
	
	FILE*f__ =fopen("initial_directory_3.bin","wb+");
	ftruncate(fileno(f__),1024);
	fclose(f__);
	FILE*f__1 =fopen("initial_directory_3.bin","rb+");
	fwrite(directory_1,sizeof(directory_1),1,f__1);
	fclose(f__1);
    
    void * helper = init_fs("initial_filedata_3.bin", "initial_directory_3.bin", "sample-part1-only_hash_data.bin", 1); // Remember you need to provide your own test files and also check their contents as part of testing

    char*newfile="some.txt";
    char* newfile_2="someother.txt";
    
    create_file(newfile,50,helper);
    create_file(newfile_2,20,helper);
    repack(helper);
    unsigned char* map;
    map=maker("initial_filedata_3.bin");
    //tests to see if all the file data has shift to the left
    int repack_success=-1;
    for(int i=0;i<(strlen(filename_content)+50+20);i++){
        if(map[i]!=0){
            repack_success=1;
        }
    }
    if(repack_success==1){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    
    return 1;
}
int fletcher_hash_tree(){
    uint8_t answer[16]={105, 111, 117, 123 ,170, 191 ,212 ,233 ,106 ,161 ,217 ,17 ,176, 44 ,171 ,41};
    uint8_t out[16];
    uint8_t array[24];
    for(int i=0;i<24;i++){
        array[i]=220+i;
    }
    
    char* filename="text.txt";
	unsigned char filename_binary[strlen(filename)];
	memcpy(filename_binary,filename,strlen(filename));
	
	
	char* filename_content="this is my content";
	unsigned char filename_content_binary[strlen(filename_content)];
	memcpy(filename_content_binary,filename_content,strlen(filename_content));
	
	
	unsigned char filename_offset[4];
	int t=10;
	memcpy(filename_offset,&t,4);
	
	
	unsigned char filename_size[4];
	int s=strlen(filename_content);
	memcpy(filename_size,&s,4);
	
	
	unsigned char directory_1[72];
	for(int i=0;i<72;i++){
		directory_1[i]=0;
	}
	for(int i=0;i<sizeof(filename_binary);i++){
		directory_1[i]=filename_binary[i];
	}
	int h=0;
	for(int i=64;i<68;i++){
		directory_1[i]=filename_offset[h];
		h++;
	}
	int h_=0;
	for(int i=68;i<72;i++){
		directory_1[i]=filename_size[h_];
		h_++;
	}
	
	

	FILE* f_ =fopen("initial_filedata_4.bin","wb+");
	ftruncate(fileno(f_),1024);
	fclose(f_);
	FILE*f_1 =fopen("initial_filedata_4.bin","rb+");
	fseek(f_1,10,SEEK_SET);
	fwrite(filename_content_binary,sizeof(filename_content_binary),1,f_1);
	fclose(f_1);
	
	FILE*f__ =fopen("initial_directory_4.bin","wb+");
	ftruncate(fileno(f__),1024);
	fclose(f__);
	FILE*f__1 =fopen("initial_directory_4.bin","rb+");
	fwrite(directory_1,sizeof(directory_1),1,f__1);
	fclose(f__1);
    void * helper = init_fs("initial_filedata_4.bin", "initial_directory_4.bin", "sample-part1-only_hash_data.bin", 1); // Remember you need to provide your own test files and also check their contents as part of testing


    char*newfile="some.txt";
    char* newfile_2="someother.txt";
    
    create_file(newfile,50,helper);
    create_file(newfile_2,20,helper);
    unsigned char ans[16]={47,152,70,191,248,148,222,213,213,228,135,253,174,32,21,138};
    compute_hash_tree(helper);
    unsigned char*map;
    map=maker("sample-part1-only_hash_data.bin");
    


    //testing the root value of hash, and the expected fletcher output
    fletcher(array,24,out);
    if(memcmp(answer,out,16)==0&&(memcmp(map,ans,16)==0)){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
    
}

/****************************/

/* Helper function */
void test(int (*test_function) (), char * function_name) {
    int ret = test_function();
    if (ret == 0) {
        printf("Passed %s\n", function_name);
    } else {
        printf("Failed %s returned %d\n", function_name, ret);
    }
}

/************************/

int main(int argc, char * argv[]) {
    
    // You can use the TEST macro as TEST(x) to run a test function named "x"
    TEST(success);
    TEST(failure);
    TEST(no_operation);
    TEST(init_close_createfile_filesize_rename_delete_resize);
    TEST(read_write);
    TEST(test_repack);
    TEST(fletcher_hash_tree);

    // Add more tests here

    return 0;
}

