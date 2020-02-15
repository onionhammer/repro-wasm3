# Generate SPIFFS
# https://docs.platformio.org/en/latest/platforms/espressif32.html#uploading-files-to-file-system-spiffs
pio run -e "esp32dev" --target buildfs

# Upload SPIFFS
pio run -e "esp32dev" --target uploadfs

# Upload program
pio run -e "esp32dev" --target upload

