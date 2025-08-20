#include <stdio.h> 

#include <string.h> 

#include <stdlib.h> 

#include <ctype.h> 

#define MAX_LINE_LENGTH 1024  

#define MAX_SYMBOL_LENGTH 256 

#define MAX_FILENAME_LENGTH 256 

void remove_newline(char *str) { 

    str[strcspn(str, "\n")] = 0; 

} 

int is_c_identifier_char(char c) { 

    return isalnum(c) || c == '_'; 

} 

int is_c_identifier_start_char(char c) { 

    return isalpha(c) || c == '_'; 

} 

int count_lines(const char *filename) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file to count lines"); 

        return -1; 

    } 

    int count = 0; 

    int c; 

    while ((c = fgetc(file_ptr)) != EOF) { 

        if (c == '\n') { 

            count++; 

        } 

    } 

    if (count > 0 && fseek(file_ptr, -1, SEEK_END) == 0 && fgetc(file_ptr) != '\n') { 

         count++; 

    } else if (count == 0 && fseek(file_ptr, 0, SEEK_END) == 0 && ftell(file_ptr) > 0) { 

        count = 1; 

    } 

    fclose(file_ptr); 

    return count; 

} 

void find_c_symbol(const char *filename, const char *symbol) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file"); 

        return; 

    } 

    char line_content[MAX_LINE_LENGTH]; 

    int line_number = 0; 

    int found_count = 0; 

    size_t symbol_len = strlen(symbol); 

    printf("\nSearching for symbol '%s' in '%s'...\n", symbol, filename); 

    printf("--------------------------------------------------\n"); 

    while (fgets(line_content, sizeof(line_content), file_ptr) != NULL) { 

        line_number++; 

        int in_single_quote = 0; 

        int in_double_quote = 0; 

        int in_block_comment = 0;  

        int in_line_comment = 0;   

        for (int i = 0; line_content[i] != '\0'; i++) { 

            if (line_content[i] == '/' && line_content[i+1] == '*' && !in_single_quote && !in_double_quote) { 

                in_block_comment = 1; 

                i++; 

                continue; 

            } 

            if (line_content[i] == '*' && line_content[i+1] == '/' && in_block_comment) { 

                in_block_comment = 0; 

                i++; 

                continue; 

            } 

            if (line_content[i] == '/' && line_content[i+1] == '/' && !in_single_quote && !in_double_quote) { 

                in_line_comment = 1; 

                break;  

            } 

            if (line_content[i] == '\'' && !in_double_quote && !in_block_comment && !in_line_comment) { 

                in_single_quote = !in_single_quote; 

            } 

            if (line_content[i] == '"' && !in_single_quote && !in_block_comment && !in_line_comment) { 

                in_double_quote = !in_double_quote; 

            } 

            if (in_block_comment || in_line_comment || in_single_quote || in_double_quote) { 

                continue; 

            } 

            if (i + symbol_len <= strlen(line_content) && 

                strncmp(&line_content[i], symbol, symbol_len) == 0) { 

                int left_boundary_ok = (i == 0) || !is_c_identifier_char(line_content[i-1]); 

                int right_boundary_ok = (i + symbol_len == strlen(line_content) || line_content[i + symbol_len] == '\n' || !is_c_identifier_char(line_content[i + symbol_len])); 

 

                if (left_boundary_ok && right_boundary_ok) { 

                    printf("Line %d: %s", line_number, line_content); 

                    found_count++; 

                } 

            } 

        } 

    } 

    fclose(file_ptr); 

    printf("--------------------------------------------------\n"); 

    if (found_count > 0) { 

        printf("Found '%s' %d time(s).\n", symbol, found_count); 

    } else { 

        printf("Symbol '%s' not found.\n", symbol); 

    } 

} 

void find_global_definition(const char *filename, const char *symbol) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file"); 

        return; 

    } 

    char line_content[MAX_LINE_LENGTH]; 

    int line_number = 0; 

    int found_count = 0; 

    size_t symbol_len = strlen(symbol); 

    printf("\nSearching for global definition of '%s' in '%s'...\n", symbol, filename); 

    printf("--------------------------------------------------\n"); 

    while (fgets(line_content, sizeof(line_content), file_ptr) != NULL) { 

        line_number++; 

        char temp_line[MAX_LINE_LENGTH]; 

        strncpy(temp_line, line_content, sizeof(temp_line) - 1); 

        temp_line[sizeof(temp_line) - 1] = '\0'; 

        remove_newline(temp_line); 

        int is_definition_candidate = 0; 

        char *first_word = strtok(temp_line, " \t("); 

        if (first_word != NULL) { 

            if (strcmp(first_word, "int") == 0 || strcmp(first_word, "void") == 0 || 

                strcmp(first_word, "char") == 0 || strcmp(first_word, "float") == 0 || 

                strcmp(first_word, "double") == 0 || strcmp(first_word, "struct") == 0 || 

                strcmp(first_word, "enum") == 0 || strcmp(first_word, "union") == 0 || 

                strcmp(first_word, "typedef") == 0 || strcmp(first_word, "#define") == 0 || 

                strcmp(first_word, "const") == 0 || strcmp(first_word, "static") == 0 || 

                strcmp(first_word, "extern") == 0 ) 

            { 

                is_definition_candidate = 1; 

            } 

        } 

        if (!is_definition_candidate) { 

            continue; 

        } 

        char *ptr = line_content; 

        while ((ptr = strstr(ptr, symbol)) != NULL) { 

            int left_boundary_ok = (ptr == line_content) || !is_c_identifier_char(*(ptr - 1)); 

            int right_boundary_ok = (*(ptr + symbol_len) == '\0' || isspace(*(ptr + symbol_len)) || 

                                     *(ptr + symbol_len) == '(' || *(ptr + symbol_len) == ';' || 

                                     *(ptr + symbol_len) == '[' || *(ptr + symbol_len) == '='); 

 

            if (left_boundary_ok && right_boundary_ok) { 

                char *open_paren = strchr(ptr + symbol_len, '('); 

                if (open_paren != NULL) { 

                    char *close_paren = strchr(open_paren, ')'); 

                    if (close_paren != NULL) { 

                        char *brace_after_paren = strchr(close_paren, '{'); 

                        if (brace_after_paren != NULL) { 

                            printf("Line %d: %s", line_number, line_content); 

                            found_count++; 

                            goto next_line_global_def;  

                        } 

                    } 

                } else if (strchr(ptr + symbol_len, '=') != NULL || strchr(ptr + symbol_len, ';') != NULL) { 

                    printf("Line %d: %s", line_number, line_content); 

                    found_count++; 

                    goto next_line_global_def; 

                } else if (strncmp(line_content, "#define", 7) == 0 && strstr(line_content, symbol) != NULL) { 

                    printf("Line %d: %s", line_number, line_content); 

                    found_count++; 

                    goto next_line_global_def; 

                } 

            } 

            ptr += symbol_len;  

        } 

        next_line_global_def:;  

    } 

    fclose(file_ptr); 

    printf("--------------------------------------------------\n"); 

    if (found_count > 0) { 

        printf("Found %d potential global definition(s) for '%s'. (Heuristic search)\n", found_count, symbol); 

    } else { 

        printf("No potential global definition for '%s' found. (Heuristic search)\n", symbol); 

    } 

} 

void find_functions_called_by(const char *filename, const char *caller_function) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file"); 

        return; 

    } 

    char line_content[MAX_LINE_LENGTH]; 

    int line_number = 0; 

    int found_count = 0; 

    char func_body_start_pattern[MAX_SYMBOL_LENGTH + 10]; 

    snprintf(func_body_start_pattern, sizeof(func_body_start_pattern), "%s(", caller_function); 

    int inside_caller_function = 0; 

    int brace_level = 0; 

    printf("\nSearching for functions called by '%s' in '%s'...\n", caller_function, filename); 

    printf("--------------------------------------------------\n"); 

    while (fgets(line_content, sizeof(line_content), file_ptr) != NULL) { 

        line_number++; 

        if (!inside_caller_function) { 

            if (strstr(line_content, func_body_start_pattern) != NULL) { 

                char *brace_pos = strchr(line_content, '{'); 

                if (brace_pos != NULL) { 

                    inside_caller_function = 1; 

                    for (char *c = line_content; *c != '\0'; c++) { 

                        if (*c == '{') brace_level++; 

                        else if (*c == '}') brace_level--; 

                    } 

                } 

            } 

            continue;  

        } 

        for (int i = 0; line_content[i] != '\0'; i++) { 

            if (line_content[i] == '{') { 

                brace_level++; 

            } else if (line_content[i] == '}') { 

                brace_level--; 

                if (inside_caller_function && brace_level == 0) { 

                    inside_caller_function = 0; 

                    break;  

                } 

            } 

        } 

        if (inside_caller_function) { 

            char *ptr = line_content; 

            while ((ptr = strstr(ptr, "(")) != NULL) { 

                char *func_name_end = ptr; 

                while (func_name_end > line_content && isspace(*(func_name_end - 1))) { 

                    func_name_end--; 

                } 

                char *func_name_start = func_name_end; 

                while (func_name_start > line_content && is_c_identifier_char(*(func_name_start - 1))) { 

                    func_name_start--; 

                } 

                int len = func_name_end - func_name_start; 

                if (len > 0 && len < MAX_SYMBOL_LENGTH) { 

                    char potential_func_name[MAX_SYMBOL_LENGTH]; 

                    strncpy(potential_func_name, func_name_start, len); 

                    potential_func_name[len] = '\0'; 

                    if (strcmp(potential_func_name, caller_function) != 0 &&  

                        strcmp(potential_func_name, "if") != 0 && 

                        strcmp(potential_func_name, "while") != 0 && 

                        strcmp(potential_func_name, "for") != 0 && 

                        strcmp(potential_func_name, "switch") != 0 && 

                        strcmp(potential_func_name, "sizeof") != 0 && 

                        strcmp(potential_func_name, "return") != 0 && 

                        strcmp(potential_func_name, "case") != 0 && 

                        strcmp(potential_func_name, "alignof") != 0 && 

                        strcmp(potential_func_name, "decltype") != 0 && 

                        strcmp(potential_func_name, "static_assert") != 0 

                        ) 

                    { 

                        if (is_c_identifier_start_char(potential_func_name[0])) { 

                            printf("  Called: %s (Line %d)\n", potential_func_name, line_number); 

                            found_count++; 

                        } 

                    } 

                } 

                ptr++;  

            } 

        } 

    } 

 

    fclose(file_ptr); 

    printf("--------------------------------------------------\n"); 

    if (found_count > 0) { 

        printf("Found %d potential function call(s) by '%s'. (Heuristic search)\n", found_count, caller_function); 

    } else { 

        printf("No potential function calls found by '%s'. (Heuristic search)\n", caller_function); 

    } 

} 

void find_functions_calling(const char *filename, const char *callee_function) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file"); 

        return; 

    } 

 

    char line_content[MAX_LINE_LENGTH]; 

    int line_number = 0; 

    int found_count = 0; 

    char search_pattern[MAX_SYMBOL_LENGTH + 5]; 

    snprintf(search_pattern, sizeof(search_pattern), "%s(", callee_function); 

    size_t search_pattern_len = strlen(search_pattern); 

 

    printf("\nSearching for functions calling '%s' in '%s'...\n", callee_function, filename); 

    printf("--------------------------------------------------\n"); 

 

    while (fgets(line_content, sizeof(line_content), file_ptr) != NULL) { 

        line_number++; 

 

        char *ptr = line_content; 

        while ((ptr = strstr(ptr, search_pattern)) != NULL) { 

            if (ptr > line_content && is_c_identifier_char(*(ptr - 1))) { 

                ptr += search_pattern_len; 

                continue; 

            } 

            char *closing_paren = strchr(ptr + search_pattern_len, ')'); 

            if (closing_paren != NULL) { 

                char *after_paren = closing_paren + 1; 

                while (*after_paren != '\0' && isspace(*after_paren)) { 

                    after_paren++; 

                } 

                if (*after_paren == '{') { 

                    ptr += search_pattern_len;  

                    continue; 

                } 

            } 

            printf("  Called at Line %d: %s", line_number, line_content); 

            found_count++; 

            ptr += search_pattern_len;  

        } 

    } 

    fclose(file_ptr); 

    printf("--------------------------------------------------\n"); 

    if (found_count > 0) { 

        printf("Found %d potential call(s) to '%s'. (Heuristic search)\n", found_count, callee_function); 

    } else { 

        printf("No potential calls to '%s' found. (Heuristic search)\n", callee_function); 

    } 

} 

void find_text_string(const char *filename, const char *text_string) { 

    FILE *file_ptr = fopen(filename, "r"); 

    if (file_ptr == NULL) { 

        perror("Error opening file"); 

        return; 

    } 

    char line_content[MAX_LINE_LENGTH];  

    int line_number = 0; 

    int found_count = 0; 

    printf("\nSearching for text string '%s' in '%s'...\n", text_string, filename); 

    printf("--------------------------------------------------\n"); 

    while (fgets(line_content, sizeof(line_content), file_ptr) != NULL) { 

        line_number++; 

        if (strstr(line_content, text_string) != NULL) { 

            printf("Line %d: %s", line_number, line_content); 

            found_count++; 

        } 

    } 

    fclose(file_ptr); 

    printf("--------------------------------------------------\n"); 

    if (found_count > 0) { 

        printf("Found '%s' %d time(s).\n", text_string, found_count); 

    } else { 

        printf("Text string '%s' not found.\n", text_string); 

    } 

} 

int main() { 

    char filename[MAX_FILENAME_LENGTH]; 

    int choice; 

    char input_symbol[MAX_SYMBOL_LENGTH]; 

    printf("--- Basic Cscope-like Tool ---\n"); 

    printf("Enter the C source file to analyze (e.g., my_program.c): "); 

    if (fgets(filename, sizeof(filename), stdin) == NULL) { 

        perror("Error reading filename"); 

        return 1; 

    } 

    remove_newline(filename); 

    FILE *test_file = fopen(filename, "r"); 

    if (test_file == NULL) { 

        perror("Error opening file. Please ensure it exists and try again."); 

        return 1; 

    } 

    fclose(test_file); 

    int total_lines = count_lines(filename); 

    if (total_lines != -1) { 

        printf("'%s' has %d lines.\n", filename, total_lines); 

    } else { 

        printf("Could not determine line count for '%s'.\n", filename); 

    } 

    do { 

        printf("\n--- Choose an operation for '%s' ---\n", filename); 

        printf("1. Find this C symbol\n"); 

        printf("2. Find this global definition (Heuristic)\n"); 

        printf("3. Find functions called by this function (Heuristic)\n"); 

        printf("4. Find functions calling this function (Heuristic)\n"); 

        printf("5. Find this text string\n"); 

        printf("0. Exit\n"); 

        printf("Enter your choice: "); 

        if (scanf("%d", &choice) != 1) { 

            printf("Invalid input. Please enter a number.\n"); 

            while (getchar() != '\n'); 

            choice = -1; 

            continue; 

        } 

        while (getchar() != '\n');  

        switch (choice) { 

            case 1: 

                printf("Enter C symbol to find: "); 

                if (fgets(input_symbol, sizeof(input_symbol), stdin) == NULL) { 

                    perror("Error reading symbol"); break; 

                } 

                remove_newline(input_symbol); 

                find_c_symbol(filename, input_symbol); 

                break; 

            case 2: 

                printf("Enter C symbol (for global definition search): "); 

                if (fgets(input_symbol, sizeof(input_symbol), stdin) == NULL) { 

                    perror("Error reading symbol"); break; 

                } 

                remove_newline(input_symbol); 

                find_global_definition(filename, input_symbol); 

                break; 

            case 3: 

                printf("Enter the CALLER function name (e.g., main): "); 

                if (fgets(input_symbol, sizeof(input_symbol), stdin) == NULL) { 

                    perror("Error reading function name"); break; 

                } 

                remove_newline(input_symbol); 

                find_functions_called_by(filename, input_symbol); 

                break; 

            case 4: 

                printf("Enter the CALLED function name (e.g., printf): "); 

                if (fgets(input_symbol, sizeof(input_symbol), stdin) == NULL) { 

                    perror("Error reading function name"); break; 

                } 

                remove_newline(input_symbol); 

                find_functions_calling(filename, input_symbol); 

                break; 

            case 5: 

                printf("Enter text string to find: "); 

                if (fgets(input_symbol, sizeof(input_symbol), stdin) == NULL) { 

                    perror("Error reading text string"); break; 

                } 

                remove_newline(input_symbol); 

                find_text_string(filename, input_symbol); 

                break; 

            case 0: 

                printf("Exiting Basic Cscope-like Tool. Goodbye!\n"); 

                break; 

            default: 

                printf("You entered an invalid choice. Please enter a number between 0 and 5.\n"); 

                break; 

        } 

    } while (choice != 0); 

 

    return 0; 

} 
