#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unicode/ustring.h>
#include <unicode/utrans.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>

void slugify(const char *input, char *output, size_t output_size, char separator) {
    UErrorCode status = U_ZERO_ERROR;
    
    if (!input || !output || output_size == 0) {
        if (output) output[0] = '\0';
        return;
    }
    
    if (separator == 0) separator = '-';
    
    int32_t input_len = strlen(input);
    UChar *unicode = (UChar*)malloc((input_len + 1) * sizeof(UChar));
    u_strFromUTF8(unicode, input_len + 1, NULL, input, -1, &status);
    
    if (U_FAILURE(status)) {
        output[0] = '\0';
        free(unicode);
        return;
    }
    
    UTransliterator *trans = utrans_open(
        "NFD; [:Nonspacing Mark:] Remove; NFC; Any-Latin; Latin-ASCII;",
        UTRANS_FORWARD, NULL, 0, NULL, &status
    );
    
    if (U_FAILURE(status)) {
        output[0] = '\0';
        free(unicode);
        return;
    }
    
    int32_t len = u_strlen(unicode);
    int32_t max_len = input_len * 6 + 10;
    UChar *translit = (UChar*)malloc(max_len * sizeof(UChar));
    u_strcpy(translit, unicode);
    
    int32_t start = 0;
    int32_t limit = len;
    utrans_transUChars(trans, translit, &len, max_len, start, &limit, &status);
    
    bool prev_sep = false;
    int32_t out_idx = 0;
    
    for (int32_t i = 0; i < len && out_idx < (int32_t)output_size - 1; i++) {
        UChar c = u_tolower(translit[i]);
        
        if (u_isalnum(c)) {
            char temp[8];
            int32_t temp_len = 0;
            u_strToUTF8(temp, sizeof(temp), &temp_len, &c, 1, &status);
            if (U_SUCCESS(status) && out_idx + temp_len < (int32_t)output_size - 1) {
                strncpy(output + out_idx, temp, temp_len);
                out_idx += temp_len;
                prev_sep = false;
            }
        } else if (!prev_sep && out_idx > 0) {
            output[out_idx++] = separator;
            prev_sep = true;
        }
    }
    
    if (out_idx > 0 && output[out_idx - 1] == separator) {
        out_idx--;
    }
    
    output[out_idx] = '\0';
    
    utrans_close(trans);
    free(unicode);
    free(translit);
}

void slugify_default(const char *input, char *output, size_t output_size) {
    slugify(input, output, output_size, '-');
}