#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <netcdf.h>
#include <libgen.h> // for basename
#include <unistd.h> // for getopt

#define MAX_VAR_DIMS 1024
#define NC_MAX_NAME 256
#define DEFAULT_SPACING 0

void handle_err(int status, int lineno) {
    if (status != NC_NOERR) {
        fprintf(stderr, "Line %d: %s\n", lineno, nc_strerror(status));
        exit(-1);
    }
}

int main(int argc, char** argv) {
    int color = 0;
    int opt;
    int additional_spacing = DEFAULT_SPACING;

    while ((opt = getopt(argc, argv, "cs:")) != -1) {
        switch (opt) {
            case 'c':
                color = 1;
                break;
            case 's':
                additional_spacing = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-c] [-s spacing] file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    char *filename_copy = strdup(argv[optind]);
    char *base_filename = basename(filename_copy);

    int nc_file, nc_status, nc_var_count, var_id;
    size_t length;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    int var_dims[MAX_VAR_DIMS];

    // Open the file
    nc_status = nc_open(argv[optind], NC_NOWRITE, &nc_file);
    handle_err(nc_status, __LINE__);

    // Get the number of variables
    nc_status = nc_inq_nvars(nc_file, &nc_var_count);
    handle_err(nc_status, __LINE__);

    // Find the maximum length of variable names
    int max_name_length = 0;
    for (int i = 0; i < nc_var_count; i++) {
        char var_name[NC_MAX_NAME + 1];
        nc_status = nc_inq_varname(nc_file, i, var_name);
        handle_err(nc_status, __LINE__);
        int name_length = strlen(base_filename) + strlen(var_name) + 1; // +1 for the colon
        if (name_length > max_name_length) {
            max_name_length = name_length;
        }
    }

    // Loop over the variables
    for (int i = 0; i < nc_var_count; i++) {
        char var_name[NC_MAX_NAME + 1];
        // Get the variable's name
        nc_status = nc_inq_varname(nc_file, i, var_name);
        handle_err(nc_status, __LINE__);

        int ndims;
        // Get the number of dimensions of the variable
        nc_status = nc_inq_varndims(nc_file, i, &ndims);
        handle_err(nc_status, __LINE__);

        // Get the ids of the dimensions of the variable
        nc_status = nc_inq_vardimid(nc_file, i, var_dims);
        handle_err(nc_status, __LINE__);

        // Calculate the length of the variable
        length = 1;
        for (int j = 0; j < ndims; j++) {
            size_t dim_length;
            nc_status = nc_inq_dimlen(nc_file, var_dims[j], &dim_length);
            handle_err(nc_status, __LINE__);
            length *= dim_length;
        }

        // Get the variable's type
        nc_type var_type;
        nc_status = nc_inq_vartype(nc_file, i, &var_type);
        handle_err(nc_status, __LINE__);

        // Initialize the SHA-256 context
        SHA256_Init(&sha256);

        switch (var_type) {
            case NC_BYTE: {
                signed char *data = malloc(length * sizeof(signed char));
                nc_status = nc_get_var_schar(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(signed char));
                free(data);
                break;
            }
            case NC_UBYTE: {
                unsigned char *data = malloc(length * sizeof(unsigned char));
                nc_status = nc_get_var_uchar(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(unsigned char));
                free(data);
                break;
            }
            case NC_SHORT: {
                short *data = malloc(length * sizeof(short));
                nc_status = nc_get_var_short(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(short));
                free(data);
                break;
            }
            case NC_USHORT: {
                unsigned short *data = malloc(length * sizeof(unsigned short));
                nc_status = nc_get_var_ushort(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(unsigned short));
                free(data);
                break;
            }
            case NC_INT: {
                int *data = malloc(length * sizeof(int));
                nc_status = nc_get_var_int(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(int));
                free(data);
                break;
            }
            case NC_UINT: {
                unsigned int *data = malloc(length * sizeof(unsigned int));
                nc_status = nc_get_var_uint(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(unsigned int));
                free(data);
                break;
            }
            case NC_FLOAT: {
                float *data = malloc(length * sizeof(float));
                nc_status = nc_get_var_float(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(float));
                free(data);
                break;
            }
            case NC_DOUBLE: {
                double *data = malloc(length * sizeof(double));
                nc_status = nc_get_var_double(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(double));
                free(data);
                break;
            }
            case NC_INT64: {
                long long *data = malloc(length * sizeof(long long));
                nc_status = nc_get_var_longlong(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(long long));
                free(data);
                break;
            }
            case NC_UINT64: {
                unsigned long long *data = malloc(length * sizeof(unsigned long long));
                nc_status = nc_get_var_ulonglong(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(unsigned long long));
                free(data);
                break;
            }
            case NC_CHAR: {
                char *data = malloc(length * sizeof(char));
                nc_status = nc_get_var_text(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                SHA256_Update(&sha256, data, length * sizeof(char));
                free(data);
                break;
            }
            case NC_STRING: {
                char **data = malloc(length * sizeof(char *));
                nc_status = nc_get_var_string(nc_file, i, data);
                handle_err(nc_status, __LINE__);
                for (size_t j = 0; j < length; j++) {
                    SHA256_Update(&sha256, data[j], strlen(data[j]) * sizeof(char));
                }
                nc_free_string(length, data);
                free(data);
                break;
            }
            default: {
                fprintf(stderr, "Unsupported data type %d for variable %s\n", var_type, var_name);
                exit(-1);
            }
        }





        SHA256_Final(hash, &sha256);

        if (color) {
            printf("\033[31m%s\033[0m: %s%*s ", base_filename, var_name, additional_spacing + max_name_length - strlen(base_filename) - strlen(var_name), "");
        } else {
            printf("%s: %s%*s ", base_filename, var_name, additional_spacing + max_name_length - strlen(base_filename) - strlen(var_name), "");
        }

        for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
            printf("%02x", hash[j]);
        }
        printf("\n");

        SHA256_Init(&sha256); // Reset SHA256 context for the next variable

    }

    // Close the file
    nc_status = nc_close(nc_file);
    handle_err(nc_status, __LINE__);

    free(filename_copy);

    return 0;
}

