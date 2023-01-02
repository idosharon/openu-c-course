#include "command.h"

const command set_commands[] = {
    {"union_set", WRITE, 3, {&union_set}}, 
    {"intersect_set", WRITE, 3, {&intersect_set} }, 
    {"sub_set", WRITE, 3, {&sub_set} }, 
    {"symdiff_set", WRITE, 3, {&symdiff_set} }, 
    {"print_set", DEBUG, 1, {.debug=&print_set} }, 
    {"read_set", READ, 1, {.read=&read_set} }, 
    {"stop", NONE, 0, {.none=&stop} }
};

const command* to_command(char* str) {
    int i = 0;
    for (; i < LENGTH(set_commands); i++) {
        if (equal(str, set_commands[i].value)) return &set_commands[i];
    }
    return NULL;
}
set* to_set(char* str) {
    int i = 0;
    for (; i < LENGTH(sets); i++) {
        if (equal(str, sets[i].name)) return &sets[i];
    }
    return NULL;
}

static char* next_token(char** str, char delimiter) {
    static char* p;
    static char** str_p;
    char c;

    if (str != NULL) {
        str_p = str;
        p = *str_p;
    } else {
        *str_p = p;
    };

    
    for(; (c = *p++) != '\0' && c != delimiter;);

    return p;
}

static int clean_spaces(char* str) {
    char *p = str;
    char *q = str;
    int counter = 0;
    int flag = 1;
    for (; *p != '\0'; p++)
    {
        if (IS_SPACE(*p)) {
            flag = 1;
            continue;
        } else {
            *q++ = *p;
            counter += flag;
            flag = 0;
        };
    }

    *q++ = '\0';
    return counter;
}

static int contains(char* str, char c) {
    char* p = str;
    for (; *p != '\0'; p++) {
        if (*p == c) return 1;
    }
    return 0;
}

static int is_number(char* str) {
    if(*str == '-' || *str == '+') str++;

    for (; *str != '\0'; str++) {
        if (!IS_DIGIT(*str)) return 0;
    }
    return 1;
}

int exec(char* str) {
    char* p = next_token(&str, ' ');
    
    char* command_name = strndup(str, p - str - 1);

    if(contains(command_name, ',')) error("Illegal comma");

    const command* command = to_command(command_name);
    /* first check - command name */
    if (command == NULL) error("Undefined command name");

    /* create empty list of sets with size command->type_Command */
    set** sets_args = (set**) malloc(sizeof(set*));
    
    int i = 0;

    for (; i < command->num_of_sets;)
    {
        if (*p == '\0') error("Missing parameter");

        p = next_token(NULL, ',');

        char* set_str = strndup(str, p - str - 1);

        int error = clean_spaces(set_str);
        
        if (error == 0) { error("Multiple consecutive commas"); }
        else if (error > 2) error("Missing comma");
        
        set* set = to_set(set_str);
        if (set == NULL) error("Undefined set name");

        printf("set: %s\n", set_str);
        
        sets_args[i++] = set;
        sets_args = realloc(sets_args, i);
    }

    int num;
    int* arr = (int*) malloc(sizeof(int));
    int len = 0;

    if (command->command_type == READ) {
        for (;;)
        {
            p = next_token(NULL, ',');

            char* str_num = strndup(str, p - str - 1);

            int error = clean_spaces(str_num);

            if(*str_num == '\0') break;
        
            if (error == 0) { error("Multiple consecutive commas"); }
            else if (error > 2) error("Missing comma");
            
            if(!is_number(str_num)) error("Invalid set member - not an integer");
            
            num = atoi(str_num);
            
            if (num == -1) break;

            if(!IS_VALID(num)) error("Invalid set member - value out of range"); 

            arr[len++] = num;
            arr = realloc(arr, len);
        }

        if (num != -1) error("List of set members is not terminated correctly");
    };

    if (*(p-1) != '\0') error("Extraneous text after end of command");

    
    switch(command->command_type) {
        case READ:
            command->func.read(sets_args[0], arr, len);
            break;
        case WRITE:
            command->func.write(sets_args[0], sets_args[1], sets_args[2]);
            break;
        case DEBUG:
            command->func.debug(sets_args[0]);
            break;
        case NONE:
            command->func.none();
            break;
    }

    free(sets_args);
    free(arr);
    
    return 0;
}
