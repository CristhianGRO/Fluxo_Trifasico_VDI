################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AEMO_DOC_VF.c \
../src/aemt.c \
../src/funcoesAuxiliares.c \
../src/funcoesChaveamento.c \
../src/funcoesHeuristica.c \
../src/funcoesInicializacao.c \
../src/funcoesLeituraDados.c \
../src/funcoesProblema.c \
../src/funcoesRNP.c \
../src/funcoesSetor.c 

OBJS += \
./src/AEMO_DOC_VF.o \
./src/aemt.o \
./src/funcoesAuxiliares.o \
./src/funcoesChaveamento.o \
./src/funcoesHeuristica.o \
./src/funcoesInicializacao.o \
./src/funcoesLeituraDados.o \
./src/funcoesProblema.o \
./src/funcoesRNP.o \
./src/funcoesSetor.o 

C_DEPS += \
./src/AEMO_DOC_VF.d \
./src/aemt.d \
./src/funcoesAuxiliares.d \
./src/funcoesChaveamento.d \
./src/funcoesHeuristica.d \
./src/funcoesInicializacao.d \
./src/funcoesLeituraDados.d \
./src/funcoesProblema.d \
./src/funcoesRNP.d \
./src/funcoesSetor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


