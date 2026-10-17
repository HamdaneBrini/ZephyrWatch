
BOARD := esp32s3_touch_lcd_1_28/esp32s3/procpu
BUILD_DIR := build
PORT := /dev/ttyACM0
BAUDRATE := 115200
APP_NAME:= ZephyrWatch


.PHONY: build rebuild flash monitor clean help

build:
	clear
	@echo "Building $(APP_NAME)'s firmware..."
	@west build -b $(BOARD) . -d $(BUILD_DIR) -- \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

rebuild:
	@echo "Rebuilding $(APP_NAME)'s firmware..."
	@west build -b $(BOARD) . -d $(BUILD_DIR) \
		--pristine=always -- \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

flash:
	@echo "Flashing $(BOARD)..."
	@west flash -d $(BUILD_DIR) --esp-device $(PORT)

monitor:
	clear
	@echo "Starting UART terminal..."
	@picocom -b $(BAUDRATE) $(PORT)


clean:
	@echo "Cleaning build..."
	@rm -rf $(BUILD_DIR)
	@echo "Successfully removed build directroy !"

help:
	@echo "make build   - Compile ZephyrWatch"
	@echo "make rebuild - Clean and recompile"
	@echo "make flash   - Flash ESP32-S3"
	@echo "make monitor - Open UART terminal"
	@echo "make clean   - Remove build files"