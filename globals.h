#define FIRMWAREVERSION "0.8.2"

#define THERMO_HYGRO_DEVICE 0x1e
#define MY_RF_RECEIVER_ID 27 // 12|27

#define HOSTNAME "wetter"

#define UBI_HOSTNAME "esp-wetter"
#define UBIDOTS_UPLOAD_INTERVAL 2*60

#define OLED_RESET D3

#define MIN_DIFF 3.0
#define RECEIVER_PIN D6

#define MIN_RECEIVE_WAIT_INT 2
#define MIN_RECEIVE_WAIT_EXT 200

#define UPDATE_NTP_TIME_INTERVAL 3600

#define OFFSET 1

#define DRD_TIMEOUT 2
#define DRD_ADDRESS 0

const char TYPE_NUMBER[]  = "type=\"number\" step=\"any\"";

#define PORTAL_DEFAULT_PASSWORD "password"

#define CONFIG_FILE "/config.json"

// Specify the constants for water vapor and barometric pressure.
#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.12f

#define TEMP_OFFSET_INDOOR 1.0
