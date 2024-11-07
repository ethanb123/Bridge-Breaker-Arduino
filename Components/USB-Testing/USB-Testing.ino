#include <Usb.h>
#include <usbhub.h>
#include <masstorage.h>
#include <fatfs.h>

USB Usb;
USBHub Hub(&Usb);
BulkOnly bulk(&Usb);
MasStorage<MSC_MAX_LUNS> ms(&Usb, &bulk);
FatFS fs;

void setup() {
  Serial.begin(115200);

  if (Usb.Init() == -1) {
    Serial.println("USB Host Shield Initialization failed!");
    while (1); // Halt
  }
  Serial.println("USB Host Shield Initialized.");

  // Initialize mass storage device
  if (ms.Init() == 0) {
    Serial.println("USB Mass Storage device initialized.");
  } else {
    Serial.println("Failed to initialize mass storage device.");
    while (1); // Halt
  }

  // Mount the file system
  if (fs.mount(&ms) != 0) {
    Serial.println("Failed to mount filesystem.");
    while (1); // Halt
  } else {
    Serial.println("Filesystem mounted.");
  }

  // Create and write to a file
  FIL file;
  if (f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
    f_write(&file, "Hello, USB!\r\n", 13, NULL);
    f_close(&file);
    Serial.println("File written successfully.");
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

void loop() {
  // Nothing in loop for now
}
