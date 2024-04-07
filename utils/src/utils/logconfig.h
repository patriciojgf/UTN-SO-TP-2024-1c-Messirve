#ifndef LOGCONFIG_H
#define LOGCONFIG_H

#include<stdio.h>
#include<stdlib.h>
#include<commons/error.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

t_log* iniciar_logger(char* nombreLog, char* proceso);
t_config* iniciar_config(char* archivo);
void finalizar_log(t_log* log);
void finalizar_config(t_config* config);

#endif 