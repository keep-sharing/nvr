#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define MAX_FILE_LEN 2048
#define MAX_SEARCH_KEYWORD_LEN 64*1024*1024 // follow busybox
#define MAX_WRITE_CONTENT_LEN 1024*1024*10
char module_tree_root[MAX_FILE_LEN]={0};
const char ALIAS_STRING[]="alias=";
const char DEPENDS_STRING[]="depends=";
const char KSYMTAB_STRING[]="__ksymtab_";
const char KSYMTAB_GPL_STRING[]="__ksymtab_gpl";
const char KSYMTAB_STRINGS_STRING[]="__ksymtab_strings";
char image_content[MAX_SEARCH_KEYWORD_LEN] ={0};


void replace_char(char *ptr,char old_char,char new_char, int len){

    int i=0;
    for(i=0;i<len;i++){
        if(ptr[i]==old_char){
            ptr[i]=new_char;
        }
    }
}


int parse_depneds_string(char *write_content, char* image_content, int len){


    char *ptr =image_content;
    char *old_ptr=image_content;
    int ret=0;
    if(image_content == NULL){
        printf("parse_depneds_string: image_content NULL fail\n");
        return -1;
    }
    if(write_content ==NULL){
        printf("parse_depneds_string: write_content=NULL fail\n");
        return -1;
    }
    //if last len not enough, no need check
    if(len <= (int)strlen(DEPENDS_STRING)){
        return 0;
    }
    
    len = len -strlen(DEPENDS_STRING);
    while(len >0){
        ptr = memchr(ptr,DEPENDS_STRING[0], len);
        if(ptr == NULL){
            //can not find depend first word 'd', no need check more
            sprintf(write_content,"%s\n",write_content);
            return 0;
        }
        ret = memcmp(ptr, DEPENDS_STRING, strlen(DEPENDS_STRING));
        if(ret == 0){
             //get keyword
            ptr = ptr+ strlen(DEPENDS_STRING);
            if(strlen(ptr)==0){
                sprintf(write_content,"%s\n\n",write_content);
                return 0;
            }
            //follow busybox rule
            replace_char(ptr,',',' ',strlen(ptr));
            replace_char(ptr,'-','_',strlen(ptr));
            sprintf(write_content,"%s\n%s\n\n",write_content,ptr);
            return 0;
        }
        else{
            ptr++;
        }
        len = len - (ptr-old_ptr);
        old_ptr = ptr;
    }
    return 0;


}

int parse_symbol_string(char *write_content, char* image_content, int len){

    char *ptr =image_content;
    char *old_ptr=image_content;
    int ret=0;
    if(image_content == NULL){
        printf("parse_symbol_string: image_content NULL fail\n");
        return -1;
    }
    if(write_content ==NULL){
        printf("parse_symbol_string: write_content=NULL fail\n");
        return -1;
    }
    //if last len not enough, no need check
    if(len <= (int)strlen(KSYMTAB_STRING)){
        return 0;
    }

    len = len -strlen(KSYMTAB_STRING);
    while(len >0){
        ptr = memchr(ptr,KSYMTAB_STRING[0], len);
        if(ptr == NULL){
            //can not fild alias first word '_', no need check more
            return 0;
        }
        ret = memcmp(ptr, KSYMTAB_STRING, strlen(KSYMTAB_STRING));
        if(ret == 0){
            if(!memcmp(ptr, KSYMTAB_GPL_STRING, strlen(KSYMTAB_GPL_STRING)) || !memcmp(ptr, KSYMTAB_STRINGS_STRING, strlen(KSYMTAB_STRINGS_STRING))){
                // no need to do , follow busybox rule
                ptr++;
                len = len - (ptr-old_ptr);
                old_ptr = ptr;
        
                continue;
            }

            //get keyword
            ptr = ptr+ strlen(KSYMTAB_STRING);
            //follow busybox rule
            replace_char(ptr,'-','_',strlen(ptr));
            sprintf(write_content,"%s symbol:%s",write_content,ptr);
        }
        else{
            ptr++;
        }
        len = len - (ptr-old_ptr);
        old_ptr = ptr;
    }
    return 0;

}


int parse_alias_string(char *write_content, char* image_content, int len){

    char *ptr =image_content;
    char *old_ptr=image_content;
    int ret=0;
    if(image_content == NULL){
        printf("parse_alias_string: image_content NULL fail\n");
        return -1;
    }
    if(write_content ==NULL){
        printf("parse_alias_string: write_content=NULL fail\n");
        return -1;
    }
    //if last len not enough, no need check
    if(len <= (int)strlen(ALIAS_STRING)){
        return 0;
    }
    
    len = len -strlen(ALIAS_STRING);
    while(len >0){
        ptr = memchr(ptr,ALIAS_STRING[0], len);
        if(ptr == NULL){
            //can not fild alias first word 'a', no need check more
            return 0;
        }
        ret = memcmp(ptr, ALIAS_STRING, strlen(ALIAS_STRING));
        if(ret == 0){
            //get keyword
            ptr = ptr+ strlen(ALIAS_STRING);
            //follow busybox rule
            replace_char(ptr,'-','_',strlen(ptr));
            sprintf(write_content,"%s %s",write_content,ptr);
        }
        else{
            ptr++;
        }
        len = len - (ptr-old_ptr);
        old_ptr = ptr;
    }
    return 0;

}


int read_file_content(char *file_path, char *output_buf){

    FILE *fp=NULL;
    int file_len =0;
    int output_len=0;
    
    if(output_buf ==NULL){
        printf("read_file_content: output_buf null fail\n");
        return -1;
    }   
    if(file_path==NULL){
        printf("read_file_content: file_path null fail\n");
        return -1;
    }
    fp= fopen(file_path,"rb");
    if (fp == NULL) {
        printf("read_file_content: fopen fail\n");
        return -1;
    }
    if(fp ==NULL){
        printf("read_file_content: fp null fail\n");
        return -1;
    }
    fseek(fp,0L,SEEK_END);
    file_len = ftell(fp);
    fseek(fp,0L,SEEK_SET);
    if(file_len < MAX_SEARCH_KEYWORD_LEN){
        output_len = file_len;
    }
    else{
        output_len = MAX_SEARCH_KEYWORD_LEN;
    }
    output_len = fread(output_buf,1,output_len,fp);
    fclose(fp);
    return output_len;
}

int write_to_output_file(char *output_name,char *write_content){


    FILE *fp=NULL;

    char file_path[MAX_FILE_LEN]={0};
    
    
    strcat(file_path,module_tree_root);
    strcat(file_path,"/");
    strcat(file_path,output_name);
    fp = fopen(file_path,"a");
    if(fp==NULL){
        printf("write_to_output_file: fopen fail\r\n");
        return -1;
    }
    fprintf(fp,"%s",write_content);
    fclose(fp);
    return 0;
}


int parse_file_content(char *file_path, char *output_name){

    char *write_content = malloc(MAX_WRITE_CONTENT_LEN);
    int image_len =0;
    char *ptr=NULL;

    if(write_content == NULL){
        printf("write_content NULL \r\n");
        return -1;
    }
    if(file_path == NULL){
        printf("parse_file_content: file_path == null fail\n");
        free(write_content);
        return -1;
    }
    if(output_name==NULL){

        printf("parse_file_content: output_name = NULL fail\n");
        free(write_content);
        return -1;
    }
    //get data from ko image
    memset(image_content,0,sizeof(image_content));
    image_len = read_file_content(file_path,image_content);
    if(image_len <=0){
        printf("read_file_content fail  image_len:%d\n",image_len);
        free(write_content);
        return -1;
    }
    sprintf(write_content,"%s",file_path);
    //parsing alias
    parse_alias_string(write_content, image_content, image_len);
    //parsing symbol
    parse_symbol_string(write_content, image_content, image_len);
    
    //parsing depends
    parse_depneds_string(write_content,image_content,image_len);
    //remove module root path
    ptr = &write_content[strlen(module_tree_root)+1];  //+1 for remove '/' 
    //printf("%s\n",ptr);
    //write to file
    write_to_output_file(output_name,ptr);
    free(write_content);
    return 0;

}

int parse_ko_file(char *file_path,char * output_name){

    int ret =0;
    if(file_path == NULL){
        printf("parse_ko_file: file_path == null fail\n");
        return -1;
    }
    if(strlen(file_path)>=MAX_FILE_LEN || strlen(file_path)<=0){
        printf("file path error, path:%s  size:%lu\n",file_path,strlen(file_path));
        return -1;
    }
    if( strlen(file_path)>3){
        if((file_path[strlen(file_path)-1]=='O' || file_path[strlen(file_path)-1]=='o') &&
            (file_path[strlen(file_path)-2]=='K' || file_path[strlen(file_path)-2]=='k') &&
            file_path[strlen(file_path)-3]=='.'){

         //   printf("%s\n",file_path);
            ret = parse_file_content(file_path, output_name);
            if(ret < 0){
                printf("parse_file_content fail\n");
                return -1;
            }
        
        }
    }
    return 0;
}


int parsing_file_recursive(char *path,char * output_name){

    struct stat s_buf;
    int ret=0;
    stat(path,&s_buf);
    if(S_ISDIR(s_buf.st_mode)){
        //is folder
        struct dirent *filename;
        DIR *dp = opendir(path);
        if(dp==NULL){
            printf("opendir fail path:%s\n",path);
            return -1;
        }
        while((filename = readdir(dp))!= NULL){
            char file_path[MAX_FILE_LEN];
            memset(file_path,0x00,sizeof(file_path));
            strcat(file_path,path);
            strcat(file_path,"/");
            if(strcmp(filename->d_name,".") == 0 || strcmp(filename->d_name,"..") == 0){
                continue;
            }
            strcat(file_path,filename->d_name);
            lstat(file_path,&s_buf);
            if(S_ISLNK(s_buf.st_mode)){
                //is symbolic link
                continue;
            }
            else if(S_ISDIR(s_buf.st_mode)){
                ret = parsing_file_recursive(file_path, output_name);
                if(ret < 0){
                    printf("parsing_file_recursive fail file_path:%s\n",file_path);
                    closedir(dp);
                    return ret;
                }
            }
            else if(S_ISREG(s_buf.st_mode)){
                ret = parse_ko_file(file_path, output_name);
                if(ret < 0){
                    printf("parse_ko_file fail file_path:%s\n",file_path);
                    closedir(dp);
                    return ret;
                }
            }
            else{
                printf("recursive file check error\n");
                closedir(dp);
                return -1;
            }
        }
        closedir(dp);
    }
    else if(S_ISREG(s_buf.st_mode)){
        //is file
        ret = parse_ko_file(path, output_name);
        if(ret < 0){
            printf("parse_ko_file fail path:%s\n",path);
            return ret;
        }
    }
    else{
        printf("check file error!\n");
        return -1;
    }

    return 0;
}

int reset_output_file(char *output_name){


    char path[MAX_FILE_LEN]={0};

    FILE *fp=NULL;

    strcat(path,module_tree_root);
    strcat(path,"/");
    strcat(path,output_name);
    printf("create output file:%s\n",path);
    fp = fopen(path,"w");
    if(fp==NULL){
        printf("reset_output_file: fopen fail\n");
        printf("Error: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    fclose(fp);
    return 0;
}


int main(int argc, char* argv[]){

    char output_name[MAX_FILE_LEN]={0};
    int ret=0;
    if(argc >=2){
        sprintf(module_tree_root,"%s",argv[1]);
        if(module_tree_root[strlen(module_tree_root)-1]=='/'){
            //remove '/',if last word is '/' 
            module_tree_root[strlen(module_tree_root)-1]='\0';
        }
    }
    else{
        sprintf(module_tree_root,".");
    }

    if(argc >=3){
        sprintf(output_name,"%s",argv[2]);
    }
    else{
        sprintf(output_name,"modules.dep.bb");
    }
    //reset output file
    ret = reset_output_file(output_name);
    if(ret < 0){
        printf("reset_output_file fail\r\n");
        return 0;
    }
    printf("ready to parsing ko\n");
    parsing_file_recursive(module_tree_root,output_name);
    
    printf("finish!!\r\n");
    return 0;
}
