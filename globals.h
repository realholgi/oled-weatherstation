#define FIRMWAREVERSION "0.8.2"

#define RF_RECEIVER_ID 27 // 12|27

#define HOSTNAME "ESP-Wetter"
#define UBIDOTS_MIN_UPLOAD_INTERVAL 2*60*1000

#define OLED_RESET D3

#define MIN_DIFF 3.0
#define RECEIVER_PIN D6

#define MIN_TEMP_READ_INTERVAL 2000
#define MIN_RECEIVE_INTERVAL 200

#define OFFSET 1

#define DRD_TIMEOUT 2
#define DRD_ADDRESS 0

const char TYPE_NUMBER[]  = "type=\"number\" step=\"any\"";

#define PORTAL_DEFAULT_PASSWORD "password"

#define CONFIG_FILE "/config.json"

// Specify the constants for water vapor and barometric pressure.
#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.12f

