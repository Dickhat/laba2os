#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int process_id[1024] = {0};     // Процессы, созданные в программе. Где нулевой элемент указывает их число в массиве

// Структура команды
struct command
{
    char name_comand[10];       // Название команды
    char ** arguments;           // Аргументы команды
};

// Создание процессов (по командам)
void proccess_create(struct command * massive_command, int process_id[])
{
    char command[256];                  // Буфер команды
    int index = 0;                      // Номер команды
    int result_code = 0;                // Код возврата system

    // Извлечение команды из набора введенных команд
    while(strcmp(massive_command[index].name_comand, "\0") != 0)
    {
        // Очистка старой команды
        memset(command, 0, sizeof(char));
        strcpy(command, massive_command[index].name_comand);

        int index_argument = 0;
        // Извлечение аргументов команды
        while (massive_command[index].arguments[index_argument] != NULL)
        {
            strcat(command, " ");
            strcat(command, massive_command[index].arguments[index_argument]);
            index_argument+=1;
        }

        int pid = fork();                   // Создать копию процесса

        //if Потомок, то выполнить процесс
        if(pid == 0)
        {   
            // Вызов процесса с помощью замещения исходного кода другим
            result_code = execvp(massive_command[index].name_comand, massive_command[index].arguments);
            exit(0);                            // Конец дочернего процесса
        }
        // else if родитель - сохранение процесса
        else if (pid != -1)
        {
            int code = waitpid(pid, 0, WNOHANG);// Проверка завершился ли процесс

            // Зомби процесс
            if (code == 0)
            {
                //printf(" Процесс зомби\n");
                process_id[0]+= 1;                  // Увеличение числа процессов
                process_id[process_id[0]] = pid;    // Запись id процесса
            }
        }
        // печать ошибки
        else
        {
            printf(" Команда %s не запущена \n", command);
        }

        index+=1;   // Следующая команда
    }
}

// Извлечение команд из строки терминала
struct command extraction_commands(struct command * massive_command)
{
    int index = 0;                              // Номер команды
    int index_argument;                         // Номер аргумента команды
    char buffer[1024];                          // Размер буфера входной строки
    char pre_command[20][100];                  // Команды до разделения на: название команды и аргументы

    fgets(buffer, sizeof(buffer), stdin);       // считываем строку с пробелами
    
    char *findchar = strchr(buffer, '\n');      // Поиск символа новой строки в строке
    // Если символ новой строки найден, заменить его на нулевой символ
    if (findchar != NULL) 
    {
        *findchar = '\0';
    }

    char *command = strtok(buffer, "|");        // Разбиение команд по '|'
    int k = 0;                                  // Число команд

    // Цикл отделения команд друг от друга
    while (command != NULL)
    {
        strcpy(pre_command[k], command);        // Копирование команды
        k+=1;
        command = strtok(NULL, "|");            // Последующий вызов другим команд
    }

    // Пока команды не закончились
    for(int i = 0; i < k; ++i)
    {   
        char *element = strtok(pre_command[i], " ");
        index_argument = 0;

        // Последующие вызовы strtok для получения следующих элементов
        while (element != NULL)
        {   
            // Текущее слово является командой
            if(index_argument == 0)
            {
                // Копирование название команды в структуру команды на место названия команды
                strcpy(massive_command[index].name_comand, element);
                // Копирование название команды в структуру команды на место первого аргумента
                strcpy(massive_command[index].arguments[index_argument], element);
                index += 1;
                index_argument = 1;
            }
            else 
            {
                if(index_argument > 9)
                {
                    printf(" Превышено число аргументов команды (count = 0..9):%d\n Остальные команды не будут считаны", index_argument );
                    return *massive_command;
                }

                // Копирование аргумента команды в структуру команды
                strcpy(massive_command[index - 1].arguments[index_argument], element);
                index_argument += 1;
            }

            element = strtok(NULL, " ");        // Получение следующего элемента команды
        }
            // Для execvp зануление
            massive_command[index - 1].arguments[index_argument] = NULL;
    }

    //// Эхо-Печать команд
    //for(int i = 0; i < 10; ++i)
    //{
    //    printf(" Name command: %s \n", massive_command[i].name_comand);
    //
    //    for(int j = 0; j < 10; ++j)
    //    {
    //        printf(" Argument %d:%s \n", j, massive_command[i].arguments[j]);
    //    }
    //}

    return *massive_command;
}

// Выделение памяти/очистка под команды
struct command * create_or_clear_struct_command(struct command * massive_command, int flag)
{
    // Выделение памяти и зануление
    if(flag == 0)
    {   
        // Выделение и зануление команд
        for(int i = 0; i < 10; ++i)
        {
            // Выделение памяти под 10 аргументов
            massive_command[i].arguments = (char**)malloc(10*sizeof(char*));
            memset(massive_command[i].name_comand, 0, sizeof(char));

            for(int j = 0; j < 10; ++j)
            {
                // Выделение памяти под 256 символов
                massive_command[i].arguments[j] = (char*)malloc(256*sizeof(char));
                memset(massive_command[i].arguments[j], 0, sizeof(char));
            }
        }
    }
    // Очистка предыдущих команд
    else if (flag == 1)
    {
                // Выделение и зануление команд
        for(int i = 0; i < 10; ++i)
        {
            for(int j = 0; j < 10; ++j)
            {
                free(massive_command[i].arguments[j]);
            }
        }
    }

    return massive_command;
}

// Убийство всех дочерних процессов от программы на Ctrl+C
void kill_process()
{
    // Убийство всех дочерних процессов созданных программой
    for(int i = 1; i <= process_id[0]; ++i)
    {
        int child_pid = waitpid(process_id[i], 0, WNOHANG);  // Проверка завершился ли процесс

        //printf("Code %d:%d", child_pid, process_id[i]);

        // Если процесс еще работает, то возвращается 0
        if (child_pid == 0)
        {
            int result = kill(process_id[i], SIGKILL);      // Убийство процесса по pid
            //printf("Result = %d \n", result);

            // Процесс убит
            if(result != -1)
            {
                printf("Процесс %d удален\n", process_id[i]);
                child_pid = waitpid(process_id[i], 0, 0);
                //printf("Code %d:%d", child_pid, process_id[i]);    
            }
            else
                printf("Процесс %d не удален корректно\n", process_id[i]);
        }
        else if (child_pid  > 0)
        {            
            //printf(" Процесс завершился\n");
        }
    }

    memset(process_id, 0, sizeof(int)*1024);

    //for(int i = 0; i < 1024; ++i)
    //{
    //    printf(" Элемент %d:%d\n", i, process_id[i]);
    //}

    // Опять ожидать сигнал Ctrl+C
    signal(SIGINT, kill_process);
}

int main(int argc, char * argv[])
{
    struct command * massive_command = (struct command*)malloc(sizeof(struct command)*20);  // Массив команд 20 штук
    massive_command = create_or_clear_struct_command(massive_command, 0);                   // Выделение памяти

    //// Эхо-Печать команд
    //for(int i = 0; i < 10; ++i)
    //{
    //    printf(" Name command: %s \n", massive_command[i].name_comand);
    //
    //    for(int j = 0; j < 10; ++j)
    //    {
    //        printf(" Argument %d:%s \n", j, massive_command[i].arguments[j]);
    //    }
    //}

    // Опять ожидать сигнал Ctrl+C
    signal(SIGINT, kill_process);

    // Основная программа (Терминал)
    while (1)
    {
        // Получение команд с консоли и занесение их в структуру команд
        *massive_command = extraction_commands(massive_command);

        //// Эхо-Печать команд
        //for(int i = 0; i < 10; ++i)
        //{
        //    printf(" Name command: %s \n", massive_command[i].name_comand);
        //
        //    for(int j = 0; j < 10; ++j)
        //    {
        //        printf(" Argument %d:%s \n", j, massive_command[i].arguments[j]);
        //    }
        //}

        proccess_create(massive_command, process_id);                           // Создание процессов и их учет в process_id
        massive_command = create_or_clear_struct_command(massive_command, 1);   // Очистка предыдущих команд
        massive_command = create_or_clear_struct_command(massive_command, 0);   // Выделение места под новые команды
        printf("\n Число процессов %d \n", process_id[0]);                      // Печать числа запущенных процессов

        for(int i = 0; i <= process_id[0]; ++i)
        {
            printf(" Процесс %d: %d \n", i, process_id[i]);
        }
    }

    exit(0);
}