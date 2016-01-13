CXX=avr-g++

CORE_DIR=../../../Core/AttinyCore/
PROJECT_DIR=..
SOFTWARESERIAL_DIR=../../../Utility/SoftwareSerial/
SEEED_RFID_DIR=../../SeeedRFID/
BOUNCE2_DIR=../../../Utility/Bounce2/
LIB_DIR=Attiny84

# avr tools path
AVR_OBJCOPY=/usr/bin/avr-objcopy
AVR_SIZE=/usr/bin/avr-size
AVRDUDE=/usr/bin/avrdude

# Debug flag
#DEBUG=-DDEBUG

# CPU type and speed
MCU=attiny84
CPU_SPEED=8000000UL


# Include (dependencies: Attiny core, FPS_GT511C1R, SoftwareSerial, Bounce2)
INCLUDE=-I$(CORE_DIR) -I$(SEEED_RFID_DIR) -I$(SOFTWARESERIAL_DIR) -I$(BOUNCE2_DIR)

# Libraries (dependencies: Attiny core, FPS_GT511C1R, SoftwareSerial, Bounce2)
LIBS=-L$(CORE_DIR)/$(LIB_DIR) -L$(SOFTWARESERIAL_DIR)/$(LIB_DIR)\
-L$(SEEED_RFID_DIR)/$(LIB_DIR) -L$(BOUNCE2_DIR)/$(LIB_DIR)\
-lbounce2 -lseeedrfid -lsoftwareserial -lattinycore

# Source file and application name
OBJ=main
TARGET=RFID
EEPROM_IMAGE=../RFID.eep

CFLAGS=-Wall -Wno-unused-local-typedefs -Os -fpack-struct -fshort-enums\
-funsigned-char -funsigned-bitfields -fno-exceptions -ffunction-sections\
-fdata-sections -fno-use-cxa-atexit -mmcu=$(MCU) -DF_CPU=$(CPU_SPEED)\
-MMD -MP -MF"$(OBJ).d" -MT"$(OBJ).d" $(DEBUG)

CLINKFLAGS=-Wl,-Map,$(TARGET).map,--cref -Wl,-gc-sections -mmcu=$(MCU)


default: build sizedummy upload 

build: $(TARGET).hex
	
$(TARGET).hex: $(TARGET).elf
	@echo 'Create Flash image (ihex format)'
	$(AVR_OBJCOPY) -R .eeprom -O ihex $< $@
	@echo 'Finished building target: $@'
	@echo ' '

$(TARGET).elf: $(OBJ).o
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	$(CXX) $(CLINKFLAGS) $(INCLUDE) $^ -o $@ $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	
%.o: ../%.cpp
	$(CXX) $< $(CFLAGS) $(INCLUDE) -c -o $@
	
upload:
	@echo 'Invoking: AVRDude'
	$(AVRDUDE) -p$(MCU) -cusbasp -Uflash:w:$(TARGET).hex:a -Ueeprom:w:$(EEPROM_IMAGE):a
	@echo 'Finished building: $@'
	@echo ' '
	
sizedummy: $(TARGET).elf
	@echo 'Invoking: Print Size'
	$(AVR_SIZE) --format=avr --mcu=$(MCU) $(TARGET).elf
	@echo 'Finished building: $@'
	@echo ' '	
	
clean:
	@echo -n Cleaning ...
	$(shell rm $(TARGET).elf 2> /dev/null)
	$(shell rm $(TARGET).hex 2> /dev/null)
	$(shell rm $(TARGET).map 2> /dev/null)
	$(shell rm *.o 2> /dev/null)
	$(shell rm *.d 2> /dev/null)
	@echo " done"
	
