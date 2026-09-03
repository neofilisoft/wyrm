#include "wyrm_str.h"

Value val_split(Value s, Value sep) {
    if (s.type != VAL_STRING || sep.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: split() arguments must be strings\n");
        exit(1);
    }
    const char *str = s.as.string;
    const char *delim = sep.as.string;
    size_t delim_len = strlen(delim);
    
    int count = 0;
    if (delim_len == 0) {
        count = (int)strlen(str);
    } else {
        const char *temp = str;
        while ((temp = strstr(temp, delim)) != NULL) {
            count++;
            temp += delim_len;
        }
        count++;
    }
    
    Value arr = val_array_create(count);
    if (delim_len == 0) {
        for (int i = 0; i < count; i++) {
            char buf[2] = { str[i], '\0' };
            arr.as.array->data[i] = val_string(buf);
        }
    } else {
        const char *start = str;
        const char *match;
        int idx = 0;
        while ((match = strstr(start, delim)) != NULL) {
            size_t part_len = (size_t)(match - start);
            char *buf = malloc(part_len + 1);
            wyrm_check_oom(buf, "val_split (split substring buffer)");
            memcpy(buf, start, part_len);
            buf[part_len] = '\0';
            arr.as.array->data[idx++] = val_string(buf);
            free(buf);
            start = match + delim_len;
        }
        arr.as.array->data[idx] = val_string(start);
    }
    return arr;
}

Value val_join(Value sep, Value lst) {
    if (sep.type != VAL_STRING || lst.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: join() arguments must be separator string and array\n");
        exit(1);
    }
    int count = lst.as.array->size;
    if (count == 0) {
        return val_string("");
    }
    
    char **strs = malloc((size_t)count * sizeof(char*));
    wyrm_check_oom(strs, "val_join (temp strings array)");
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        strs[i] = val_to_str_ptr(lst.as.array->data[i]);
        total_len += strlen(strs[i]);
    }
    
    size_t sep_len = strlen(sep.as.string);
    total_len += sep_len * (size_t)(count - 1);
    
    char *res = malloc(total_len + 1);
    wyrm_check_oom(res, "val_join (joined result string)");
    res[0] = '\0';
    
    for (int i = 0; i < count; i++) {
        strcat(res, strs[i]);
        free(strs[i]);
        if (i < count - 1) {
            strcat(res, sep.as.string);
        }
    }
    free(strs);
    
    Value val = val_string(res);
    free(res);
    return val;
}

Value val_trim(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: trim() argument must be a string\n");
        exit(1);
    }
    const char *start = s.as.string;
    while (*start && ((unsigned char)*start <= ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }
    size_t len = strlen(start);
    while (len > 0 && ((unsigned char)start[len - 1] <= ' ' || start[len - 1] == '\t' || start[len - 1] == '\n' || start[len - 1] == '\r')) {
        len--;
    }
    char *buf = malloc(len + 1);
    wyrm_check_oom(buf, "val_trim (trim result buffer)");
    memcpy(buf, start, len);
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_upper(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: upper() argument must be a string\n");
        exit(1);
    }
    size_t len = strlen(s.as.string);
    char *buf = malloc(len + 1);
    wyrm_check_oom(buf, "val_upper (upper result buffer)");
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s.as.string[i];
        if (c >= 'a' && c <= 'z') {
            buf[i] = (char)(c - 32);
        } else {
            buf[i] = (char)c;
        }
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_lower(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: lower() argument must be a string\n");
        exit(1);
    }
    size_t len = strlen(s.as.string);
    char *buf = malloc(len + 1);
    wyrm_check_oom(buf, "val_lower (lower result buffer)");
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s.as.string[i];
        if (c >= 'A' && c <= 'Z') {
            buf[i] = (char)(c + 32);
        } else {
            buf[i] = (char)c;
        }
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_contains(Value s, Value sub) {
    if (s.type != VAL_STRING || sub.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: contains() arguments must be strings\n");
        exit(1);
    }
    return val_bool(strstr(s.as.string, sub.as.string) != NULL);
}

Value val_replace(Value s, Value old, Value sub_new) {
    if (s.type != VAL_STRING || old.type != VAL_STRING || sub_new.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: replace() arguments must be strings\n");
        exit(1);
    }
    const char *str = s.as.string;
    const char *old_str = old.as.string;
    const char *new_str = sub_new.as.string;
    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    
    if (old_len == 0) {
        return val_string(str);
    }
    
    int count = 0;
    const char *temp = str;
    while ((temp = strstr(temp, old_str)) != NULL) {
        count++;
        temp += old_len;
    }
    
    // Use careful arithmetic to avoid unsigned underflow when new_len < old_len
    size_t base_len = strlen(str);
    size_t total_len;
    if (new_len >= old_len) {
        total_len = base_len + (new_len - old_len) * (size_t)count;
    } else {
        total_len = base_len - (old_len - new_len) * (size_t)count;
    }
    char *res = malloc(total_len + 1);
    wyrm_check_oom(res, "val_replace (replace result buffer)");
    
    const char *start = str;
    const char *match;
    char *dest = res;
    while ((match = strstr(start, old_str)) != NULL) {
        size_t copy_len = (size_t)(match - start);
        memcpy(dest, start, copy_len);
        dest += copy_len;
        memcpy(dest, new_str, new_len);
        dest += new_len;
        start = match + old_len;
    }
    strcpy(dest, start);
    
    Value val = val_string(res);
    free(res);
    return val;
}

Value val_starts_with(Value s, Value prefix) {
    if (s.type != VAL_STRING || prefix.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: starts_with() arguments must be strings\n");
        exit(1);
    }
    size_t prefix_len = strlen(prefix.as.string);
    if (strlen(s.as.string) < prefix_len) {
        return val_bool(false);
    }
    return val_bool(strncmp(s.as.string, prefix.as.string, prefix_len) == 0);
}

Value val_ends_with(Value s, Value suffix) {
    if (s.type != VAL_STRING || suffix.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: ends_with() arguments must be strings\n");
        exit(1);
    }
    size_t s_len = strlen(s.as.string);
    size_t suffix_len = strlen(suffix.as.string);
    if (s_len < suffix_len) {
        return val_bool(false);
    }
    return val_bool(strcmp(s.as.string + s_len - suffix_len, suffix.as.string) == 0);
}

Value val_char_at(Value s, Value idx) {
    if (s.type != VAL_STRING || idx.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: char_at() arguments must be string and index\n");
        exit(1);
    }
    int len = (int)strlen(s.as.string);
    int i = (int)idx.as.number;
    if (i < 0) i += len;
    if (i < 0 || i >= len) {
        fprintf(stderr, "Runtime Error: char_at() index out of bounds: %d\n", i);
        exit(1);
    }
    char buf[2] = { s.as.string[i], '\0' };
    return val_string(buf);
}

Value val_ord_val(Value c) {
    if (c.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: ord_val() argument must be a string\n");
        exit(1);
    }
    if (strlen(c.as.string) != 1) {
        fprintf(stderr, "Runtime Error: ord_val() argument must be a single character\n");
        exit(1);
    }
    return val_number((double)((unsigned char)c.as.string[0]));
}

Value val_chr_val(Value n) {
    if (n.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: chr_val() argument must be a number\n");
        exit(1);
    }
    int val = (int)n.as.number;
    if (val < 0 || val > 255) {
        fprintf(stderr, "Runtime Error: chr_val() argument out of valid single-byte range: %d\n", val);
        exit(1);
    }
    char buf[2] = { (char)val, '\0' };
    return val_string(buf);
}

Value val_to_bytes(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: to_bytes() argument must be a string\n");
        exit(1);
    }
    int len = (int)strlen(s.as.string);
    Value arr = val_array_create(len);
    for (int i = 0; i < len; i++) {
        arr.as.array->data[i] = val_number((double)((unsigned char)s.as.string[i]));
    }
    return arr;
}

Value val_from_bytes(Value lst) {
    if (lst.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: from_bytes() argument must be an array\n");
        exit(1);
    }
    int len = lst.as.array->size;
    char *buf = malloc((size_t)len + 1);
    wyrm_check_oom(buf, "val_from_bytes (byte construction buffer)");
    for (int i = 0; i < len; i++) {
        Value val = lst.as.array->data[i];
        if (val.type != VAL_NUMBER) {
            fprintf(stderr, "Runtime Error: from_bytes() array element must be a number\n");
            free(buf);
            exit(1);
        }
        int code = (int)val.as.number;
        if (code < 0 || code > 255) {
            fprintf(stderr, "Runtime Error: from_bytes() byte value out of range: %d\n", code);
            free(buf);
            exit(1);
        }
        buf[i] = (char)code;
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

