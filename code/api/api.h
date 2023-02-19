#ifndef __API__
    #define __API__
    void instruction_help();
    void format();
    void init_system();
    char *get_pwd();
    int make_dir(char *dirname);
    int remove_dir(char *dirname);
    void list_directory_contents();
    int cd_dir(char *dirname);
    int create_file(char *filename);
    int delete_file(char *filename);
    int open_file(char *filename);
    int close_file(int fd);
    int write_file(int fd,int *len,char wstyle);
    int read_file(int fd,int *len);
    void exit_system();
#endif
