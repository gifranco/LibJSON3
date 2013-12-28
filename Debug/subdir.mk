################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../json_reader.cpp \
../json_value.cpp \
../json_writer.cpp \
../main.cpp 

OBJS += \
./json_reader.o \
./json_value.o \
./json_writer.o \
./main.o 

CPP_DEPS += \
./json_reader.d \
./json_value.d \
./json_writer.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


