#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <netcdf.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#define DEFAULT_SPACING 0
#define DEFAULT_HASH_ALGORITHM '2'  // SHA256 by default
#define HANDLE_ERR(e) {if(e!=NC_NOERR){printf("Error: %s\n", nc_strerror(e)); exit(2);}}
#define MAX_NAME_LEN 1024

// Function to handle errors
void handle_err(int status, int line) {
    if (status != NC_NOERR) {
        fprintf(stderr, "line %d: %s\n", line, nc_strerror(status));
        exit(-1);
    }
}

void update_hash(char hash_algorithm, void* data, size_t data_len, SHA256_CTX* sha256, SHA_CTX* sha1, MD5_CTX* md5) {
    switch (hash_algorithm) {
        case '2':  // SHA256
            SHA256_Update(sha256, data, data_len);
            break;
        case 'm':  // MD5
            MD5_Update(md5, data, data_len);
            break;
        case 's':  // SHA1
            SHA1_Update(sha1, data, data_len);
            break;
        default:
            fprintf(stderr, "Invalid hash algorithm!\n");
            exit(-1);
    }
}

int main(int argc, char** argv) {
    int nc_status;  // Status for NetCDF operations
    int nc_file;  // NetCDF file id
    int n_vars;  // Number of variables in the file
    char hash_algorithm = DEFAULT_HASH_ALGORITHM;  // Hash algorithm ('2' for SHA256, 'm' for MD5, 's' for SHA1)
    int additional_spacing = DEFAULT_SPACING;  // Additional spacing between variable names and hashes
    int color = 0;  // Whether to colorize the output
    char* filename;  // Name of the file to process
    char* filename_copy;  // Copy of filename for use with basename()
    char* base_filename;  // Basename of the file to process
    int max_name_length = 0;  // Maximum length of a variable name

    // Parse command line options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--color") == 0 || strcmp(argv[i], "-c") == 0) {
            color = 1;
        } else if (strcmp(argv[i], "--hash") == 0 || strcmp(argv[i], "-a") == 0) {
            i++;
            if (i < argc) {
                hash_algorithm = argv[i][0];
            } else {
                fprintf(stderr, "Missing argument for --hash\n");
                exit(-1);
            }
        } else if (strcmp(argv[i], "--spacing") == 0 || strcmp(argv[i], "-s") == 0) {
            i++;
            if (i < argc) {
                additional_spacing = atoi(argv[i]);
            } else {
                fprintf(stderr, "Missing argument for --spacing\n");
                exit(-1);
            }
        } else {
            filename = argv[i];
            filename_copy = strdup(filename);
            base_filename = basename(filename_copy);
        }
    }

    // Open the file
    nc_status = nc_open(filename, NC_NOWRITE, &nc_file);
    handle_err(nc_status, __LINE__);

    // Get the number of variables
    nc_status = nc_inq_nvars(nc_file, &n_vars);
    handle_err(nc_status, __LINE__);

    // Initialize hashing contexts
    SHA256_CTX sha256;
    SHA_CTX sha1;
    MD5_CTX md5;
    SHA256_Init(&sha256);
    SHA1_Init(&sha1);
    MD5_Init(&md5);

    // Process each variable
    for (int i = 0; i < n_vars; i++) {
        // Get variable name
        char var_name[MAX_NAME_LEN];
        nc_status = nc_inq_varname(nc_file, i, var_name);
        handle_err(nc_status, __LINE__);

        // Update maximum name length
        int name_length = strlen(base_filename) + strlen(var_name) + 2;  // Add 2 for ':' and ' '
        if (name_length > max_name_length) {
            max_name_length = name_length;
        }
    }

    for (int i = 0; i < n_vars; i++) {
        // Get variable name
        char var_name[MAX_NAME_LEN];
        nc_status = nc_inq_varname(nc_file, i, var_name);
        handle_err(nc_status, __LINE__);

        // Get variable type
        nc_type var_type;
        nc_status = nc_inq_vartype(nc_file, i, &var_type);
        handle_err(nc_status, __LINE__);

        // Get total length of variable data
        int n_dims;
        nc_status = nc_inq_varndims(nc_file, i, &n_dims);
        handle_err(nc_status, __LINE__);

        int dim_ids[NC_MAX_VAR_DIMS];
        nc_status = nc_inq_vardimid(nc_file, i, dim_ids);
        handle_err(nc_status, __LINE__);

        size_t length = 1;
        for (int j = 0; j < n_dims; j++) {
            size_t dim_length;
            nc_status = nc_inq_dimlen(nc_file, dim_ids[j], &dim_length);
            handle_err(nc_status, __LINE__);

            length *= dim_length;
        }

        switch (var_type) {
            case NC_BYTE: {
                signed char* data = malloc(length * sizeof(signed char));
                nc_status = nc_get_var_schar(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(signed char), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            case NC_CHAR: {
                char* data = malloc(length * sizeof(char));
                nc_status = nc_get_var_text(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(char), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            case NC_SHORT: {
                short* data = malloc(length * sizeof(short));
                nc_status = nc_get_var_short(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(short), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            case NC_INT: {
                int* data = malloc(length * sizeof(int));
                nc_status = nc_get_var_int(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(int), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            case NC_FLOAT: {
                float* data = malloc(length * sizeof(float));
                nc_status = nc_get_var_float(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(float), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            case NC_DOUBLE: {
                double* data = malloc(length * sizeof(double));
                nc_status = nc_get_var_double(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                update_hash(hash_algorithm, data, length * sizeof(double), &sha256, &sha1, &md5);
                free(data);
                break;
            }
            default: {
                fprintf(stderr, "Unsupported variable type %d\n", var_type);
                exit(-1);
            }
        }

        // Print hash
        unsigned char hash[SHA256_DIGEST_LENGTH];
        switch (hash_algorithm) {
            case '2':  // SHA256
                SHA256_Final(hash, &sha256);
                break;
            case 'm':  // MD5
                MD5_Final(hash, &md5);
                break;
            case 's':  // SHA1
                SHA1_Final(hash, &sha1);
                break;
            default:
                fprintf(stderr, "Invalid hash algorithm!\n");
                exit(-1);
        }

        // Print filename, variable name, and hash
        if (color) {
            printf("\033[1;31m");
        }
        printf("%s:%s", base_filename, var_name);
        if (color) {
            printf("\033[0m");
        }
        printf("%*s", max_name_length + additional_spacing - strlen(base_filename) - strlen(var_name), "");
        for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
            printf("%02x", hash[j]);
        }
        printf("\n");
    }

    // Close the file
    nc_status = nc_close(nc_file);
    handle_err(nc_status, __LINE__);

    // Clean up
    free(filename_copy);

    return 0;
}

