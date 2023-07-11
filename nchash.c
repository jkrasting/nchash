#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <netcdf.h>
#include <archive.h>
#include <archive_entry.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#define RED "\033[31m"
#define RESET "\033[0m"

int nc_open_mem(const char *path, int mode, size_t size, void *memory, int *ncidp);

size_t nc_type_size(nc_type type) {
    switch (type) {
        case NC_BYTE: return sizeof(signed char);
        case NC_CHAR: return sizeof(char);
        case NC_SHORT: return sizeof(short);
        case NC_INT: return sizeof(int);
        case NC_FLOAT: return sizeof(float);
        case NC_DOUBLE: return sizeof(double);
        default: return 0;
    }
}

void hash_data(int ncid, int varid, const EVP_MD *md) {
    nc_type type;
    size_t total = 1;
    int dim;
    int ndims;
    int dims[NC_MAX_VAR_DIMS];

    nc_inq_varndims(ncid, varid, &ndims);
    nc_inq_vardimid(ncid, varid, dims);
    for (dim = 0; dim < ndims; dim++) {
        size_t len;
        nc_inq_dimlen(ncid, dims[dim], &len);
        total *= len;
    }

    nc_inq_vartype(ncid, varid, &type);
    switch (type) {
        case NC_BYTE:
        case NC_CHAR:
        case NC_SHORT:
        case NC_INT:
        case NC_FLOAT:
        case NC_DOUBLE:
            {
                void *data = malloc(nc_type_size(type) * total);
                nc_get_var(ncid, varid, data);

                EVP_MD_CTX *ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, md, NULL);
                EVP_DigestUpdate(ctx, data, nc_type_size(type) * total);

                unsigned char digest[EVP_MAX_MD_SIZE];
                unsigned int digest_len;
                EVP_DigestFinal_ex(ctx, digest, &digest_len);

                for (unsigned int i = 0; i < digest_len; i++) {
                    printf("%02x", digest[i]);
                }

                printf("\n");
                free(data);
                EVP_MD_CTX_free(ctx);
            }
            break;
        default:
            fprintf(stderr, "Unsupported data type\n");
    }
}

void handle_ncfile(int ncid, const char *filename, int color_flag, const EVP_MD *md) {
    char varname[NC_MAX_NAME + 1];
    int varid;

    for (varid = 0; varid < NC_MAX_VARS; varid++) {
        if (nc_inq_varname(ncid, varid, varname) != NC_NOERR) {
            break;
        }
        if (color_flag == 1) {
            printf(RED "%s" RESET " : %s  ", basename(filename), varname);
        } else {
            printf("%s %s: ", basename(filename), varname);
        }
        hash_data(ncid, varid, md);
    }
}

void handle_entry(struct archive *a, struct archive_entry *entry, int color_flag, const EVP_MD *md) {
    const char *filename = archive_entry_pathname(entry);
    const void *buf;
    size_t size;
    off_t offset;

    if (archive_read_data_block(a, &buf, &size, &offset) != ARCHIVE_OK) {
        fprintf(stderr, "Failed to read entry data: %s\n", filename);
        return;
    }

    int ncid;
    if (nc_open_mem(filename, NC_NOWRITE, size, buf, &ncid) == NC_NOERR) {
        char *base = basename((char *)filename);
        if (color_flag == 1) {
            printf(RED "%s" RESET "\n", base);
        } else {
            printf("%s\n", base);
        }
        handle_ncfile(ncid, filename, color_flag, md);
        if (nc_close(ncid) != NC_NOERR) {
            fprintf(stderr, "Failed to close file: %s\n", filename);
        }
    }
}

void handle_file(int ncid, const char *filename, int color_flag, const EVP_MD *md) {
    int nvars;
    if (nc_inq_nvars(ncid, &nvars) != NC_NOERR) {
        fprintf(stderr, "Failed to get number of variables: %s\n", filename);
        return;
    }

    for (int i = 0; i < nvars; ++i) {
        char varname[NC_MAX_NAME + 1];
        nc_type type;
        int ndims;
        if (nc_inq_var(ncid, i, varname, &type, &ndims, NULL, NULL) != NC_NOERR) {
            fprintf(stderr, "Failed to inquire variable: %s\n", filename);
            continue;
        }

        size_t total = 1;
        int dims[ndims];
        if (nc_inq_vardimid(ncid, i, dims) != NC_NOERR) {
            fprintf(stderr, "Failed to inquire variable dimensions: %s\n", filename);
            continue;
        }

        for (int j = 0; j < ndims; ++j) {
            size_t len;
            if (nc_inq_dimlen(ncid, dims[j], &len) != NC_NOERR) {
                fprintf(stderr, "Failed to inquire dimension length: %s\n", filename);
                break;
            }
            total *= len;
        }

        void *data = malloc(nc_type_size(type) * total);
        if (!data) {
            fprintf(stderr, "Failed to allocate memory for data: %s\n", filename);
            continue;
        }

        if (nc_get_var(ncid, i, data) != NC_NOERR) {
            fprintf(stderr, "Failed to get variable data: %s\n", filename);
            free(data);
            continue;
        }

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, data, nc_type_size(type) * total);
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        EVP_DigestFinal_ex(mdctx, hash, &hash_len);
        EVP_MD_CTX_free(mdctx);

        printf("  %s: ", varname);
        for (unsigned int j = 0; j < hash_len; ++j) {
            printf("%02x", hash[j]);
        }
        printf("\n");

        free(data);
    }
}

int handle_tar(const char *filename, int color_flag, const EVP_MD *md) {
    struct archive *a;
    struct archive_entry *entry;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);

    if (archive_read_open_filename(a, filename, 10240) != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open tar file: %s\n", filename);
        archive_read_free(a);
        return 1;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        size_t total_size = archive_entry_size(entry);
        void *buffer = malloc(total_size);
        if (!buffer) {
            fprintf(stderr, "Failed to allocate memory\n");
            continue;
        }

        size_t offset = 0;
        const void *data;
        size_t size;
        off_t data_offset;

        while (archive_read_data_block(a, &data, &size, &data_offset) == ARCHIVE_OK) {
            memcpy((char *)buffer + offset, data, size);
            offset += size;
        }

        int ncid;
        if (nc_open_mem(archive_entry_pathname(entry), NC_NOWRITE, total_size, buffer, &ncid) == NC_NOERR) {
            char *base = basename((char *)archive_entry_pathname(entry));
            handle_ncfile(ncid, archive_entry_pathname(entry), color_flag, md);
            if (nc_close(ncid) != NC_NOERR) {
                fprintf(stderr, "Failed to close file: %s\n", archive_entry_pathname(entry));
            }
        }

        free(buffer);
        archive_read_data_skip(a);
    }

    archive_read_free(a);
    return 0;
}

int main(int argc, char **argv) {
    const char *filename = NULL;
    int color_flag = 0;
    const EVP_MD *md = EVP_sha256();

    int opt;
    while ((opt = getopt(argc, argv, "ch:")) != -1) {
        switch (opt) {
            case 'c':
                fprintf(stderr, "Setting color mode\n");
                color_flag = 1;
                break;
            case 'h':
                if (strcmp(optarg, "sha256") == 0) {
                    md = EVP_sha256();
                } else if (strcmp(optarg, "sha1") == 0) {
                    md = EVP_sha1();
                } else if (strcmp(optarg, "md5") == 0) {
                    md = EVP_md5();
                } else {
                    fprintf(stderr, "Unsupported hash type: %s\n", optarg);
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-c] [-h hash] file\n", argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected filename\n");
        return 1;
    }

    filename = argv[optind];

    // Initialize OpenSSL
    OpenSSL_add_all_digests();

    if (handle_tar(filename, color_flag, md) != 0) {
        int ncid;
        if (nc_open(filename, NC_NOWRITE, &ncid) == NC_NOERR) {
            handle_ncfile(ncid, filename, color_flag, md);
            if (nc_close(ncid) != NC_NOERR) {
                fprintf(stderr, "Failed to close file: %s\n", filename);
            }
        }
    }

    return 0;
}

