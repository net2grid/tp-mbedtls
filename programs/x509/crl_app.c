/*
 *  CRL reading application
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

#include "mbedtls/build_info.h"

#include "mbedtls/platform.h"

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_RSA_C) ||  \
    !defined(MBEDTLS_X509_CRL_PARSE_C) || !defined(MBEDTLS_FS_IO) || \
    defined(MBEDTLS_X509_REMOVE_INFO)
int main(void)
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_RSA_C and/or "
                   "MBEDTLS_X509_CRL_PARSE_C and/or MBEDTLS_FS_IO not defined and/or "
                   "MBEDTLS_X509_REMOVE_INFO defined.\n");
    mbedtls_exit(0);
}
#else

#include "mbedtls/x509_crl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DFL_FILENAME            "crl.pem"
#define DFL_DEBUG_LEVEL         0

#define USAGE \
    "\n usage: crl_app param=<>...\n"                   \
    "\n acceptable parameters:\n"                       \
    "    filename=%%s         default: crl.pem\n"      \
    "\n"


/*
 * global options
 */
struct options {
    const char *filename;       /* filename of the certificate file     */
} opt;

int main(int argc, char *argv[])
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    unsigned char buf[100000];
    mbedtls_x509_crl crl;
    int i;
    char *p, *q;

    /*
     * Set to sane values
     */
    mbedtls_x509_crl_init(&crl);

#if defined(MBEDTLS_USE_PSA_CRYPTO)
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        mbedtls_fprintf(stderr, "Failed to initialize PSA Crypto implementation: %d\n",
                        (int) status);
        goto exit;
    }
#endif /* MBEDTLS_USE_PSA_CRYPTO */

    if (argc < 2) {
usage:
        mbedtls_printf(USAGE);
        goto exit;
    }

    opt.filename            = DFL_FILENAME;

    for (i = 1; i < argc; i++) {
        p = argv[i];
        if ((q = strchr(p, '=')) == NULL) {
            goto usage;
        }
        *q++ = '\0';

        if (strcmp(p, "filename") == 0) {
            opt.filename = q;
        } else {
            goto usage;
        }
    }

    /*
     * 1.1. Load the CRL
     */
    mbedtls_printf("\n  . Loading the CRL ...");
    fflush(stdout);

    ret = mbedtls_x509_crl_parse_file(&crl, opt.filename);

    if (ret != 0) {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crl_parse_file returned %d\n\n", ret);
        mbedtls_x509_crl_free(&crl);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /*
     * 1.2 Print the CRL
     */
    mbedtls_printf("  . CRL information    ...\n");
    ret = mbedtls_x509_crl_info((char *) buf, sizeof(buf) - 1, "      ", &crl);
    if (ret == -1) {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crl_info returned %d\n\n", ret);
        mbedtls_x509_crl_free(&crl);
        goto exit;
    }

    mbedtls_printf("%s\n", buf);

    exit_code = MBEDTLS_EXIT_SUCCESS;

exit:
    mbedtls_x509_crl_free(&crl);
#if defined(MBEDTLS_USE_PSA_CRYPTO)
    mbedtls_psa_crypto_free();
#endif /* MBEDTLS_USE_PSA_CRYPTO */

    mbedtls_exit(exit_code);
}
#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_RSA_C && MBEDTLS_X509_CRL_PARSE_C &&
          MBEDTLS_FS_IO */
