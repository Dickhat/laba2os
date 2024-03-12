#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Структура команды
struct command
{
    char name_comand[10];   // Название команды
    char arguments[10][30]; // Аргументы команды
};

// Создание процессов
void proccess_create(struct command * massive_command)
{
    char command[256];  // Буфер команды
    int index = 0;      // Номер команды
    int result_code;    // Код возврата system

    // Извлечение команды из набора введенных команд
    while(strcmp(massive_command[index].name_comand, "\0") != 0)
    {
        memset(command, 0, sizeof(char));
        strcpy(command, massive_command[index].name_comand);

        int index_argument = 0;
        // Извлечение аргументов команды
        while (strcmp(massive_command[index].arguments[index_argument], "\0") != 0)
        {
            strcat(command, massive_command[index].arguments[index_argument]);
            index_argument+=1;
        }
        
        result_code = system(command);    // Вызов процеса по переданной команде

        // Ошибка вызова процесса
        if(result_code == 127 || result_code == -1)
            printf("Command %s incorrect", command);

        index+=1;
    }
}

struct command extraction_commands(struct command * massive_command)
{
    int index = -1;                             // Номер команды
    int index_argument = 0;                     // Номер аргумента команды
    char buffer[1024];                          // Размер буфера входной строки

    fgets(buffer, sizeof(buffer), stdin);       // считываем строку с пробелами
    char *element = strtok(buffer, " ");        // Инициализация первого вызова strtok

    //if Не указана команда терминала
    if (element == NULL)
        printf(" Error command\n");

    // Последующие вызовы strtok для получения следующих элементов
    while (element != NULL)
    {   
        // Текущее слово является командой
        if(strcmp(element, "ls") == 0 || strcmp(element,"cat") == 0 || strcmp(element,"nice") == 0 || strcmp(element,"killall") == 0)
        {
            index += 1;
            index_argument = 0;
            // Копирование название команды в структуру команды
            strcpy(massive_command[index].name_comand, element);
        }
        else 
        {
            // Копирование аргумента команды в структуру команды
            strcpy(massive_command[index].arguments[index_argument], element);
            index_argument += 1;
        }

        element = strtok(NULL, " ");
    }

    return *massive_command;
}

int main(int argc, char * argv[])
{
    struct command massive_command[10]; // Массив команд
    
    // Основная программа (Терминал)
    while (1)
    {
        // Получение команд с консоли и занесение их в структуру команд
        *massive_command = extraction_commands(massive_command);

        // Эхо-Печать команд
        for(int i = 0; i < 10; ++i)
        {
            printf(" Name command: %s \n", massive_command[i].name_comand);

            for(int j = 0; j < 10; ++j)
            {
                printf(" Argument %d:%s \n", j, massive_command[i].arguments[j]);
            }
        }

        proccess_create(massive_command);  // Создание процессов

        // Очистка предыдущих команд
        memset(massive_command, 0, sizeof(struct command));
        memset(massive_command->arguments, 0, sizeof(struct command));
    }

    exit(0);
}