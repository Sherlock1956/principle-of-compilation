#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_ID_LENGTH 16

typedef struct
{
    int type;
    char value[MAX_ID_LENGTH + 1];
    int line;
} Token;

FILE *source, *dyd, *err;
int line_number = 1;

void report_error(const char *message, int line)
{
    fprintf(err, "***LINE:%d  %s\n", line, message);
}



Token get_token()
{
    Token token;
    int ch = fgetc(source);

    while (isspace(ch) && ch != '\n')
    {
        ch = fgetc(source);
    }

    if (ch == '\n')
    {
        line_number++;
        Token newline_token = {24, "EOLN", line_number};
        return newline_token;
    }

    if (ch == EOF)
    {
        token.type = 25;
        strcpy(token.value, "EOF");
        token.line = line_number;
        return token;
    }

    if (isalpha(ch))
    {
        int length = 0;
        char buffer[MAX_ID_LENGTH + 1];
        while (isalnum(ch))
        {
            if (length < MAX_ID_LENGTH)
            {
                buffer[length++] = ch;
            }
            else
            {
                report_error("Identifier length overflow", line_number);
                while (isalnum(ch))
                    ch = fgetc(source);
                break;
            }
            ch = fgetc(source);
        }
        ungetc(ch, source);
        buffer[length] = '\0';
        strcpy(token.value, buffer);
        token.line = line_number;
        // Check for reserved words
        token.type = 10;
        if (strcmp(token.value, "begin") == 0)
            token.type = 1;
        else if (strcmp(token.value, "end") == 0)
            token.type = 2;
        else if (strcmp(token.value, "integer") == 0)
            token.type = 3;
        else if (strcmp(token.value, "function") == 0)
            token.type = 7;
        else if (strcmp(token.value, "read") == 0)
            token.type = 8;
        else if (strcmp(token.value, "write") == 0)
            token.type = 9;
        else if (strcmp(token.value, "if") == 0)
            token.type = 4;
        else if (strcmp(token.value, "then") == 0)
            token.type = 5;
        else if (strcmp(token.value, "else") == 0)
            token.type = 6;
    }
    else if (isdigit(ch))
    {
        int length = 0;
        char buffer[MAX_ID_LENGTH + 1];
        while (isdigit(ch))
        {
            buffer[length++] = ch;
            ch = fgetc(source);
        }
        ungetc(ch, source);
        buffer[length] = '\0';

        token.type = 11;
        strcpy(token.value, buffer);
        token.line = line_number;
    }
    else if (ch == ':')
    {
        ch = fgetc(source);
        if (ch == '=')
        {
            token.type = 20;
            strcpy(token.value, ":=");
        }
        else
        {
            ungetc(ch, source);
            report_error("Colon not matched", line_number);
            token.type = 0;
            strcpy(token.value, ":");
        }
        token.line = line_number;
    }
    else if (ch == '<')
    {
        ch = fgetc(source);
        if (ch == '>')
        {
            token.type = 13;
            strcpy(token.value, "<>");
        }
        else if (ch == '=')
        {
            token.type = 14;
            strcpy(token.value, "<=");
        }
        else
        {
            ungetc(ch, source);
            token.type = 15;
            strcpy(token.value, "<");
        }
        token.line = line_number;
    }
    else if (ch == '>')
    {
        ch = fgetc(source);
        if (ch == '=')
        {
            token.type = 16;
            strcpy(token.value, ">=");
        }
        else
        {
            ungetc(ch, source);
            token.type = 17;
            strcpy(token.value, ">");
        }
        token.line = line_number;
    }
    else if (ch == '=')
    {
        token.type = 12;
        strcpy(token.value, "=");
        token.line = line_number;
    }
    else if (ch == '-'){
        token.type = 18;
        strcpy(token.value, "-");
        token.line = line_number;
    }
    else if (ch == '*')
    {
        token.type = 19;
        strcpy(token.value, "*");
        token.line = line_number;
    }
    else if (ch == '(')
    {
        token.type = 21;
        strcpy(token.value, "(");
        token.line = line_number;
    }
    else if (ch == ')')
    {
        token.type = 22;
        strcpy(token.value, ")");
        token.line = line_number;
    }
    else if (ch == ';')
    {
        token.type = 23;
        strcpy(token.value, ";");
        token.line = line_number;
    }
    else
    {
        char illegal_char[2];
        illegal_char[0] = ch;
        illegal_char[1] = '\0';
        report_error("Illegal character", line_number);
        token.type = 0;
        strcpy(token.value, illegal_char);
        token.line = line_number;
    }

    return token;
}

void lexical_analysis()
{
    Token token;
    source = fopen("E:\\C++\\Compile\\source.pas", "r");
    dyd = fopen("E:\\C++\\Compile\\output.dyd", "w");
    err = fopen("E:\\C++\\Compile\\output.err", "w");

    if (!source || !dyd || !err)
    {
        printf("Error opening files.\n");
        return;
    }

    int current_line = 1;
    while (1)
    {
        token = get_token();
        if (token.type == 25)
        {
            fprintf(dyd, "EOF 25\n");
            break;
        }
        else if (token.type == 24)
        {
            fprintf(dyd, "EOLN 24\n");
            current_line++;
        }
        else if (token.type != 0)
        {
            fprintf(dyd, "%s %d\n", token.value, token.type);
        }
    }

    fclose(source);
    fclose(dyd);
    fclose(err);
}

int main()
{
    lexical_analysis();
    return 0;
}
