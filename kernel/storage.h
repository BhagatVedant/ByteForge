#ifndef STORAGE_H
#define STORAGE_H

void storage_init(void);
void storage_devices(void);
void storage_mount(const char *device_name);
void storage_unmount(void);
void storage_list_files(void);
void storage_open_file(const char *file_name);
void storage_cat_file(const char *file_name);
void storage_status(void);
void storage_info_file(const char *file_name);

#endif