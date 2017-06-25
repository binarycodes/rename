#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<sys/stat.h>
#include<dirent.h>
#include<string.h>
#include<errno.h>
#include<libgen.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdarg.h>

int debug_mode=0;
int quiet_mode=1;


int show_params(int, char**);
void process_file(char*);
void rename_file(char*);
char* join_file_path(char*, char*);
void convert_file_name(char*);
void log_debug(FILE*, char*, ...);
void log_info(char*, ...);
void show_help(void);

int main(int argc, char** argv) {
    int status=0;
    char c;

    while ((c = getopt (argc, argv, "dvh")) != -1){
        switch (c) {
        case 'd':
            debug_mode=1;
            break;
        case 'v':
            quiet_mode=0;
            break;
        case 'h':
            show_help();
            return 0;
        }
    }

    log_debug(stderr,"\n----------------DEBUG PARAMS START----------------\n");
    log_debug(stderr,"quiet_mode :: %d\n",quiet_mode);
    log_debug(stderr,"debug_mode :: %d\n",debug_mode);
    log_debug(stderr,"optind :: %d\n",optind);
    log_debug(stderr,"argc   :: %d\n",argc);
    log_debug(stderr,"\n----------------DEBUG PARAMS END----------------\n");

    if(debug_mode){
        show_params(argc,argv);
    }

    if (optind<argc) {
        for (int ii=optind;ii<argc;ii++) {
            char actualpath [PATH_MAX];
            realpath(argv[ii],actualpath);
            log_debug(stderr,"processing file ... %s\n",actualpath);
            process_file(actualpath);
        }
    }else{
        printf("too few arguments\n");
        status=1;
    }
    return status;
}


void process_file(char* file_path){
    struct stat stat_buffer;
    DIR* directory;
    struct dirent* entry;

    if(access(file_path, F_OK ) != 0 ) {
        fprintf(stderr,">> %s >> does not exist >> %s\n",file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        stat(file_path, &stat_buffer);
        if (S_ISDIR(stat_buffer.st_mode)){
            log_debug(stderr,">> %s >> directory\n",file_path);

            rename_file(file_path);
            log_debug(stderr,">> after dir rename >> %s\n",file_path);

            directory=opendir(file_path);
            if(!directory){
                fprintf(stderr,">> %s >> cannot open directory >> %s\n",file_path,strerror(errno));
                exit(EXIT_FAILURE);
            }

            while((entry=readdir(directory))){
                if (strcmp (entry->d_name, "..") != 0 && strcmp (entry->d_name, ".") != 0){
                    char* new_file_path=join_file_path(file_path,entry->d_name);
                    log_debug(stderr,">> new file path >> %s\n",new_file_path);
                    process_file(new_file_path);
                    free(new_file_path);
                }
            }

            if (closedir(directory)) {
                fprintf(stderr, ">> %s >> could not close >> %s\n", file_path, strerror (errno));
                exit(EXIT_FAILURE);
            }
        }else{
            log_debug(stderr,">> %s >> file\n",file_path);
            rename_file(file_path);
        }
    }
}


void rename_file(char* file_path){
    log_info(">> renaming >> %s\n",file_path);
    char* old_file_path=strdup(file_path);
    char* file_name=basename(old_file_path);
    convert_file_name(file_name);
    if (strcmp (file_name, "..") != 0 && strcmp (file_name, ".") != 0 && file_name != '\0'){
        char* new_file_path=join_file_path(dirname(old_file_path),file_name);
        log_info(">> from %s >> to %s\n",file_path,new_file_path);

        if(strcmp(file_path,new_file_path)==0){
            log_debug(stderr,">> nothing to be done >> %s\n",file_path);
        }else{
            if(access(new_file_path, F_OK ) != 0 ) {
                int status=rename(file_path,new_file_path);
                if(status==0){
                    strcpy(file_path,new_file_path);
                    log_debug(stderr,">> successfully renamed >> %s\n",file_path);
                }else{
                    fprintf(stderr,">> renaming of >> %s >> to >> %s failed - %s\n",file_path,new_file_path,strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }else{
                fprintf(stderr,">> not renaming >> %s >> to >> %s >> already exists\n", file_path,new_file_path);
            }
        }
        free(new_file_path);
    }
    log_info("done\n\n\n");
    free(old_file_path);
}


void convert_file_name(char* file_name){
    char *new_name, *reference;
    new_name=reference=file_name;
    bool dot_found=false;
    while (*file_name != '\0') {
        *new_name = tolower((unsigned char) *file_name);

        int char_point=(int)*file_name;
        if(!isalnum(char_point) && char_point>0 && char_point<128){
            if(new_name!=reference){
                *new_name='.';
                if(!dot_found){
                    dot_found=true;
                    ++new_name;
                }
            }
        }else{
            dot_found=false;
            ++new_name;
        }
        ++file_name;
    }
    if(*(new_name-1)=='.'){
        --new_name;
    }
    *new_name='\0';
    file_name=reference;
    log_debug(stderr,">> file_name >> %s\n",file_name);
    log_debug(stderr,">> new_name >> %s\n",reference);
}

int show_params(int argc, char** argv) {
    for (int ii=0;ii<argc;ii++){
        printf("param at %d is %s\n",ii,argv[ii]);
    }
    printf("\n\n");
    return 0;
}


char* join_file_path(char* prefix,char* suffix){
    char* new_string;
    int new_string_length=0;
    // adding 2 - 1 for null terminator and 1 for the path separator
    new_string_length=strlen(prefix)+strlen(suffix)+2;

    log_debug(stderr,">> prefix >> %s - %d\n",prefix,strlen(prefix));
    log_debug(stderr,">> suffix >> %s - %d\n",suffix,strlen(suffix));
    log_debug(stderr,">> length >> %d\n",sizeof(new_string));

    new_string=malloc(new_string_length * sizeof(char));
    snprintf(new_string,new_string_length,"%s/%s",prefix,suffix);
    log_debug(stderr,">> new string >> %s\n",new_string);
    return new_string;
}

void log_debug(FILE* fp, char* format, ...){
    va_list valist;

    if(debug_mode){
        va_start(valist, format);
        vfprintf(fp,format,valist);
        va_end(valist);
    }
}

void log_info(char* format, ...){
    va_list valist;

    if(!quiet_mode){
        va_start(valist, format);
        vfprintf(stdout,format,valist);
        va_end(valist);
    }
}

void show_help(void){
    printf("help");
}
