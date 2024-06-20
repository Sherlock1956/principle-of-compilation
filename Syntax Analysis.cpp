#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_ID_LENGTH 16
typedef struct
{
    char vname[MAX_ID_LENGTH + 1];
    char vproc[MAX_ID_LENGTH + 1];
    int vkind; // 0 for variable, 1 for parameter
    char vtype[10];
    int vlev;
    int vadr;
} VarEntry;
typedef struct
{
    char pname[MAX_ID_LENGTH + 1];
    char ptype[10];
    int plev;
    int fadr;
    int ladr;
} FuncEntry;
typedef struct
{
    char value[MAX_ID_LENGTH + 1];
    int type;
    int line;
} Token;

FILE *file, *err, *output, *pro, *var;
VarEntry varTable[100];
FuncEntry funcTable[100];
Token tokens[1000];

int varCount = 1;
int funcCount = 1;
int level = 0;
int tokenCount = 0;
int tokenLine = 1;
int currentTokenIndex = 0;
char typetab[][20] = {{""},{"begin"}, {"end"}, {"integer"}, {"if"}, {"then"}, {"else"}, {"function"}, {"read"}, {"write"}, {"(identifier)"}, {"(number)"}, {"="}, {"<>"}, {"<="}, {"<"}, {">="}, {">"}, {"-"}, {"*"}, {":="}, {"("}, {")"}, {";"}, {"EOLN"}, {"EOF"}};

void add_variable(const char *name, const char *proc, int kind, const char *type = 0, int lev = 0)
{

    strcpy(varTable[varCount].vname, name);
    strcpy(varTable[varCount].vproc, proc);
    varTable[varCount].vkind = kind;
    if (type == 0)
    {
        strcpy(varTable[varCount].vtype, "default");
    }
    else
        strcpy(varTable[varCount].vtype, type);
    varTable[varCount].vlev = lev;
    varTable[varCount].vadr = varCount - 1;
    if (lev > 1)
    {
        funcTable[lev - 1].ladr = varCount - 1;
    }
    varCount++;
}
bool is_variable_exists(const char *name, int level, int kind)
{
    for (int i = 0; i < varCount; i++)
    {
        if ((strcmp(varTable[i].vname, name) == 0) && (varTable[i].vlev == level) && (varTable[i].vkind == kind))
        {
            return true;
        }
    }
    return false;
}
void close_file()
{
    fclose(file);
    fclose(err);
    fclose(output);
    fclose(pro);
    fclose(var);
}
void add_function(const char *name, const char *type, int lev, int fadr, int ladr)
{
    strcpy(funcTable[funcCount].pname, name);
    strcpy(funcTable[funcCount].ptype, type);
    funcTable[funcCount].plev = lev;
    funcTable[funcCount].fadr = fadr;
    funcTable[funcCount].ladr = ladr;
    funcCount++;
}
void print_tables();
Token getNextToken()
{
    return tokens[currentTokenIndex++];
}

Token peekNextToken(int i)
{
    return tokens[currentTokenIndex + i];
}

void syntaxError(const char *message, int type)
// type 0 for warning, -1 for error
{
    fprintf(err, "%s\n", message);
    if (type == -1)
    {
        print_tables();
        close_file();
        exit(1);
    }
}

void match(int expectedType)
{
    Token token = getNextToken();
    while (token.type == 24)
    {
        fprintf(output, "%s %d\n", token.value, token.type);
        token = getNextToken();
    }
    if (token.type != expectedType)
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Expected token \"%s\" but got \"%s\"", token.line, typetab[expectedType], token.value);
        syntaxError(errorMsg, -1);
    }
    else
    {
        fprintf(output, "%s %d\n", token.value, token.type);
    }
}

void parseProgram();
void parseSubProgram();
void parseDeclarationList();
void parseDeclarationListPrime();
void parseDeclaration();
void parseVariableDeclaration();
void parseVariable(int type);
void parseFunctionDeclaration();
void parseIdentifier();
void parseIdentifierPrime();
void parseStatementList();
void parseStatementListPrime();
void parseStatement();
void parseExpression();
void parseExpressionPrime();
void parseTerm();
void parseTermPrime();
void parseFactor();
void parseConditionExpression();
void parseRelationOperator();

void new_line_test()
{
    Token token = peekNextToken(0);
    if (token.type == 24)
    {
        fprintf(output, "%s %d\n", token.value, token.type);
        getNextToken();
    }
}
void parseProgram()
{
    new_line_test();
    parseSubProgram();
    match(25);
}
void parseSubProgram()
{
    match(1);
    level++; // level++ when match begin
    parseDeclarationList();
    parseStatementList();
    match(2);
    level--; // level-- when match end
}
void parseDeclarationList()
{
    new_line_test();
    parseDeclaration();
    parseDeclarationListPrime();
}
void parseDeclarationListPrime()
{
    new_line_test();
    Token token = peekNextToken(0);
    if (token.type == 3) // variable declaration and function declaration both start with 'integer'
    {
        parseDeclaration();
        parseDeclarationListPrime();
    }
}
void parseDeclaration()
{
    new_line_test();
    Token token1 = peekNextToken(0);
    Token token2 = peekNextToken(1);
    if (token1.type == 3 && token2.type == 10) // variable declaration
    {
        parseVariableDeclaration();
        match(23);
    }
    else if (token1.type == 3 && token2.type == 7) // function declaration
    {
        parseFunctionDeclaration();
        match(23);
    }
    else
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Invalid declaration", token1.line);
        syntaxError(errorMsg, -1);
    }
}
void get_proc_name(int level, char *name)
{
    new_line_test();
    int index = level - 1;
    if (index == 0)
    {
        strcpy(name, "main");
    }
    else
    {
        strcpy(name, funcTable[index].pname);
    }
}
void parseVariableDeclaration()
{
    new_line_test();
    match(3);
    Token token = getNextToken();
    if (token.type != 10)
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Expected identifier in variable declaration", token.line);
        syntaxError(errorMsg, -1);
    }
    else if (is_variable_exists(token.value, level, 0))
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Variable %s is already defined", token.line, token.value);
        syntaxError(errorMsg, 0);
    }
    else
    {
        fprintf(output, "%s %d\n", token.value, token.type);
        char proc_name[17];
        get_proc_name(level, proc_name);
        add_variable(token.value, proc_name, 0, 0, level);
    }
    // add variable to variable table

}
void parseVariable(int type)
{
    new_line_test();
    Token token = getNextToken();
    if (token.type != 10)
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Expected identifier", token.line);
        syntaxError(errorMsg, -1);
    }
    if (type == 0) // check if the variable exists
    {
        if (!is_variable_exists(token.value, level, 0))
        {
            char errorMsg[128];
            sprintf(errorMsg, "***%d: Variable %s is not defined", token.line, token.value);
            fprintf(output, "%s %d\n", token.value, token.type);
            syntaxError(errorMsg, 0);
        }
        else
        {
            fprintf(output, "%s %d\n", token.value, token.type);
        }
    }
    else if (type == 1) // formal parameters
    {
        char proc_name[17];
        get_proc_name(level + 1, proc_name);
        add_variable(token.value, proc_name, 1, 0, level + 1);
        fprintf(output, "%s %d\n", token.value, token.type);
    }
    else if (type == 2) // function
    {
        add_function(token.value, "default", level + 1, varCount - 1, varCount - 1);
        char proc_name[17];
        get_proc_name(level + 1, proc_name); // function name is the return value as well
        for (int i = 1; i <= level + 1; i++)
        { // add the function name to the variable table with lower level
            add_variable(token.value, proc_name, 0, 0, i);
        }

        fprintf(output, "%s %d\n", token.value, token.type);
    }
}
void parseFunctionDeclaration()
{
    new_line_test();
    match(3);
    match(7);
    parseVariable(2); // Function variable

    match(21); // LPAREN
    parseVariable(1);
    match(22); // RPAREN
    match(23); // SEMICOLON
    parseSubProgram();
}
void parseStatementList()
{
    new_line_test();
    parseStatement();
    parseStatementListPrime();
}
void parseStatementListPrime()
{
    new_line_test();
    Token token = peekNextToken(0);
    if (token.type == 10 || token.type == 9 || token.type == 8 || token.type == 4) // variable, write, read, if
    {
        parseStatement();
        parseStatementListPrime();
    }
}
void parseStatement()
{
    new_line_test();
    Token token = peekNextToken(0);
    if (token.type == 10) // assign
    {
        // check if the variable is in the table
        parseVariable(0);
        match(20); // assign
        parseExpression();
        match(23);
    }
    else if (token.type == 8) // read
    {
        getNextToken();
        fprintf(output, "%s %d\n", token.value, token.type);
        match(21); // LPAREN
        parseVariable(0);
        match(22); // RPAREN
        match(23); // SEMICOLON
    }
    else if (token.type == 9) // write
    {
        getNextToken();
        fprintf(output, "%s %d\n", token.value, token.type);
        match(21); // LPAREN
        parseVariable(0);
        match(22); // RPAREN
        match(23); // SEMICOLON
    }
    else if (token.type == 4) // if
    {
        getNextToken();
        fprintf(output, "%s %d\n", token.value, token.type);
        parseConditionExpression();
        new_line_test();
        match(5); // then
        parseStatement();
        new_line_test();
        match(6); // else
        parseStatement();
    }
    else
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Expected token: \"read\" / \"write\" / \"if\" / \"(variable)\" but got \"%s\".", token.line,token.value);
        syntaxError(errorMsg, -1);
    }
}
void parseExpression()
{
    new_line_test();
    parseTerm();
    parseExpressionPrime();
}
void parseExpressionPrime()
{
    new_line_test();
    if (peekNextToken(0).type == 18) // minus
    {
        match(18);
        parseTerm();
        parseExpressionPrime();
    }
}
void parseTerm()
{
    new_line_test();
    parseFactor();
    parseTermPrime();
}
void parseTermPrime()
{
    new_line_test();
    if (peekNextToken(0).type == 19) // times
    {
        match(19);
        parseFactor();
        parseTermPrime();
    }
}
void parseFactor()
{
    new_line_test();
    Token token = peekNextToken(0);
    if (token.type == 10) // variable
    {
        parseVariable(0);
        Token token = peekNextToken(0);
        if (strcmp(token.value, "(") == 0)
        {              // function call
            match(21); //(
            parseExpression();
            match(22); //)
        }
    }
    else if (token.type == 11) // number
    {
        getNextToken();
        fprintf(output, "%s %d\n", token.value, token.type);
    }
    else
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Unexpected token in factor", token.line);
        syntaxError(errorMsg, -1);
    }
}
void parseConditionExpression()
{
    new_line_test();
    parseExpression();
    parseRelationOperator();
    parseExpression();
}
void parseRelationOperator()
{
    new_line_test();
    Token token = getNextToken();
    if (token.type != 12 && token.type != 13 && token.type != 14 && token.type != 15 && token.type != 16 && token.type != 17)
    {
        char errorMsg[128];
        sprintf(errorMsg, "***%d: Expected relation operator", token.line);
        syntaxError(errorMsg, -1);
    }
    else
    {
        fprintf(output, "%s %d\n", token.value, token.type);
    }
}
void read_tokens() // read all tokens to tokens array
{
    file = fopen("E:\\C++\\Compile\\output.dyd", "r");
    err = fopen("E:\\C++\\Compile\\output.err", "a");
    output = fopen("E:\\C++\\Compile\\output.dys", "w");
    pro = fopen("E:\\c++\\Compile\\output.pro ", " w ");
    var = fopen("E:\\c++\\Compile\\output.var ", " w ");

    fseek(err, 0, SEEK_END); // move the file pointer to the end of the file
    long size = ftell(err);  // get the position of the file pointer
    fseek(err, 0, SEEK_SET); // move the file pointer back to the start of the file

    if (size > 0)
    { // if the position of the file pointer is greater than 0, the file is not empty
        close_file();
        printf("There are errors in earlier stage.\n");
        system("pause");
        exit(1); // exit the program
    }
    // else continue with the program
    if (!file || !err || !output || !pro || !var)
    {
        printf("Error opening file.\n");
        exit(1);
    }
    char buffer[16];
    int length = 0;
    int ch = fgetc(file);
    while (ch != EOF)
    {
        length = 0;
        while (!isdigit(ch) || length == 0 || buffer[length - 1] != ' ') // read value from file
        {
            buffer[length++] = ch;
            ch = fgetc(file);
        }
        ungetc(ch, file);
        buffer[length - 1] = '\0';
        strcpy(tokens[tokenCount].value, buffer);
        fscanf(file, "%d", &tokens[tokenCount].type); // read type from file
        tokens[tokenCount].line = tokenLine;
        if (tokens[tokenCount].type == 24)
        { // end of line
            tokenLine++;
        }
        tokenCount++;
        fgetc(file); // handle new line character
        ch = fgetc(file);
    }
}
void print_tables() // print process table and variable table to correspond file
{
    for (int i = 1; i < varCount; i++)
    {
        fprintf(var, "Name: %s, Proc: %s, Kind: %d, Type: %s, Level: %d, Addr: %d\n", varTable[i].vname, varTable[i].vproc, varTable[i].vkind, varTable[i].vtype, varTable[i].vlev, varTable[i].vadr);
    }

    for (int i = 1; i < funcCount; i++)
    {
        fprintf(pro, "Name: %s, Type: %s, Level: %d, First Addr: %d, Last Addr: %d\n", funcTable[i].pname, funcTable[i].ptype, funcTable[i].plev, funcTable[i].fadr, funcTable[i].ladr);
    }
}
int main()
{
    read_tokens(); // read tokens from file to tokens array
    parseProgram();
    print_tables();
    close_file();
    system("pause");
    return 0;
}
